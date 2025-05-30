#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Test counters
int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

// Function to execute commands and capture output
char* execute_commands(const char* input, const char* db_file) {
    // Create temporary file for input
    char input_filename[] = "/tmp/db_test_XXXXXX";
    int input_fd = mkstemp(input_filename);
    if (input_fd == -1) return NULL;

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
             db_file ? db_file : "test.db", 
             input_filename, 
             output_filename);

    // Execute command
    int status = system(command);
    if (status == -1) {
        unlink(input_filename);
        unlink(output_filename);
        return NULL;
    }

    // Read output
    FILE* output_file = fopen(output_filename, "r");
    if (!output_file) {
        unlink(input_filename);
        unlink(output_filename);
        return NULL;
    }

    fseek(output_file, 0, SEEK_END);
    long output_size = ftell(output_file);
    fseek(output_file, 0, SEEK_SET);

    char* output = malloc(output_size + 1);
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
void run_test(const char* test_name, const char* input, const char* expected, const char* db_file) {
    tests_run++;
    printf("Running test: %s\n", test_name);

    char* output = execute_commands(input, db_file);
    if (!output) {
        printf("TEST FAILED: Could not execute test\n");
        tests_failed++;
        return;
    }

    if (strcmp(output, expected) == 0) {
        printf("PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", test_name);
        printf("Expected:\n%s\n", expected);
        printf("Actual:\n%s\n", output);
        tests_failed++;
    }

    free(output);
}

// Test cases
void test_insert_and_select() {
    const char* input = "insert 1 user1 person1@example.com\nselect\n.exit\n";
    const char* expected = "db > Executed.\ndb > (1 | user1 | person1@example.com)\nExecuted.\ndb > ";
    run_test("Insert and Select", input, expected, "test.db");
}

void test_table_full() {
    tests_run++;
    printf("Running test: Table Full\n");

    // Generate input for 1401 inserts
    char* input = malloc(100000);
    if (!input) {
        printf("TEST FAILED: Memory allocation failed\n");
        tests_failed++;
        return;
    }

    input[0] = '\0';
    for (int i = 1; i <= 1401; i++) {
        char buf[50];
        snprintf(buf, sizeof(buf), "insert %d user%d person%d@example.com\n", i, i, i);
        strcat(input, buf);
    }
    strcat(input, ".exit\n");

    char* output = execute_commands(input, "test.db");
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
    const char* test_name = "Max Length Strings";
    tests_run++;
    printf("Running test: %s\n", test_name);

    char long_username[33];
    char long_email[256];
    memset(long_username, 'a', 32); long_username[32] = '\0';
    memset(long_email, 'a', 255); long_email[255] = '\0';

    char input[500];
    snprintf(input, sizeof(input), "insert 1 %s %s\nselect\n.exit\n", long_username, long_email);

    char expected[500];
    snprintf(expected, sizeof(expected), "db > Executed.\ndb > (1 | %s | %s)\nExecuted.\ndb > ", long_username, long_email);

    char* output = execute_commands(input, "test.db");
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
    memset(long_username, 'a', 33); long_username[33] = '\0';
    memset(long_email, 'a', 256); long_email[256] = '\0';

    char input[500];
    snprintf(input, sizeof(input), "insert 1 %s %s\nselect\n.exit\n", long_username, long_email);

    char* output = execute_commands(input, "test.db");
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
    const char* input = "insert -1 user1 person1@example.com\nselect\n.exit\n";
    const char* expected = "db > ID must be positive.\ndb > Executed.\ndb > ";
    run_test("Negative ID", input, expected, "test.db");
}

void test_persistence() {
    const char* db_file = "persist_test.db";

    // Clean up any existing test database
    unlink(db_file);

    // First session - insert data
    const char* input1 = "insert 1 user1 person1@example.com\n.exit\n";
    const char* expected1 = "db > Executed.\ndb > ";
    run_test("Persistence - Insert", input1, expected1, db_file);

    // Second session - verify data persists
    const char* input2 = "select\n.exit\n";
    const char* expected2 = "db > (1 | user1 | person1@example.com)\nExecuted.\ndb > ";
    run_test("Persistence - Verify", input2, expected2, db_file);

    // Clean up
    unlink(db_file);
}
// Updated main() function with better cleanup
int main() {
    printf("Starting test suite...\n");

    // Clean up all test database files before starting
    unlink("test.db");
    unlink("persist_test.db");

    test_insert_and_select();
    unlink("test.db");  // Clean after each test that uses test.db

    test_table_full();
    unlink("test.db");

    test_max_length_strings();
    unlink("test.db");

    test_too_long_strings();
    unlink("test.db");

    test_negative_id();
    unlink("test.db");

    test_persistence();  // Uses persist_test.db which cleans itself

    printf("\nTest Summary:\n");
    printf("Total:  %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
