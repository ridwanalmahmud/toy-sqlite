#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

// Test counters
int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

// Function to execute commands and capture output
char *execute_commands(const char *input, const char *db_file) {
    // Create temporary file for input
    char input_filename[] = "/tmp/db_test_XXXXXX";
    int input_fd = mkstemp(input_filename);
    if (input_fd == -1)
        return NULL;

    // Write input to temp file
    size_t input_len = strlen(input);
    ssize_t bytes_written = write(input_fd, input, input_len);
    if (bytes_written < 0 || (size_t)bytes_written != input_len) {
        close(input_fd);
        unlink(input_filename);
        return NULL;
    }
    close(input_fd);

    // Create output file
    char output_filename[] = "/tmp/db_test_XXXXXX";
    int output_fd = mkstemp(output_filename);
    if (output_fd == -1) {
        unlink(input_filename);
        return NULL;
    }
    close(output_fd);

    // Build command - always include database filename
    char command[512];
    snprintf(command, sizeof(command), "./bin/db %s < %s > %s 2>&1",
             db_file ? db_file : "test.db", input_filename, output_filename);

    // Execute command
    int status = system(command);
    if (status == -1) {
        unlink(input_filename);
        unlink(output_filename);
        return NULL;
    }

    // Read output
    FILE *output_file = fopen(output_filename, "r");
    if (!output_file) {
        unlink(input_filename);
        unlink(output_filename);
        return NULL;
    }

    fseek(output_file, 0, SEEK_END);
    long output_size = ftell(output_file);
    fseek(output_file, 0, SEEK_SET);

    char *output = malloc(output_size + 1);
    if (!output) {
        fclose(output_file);
        unlink(input_filename);
        unlink(output_filename);
        return NULL;
    }

    size_t bytes_read = fread(output, 1, output_size, output_file);
    output[bytes_read] = '\0';
    fclose(output_file);

    // Cleanup
    unlink(input_filename);
    unlink(output_filename);

    return output;
}

// Test case helper
void run_test(const char *test_name, const char *input, const char *expected,
              const char *db_file) {
    tests_run++;
    printf("Running test: %s\n", test_name);

    char *output = execute_commands(input, db_file);
    if (!output) {
        printf("TEST FAILED: Could not execute test\n");
        tests_failed++;
        return;
    }

    // Normalize strings by trimming trailing whitespace
    char *normalized_output = strdup(output);
    char *normalized_expected = strdup(expected);

    // Trim trailing whitespace
    for (char *p = normalized_output + strlen(normalized_output) - 1;
         p >= normalized_output && isspace(*p); p--) {
        *p = '\0';
    }
    for (char *p = normalized_expected + strlen(normalized_expected) - 1;
         p >= normalized_expected && isspace(*p); p--) {
        *p = '\0';
    }

    if (strcmp(normalized_output, normalized_expected) == 0) {
        printf("PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", test_name);
        printf("Expected:\n'%s'\n", expected);
        printf("Actual:\n'%s'\n", output);

        // Show difference
        printf("First difference at position: ");
        for (size_t i = 0;; i++) {
            if (normalized_expected[i] != normalized_output[i]) {
                printf("%zu (expected 0x%02x, got 0x%02x)\n", i,
                       (unsigned char)normalized_expected[i],
                       (unsigned char)normalized_output[i]);
                break;
            }
            if (normalized_expected[i] == '\0')
                break;
        }

        tests_failed++;
    }

    free(normalized_output);
    free(normalized_expected);
    free(output);
}

// Test cases
void test_insert_and_select() {
    const char *input = "insert 1 user1 person1@example.com\nselect\n.exit\n";
    const char *expected = "db > Executed.\ndb > (1 | user1 | "
                           "person1@example.com)\nExecuted.\ndb > ";
    run_test("Insert and Select", input, expected, "test.db");
}

void test_table_full() {
    tests_run++;
    printf("Running test: Table Full\n");

    // Generate input for 1401 inserts
    char *input = malloc(100000);
    if (!input) {
        printf("TEST FAILED: Memory allocation failed\n");
        tests_failed++;
        return;
    }

    input[0] = '\0';
    for (int i = 1; i <= 1401; i++) {
        char buf[50];
        snprintf(buf, sizeof(buf), "insert %d user%d person%d@example.com\n", i,
                 i, i);
        strcat(input, buf);
    }
    strcat(input, ".exit\n");

    char *output = execute_commands(input, "test.db");
    free(input);

    if (output && strstr(output, "Error: Table full.")) {
        printf("PASS: Table Full\n");
        tests_passed++;
    } else {
        printf("FAIL: Table Full\n");
        tests_failed++;
    }

    free(output);
}

void test_max_length_strings() {
    const char *test_name = "Max Length Strings";
    tests_run++;
    printf("Running test: %s\n", test_name);

    char long_username[33];
    char long_email[256];
    memset(long_username, 'a', 32);
    long_username[32] = '\0';
    memset(long_email, 'a', 255);
    long_email[255] = '\0';

    char input[500];
    snprintf(input, sizeof(input), "insert 1 %s %s\nselect\n.exit\n",
             long_username, long_email);

    char expected[500];
    snprintf(expected, sizeof(expected),
             "db > Executed.\ndb > (1 | %s | %s)\nExecuted.\ndb > ",
             long_username, long_email);

    char *output = execute_commands(input, "test.db");
    if (output && strcmp(output, expected) == 0) {
        printf("PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", test_name);
        printf("Expected:\n%s\n", expected);
        printf("Actual:\n%s\n", output ? output : "(null)");
        tests_failed++;
    }
    free(output);
}

void test_too_long_strings() {
    tests_run++;
    printf("Running test: Too Long Strings\n");

    char long_username[34];
    char long_email[257];
    memset(long_username, 'a', 33);
    long_username[33] = '\0';
    memset(long_email, 'a', 256);
    long_email[256] = '\0';

    char input[500];
    snprintf(input, sizeof(input), "insert 1 %s %s\nselect\n.exit\n",
             long_username, long_email);

    char *output = execute_commands(input, "test.db");
    if (output && strstr(output, "String is too long.")) {
        printf("PASS: Too Long Strings\n");
        tests_passed++;
    } else {
        printf("FAIL: Too Long Strings\n");
        tests_failed++;
    }
    free(output);
}

void test_negative_id() {
    const char *input = "insert -1 user1 person1@example.com\nselect\n.exit\n";
    const char *expected = "db > ID must be positive.\ndb > Executed.\ndb > ";
    run_test("Negative ID", input, expected, "test.db");
}

void test_persistence() {
    const char *db_file = "persist_test.db";

    // Clean up any existing test database
    unlink(db_file);

    // First session - insert data
    const char *input1 = "insert 1 user1 person1@example.com\n.exit\n";
    const char *expected1 = "db > Executed.\ndb > ";
    run_test("Persistence - Insert", input1, expected1, db_file);

    // Second session - verify data persists
    const char *input2 = "select\n.exit\n";
    const char *expected2 =
        "db > (1 | user1 | person1@example.com)\nExecuted.\ndb > ";
    run_test("Persistence - Verify", input2, expected2, db_file);

    // Clean up
    unlink(db_file);
}

void test_btree_structure() {
    const char *input = "insert 3 user3 person3@example.com\n"
                        "insert 1 user1 person1@example.com\n"
                        "insert 2 user2 person2@example.com\n"
                        ".btree\n"
                        ".exit\n";
    const char *expected = "db > Executed.\n"
                           "db > Executed.\n"
                           "db > Executed.\n"
                           "db > Tree: \n"
                           "- leaf (size 3)\n"
                           "  - 1\n"
                           "  - 2\n"
                           "  - 3\n"
                           "db > ";
    run_test("B-Tree Structure", input, expected, "test.db");
}


void test_duplicate_key() {
    const char *input = "insert 1 user1 person1@example.com\n"
                        "insert 1 user1 person1@example.com\n"
                        "select\n"
                        ".exit\n";
    const char *expected = "db > Executed.\n"
                           "db > Error: Duplicate key.\n"
                           "db > (1 | user1 | person1@example.com)\n"
                           "Executed.\n"
                           "db > ";
    run_test("Duplicate Key", input, expected, "test.db");
}

void test_print_constants() {
    const char *input = ".constants\n.exit\n";
    const char *expected = "db > Constants: \n"
                           "ROW_SIZE: 293\n"
                           "COMMON_NODE_HEADER_SIZE: 6\n"
                           "LEAF_NODE_HEADER_SIZE: 10\n"
                           "LEAF_NODE_CELL_SIZE: 297\n"
                           "LEAF_NODE_SPACE_FOR_CELLS: 4086\n"
                           "LEAF_NODE_MAX_CELLS: 13\n"
                           "db > ";
    run_test("Print Constants", input, expected, "test.db");
}

// Updated main() function with better cleanup
int main() {
    printf("Starting test suite...\n");

    // Clean up all test database files before starting
    unlink("test.db");
    unlink("persist_test.db");

    test_insert_and_select();
    unlink("test.db"); // Clean after each test that uses test.db

    test_table_full();
    unlink("test.db");

    test_max_length_strings();
    unlink("test.db");

    test_too_long_strings();
    unlink("test.db");

    test_negative_id();
    unlink("test.db");

    test_persistence(); // Uses persist_test.db which cleans itself

    test_btree_structure();
    unlink("test.db");

    test_duplicate_key();
    unlink("test.db");

    test_print_constants();
    unlink("test.db");

    printf("\nTest Summary:\n");
    printf("Total:  %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
