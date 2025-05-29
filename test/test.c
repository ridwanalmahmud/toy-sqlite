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
char* execute_commands(const char* input) {
    // Create temporary file for input
    char input_filename[] = "/tmp/db_test_XXXXXX";
    int input_fd = mkstemp(input_filename);
    if (input_fd == -1) return NULL;

    // Write input to temp file
    size_t input_len = strlen(input);
    if (write(input_fd, input, input_len) != input_len) {
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

    // Build command
    char command[512];
    snprintf(command, sizeof(command), "./bin/db < %s > %s 2>&1", 
             input_filename, output_filename);

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

    fread(output, 1, output_size, output_file);
    output[output_size] = '\0';
    fclose(output_file);

    // Cleanup
    unlink(input_filename);
    unlink(output_filename);

    return output;
}

// Test case helper
void run_test(const char* test_name, const char* input, const char* expected) {
    tests_run++;
    printf("Running test: %s\n", test_name);

    char* output = execute_commands(input);
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
    run_test("Insert and Select", input, expected);
}

void test_table_full() {
    tests_run++;  // Explicitly count this test
    printf("Running test: Table Full\n");

    // Generate input for 1401 inserts
    char* input = malloc(100000);
    input[0] = '\0';
    for (int i = 1; i <= 1401; i++) {
        char buf[50];
        snprintf(buf, sizeof(buf), "insert %d user%d person%d@example.com\n", i, i, i);
        strcat(input, buf);
    }
    strcat(input, ".exit\n");

    char* output = execute_commands(input);
    if (output && strstr(output, "Error: Table full.")) {
        printf("PASS: Table Full\n");
        tests_passed++;
    } else {
        printf("FAIL: Table Full\n");
        tests_failed++;
    }

    free(input);
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

    char* output = execute_commands(input);
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
    tests_run++;  // Explicitly count this test
    printf("Running test: Too Long Strings\n");

    char long_username[34];
    char long_email[257];
    memset(long_username, 'a', 33); long_username[33] = '\0';
    memset(long_email, 'a', 256); long_email[256] = '\0';

    char input[500];
    snprintf(input, sizeof(input), "insert 1 %s %s\nselect\n.exit\n", long_username, long_email);

    char* output = execute_commands(input);
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
    run_test("Negative ID", input, expected);
}

int main() {
    printf("Starting test suite...\n");

    test_insert_and_select();
    test_table_full();
    test_max_length_strings();
    test_too_long_strings();
    test_negative_id();

    printf("\nTest Summary:\n");
    printf("Total:  %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
