#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_OUTPUT_LINES 10000
#define MAX_LINE_LENGTH 1000

typedef struct {
    char **lines;
    int count;
} ScriptOutput;

void remove_database() {
    remove("test.db");
}

ScriptOutput run_script(const char *commands[], int num_commands) {
    int stdin_pipe[2], stdout_pipe[2];
    pid_t pid;
    char *buffer = malloc(MAX_OUTPUT_LINES * MAX_LINE_LENGTH);
    ScriptOutput output = {0};

    if (!buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
        perror("pipe");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stdout_pipe[1], STDERR_FILENO);

        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        execl("bin/db", "db", "test.db", (char *)NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        FILE *child_stdin = fdopen(stdin_pipe[1], "w");
        if (!child_stdin) {
            perror("fdopen");
            free(buffer);
            exit(EXIT_FAILURE);
        }
        
        for (int i = 0; i < num_commands; i++) {
            fprintf(child_stdin, "%s\n", commands[i]);
            fflush(child_stdin);
        }
        fclose(child_stdin);

        ssize_t bytes_read = read(stdout_pipe[0], buffer, MAX_OUTPUT_LINES * MAX_LINE_LENGTH - 1);
        close(stdout_pipe[0]);

        int status;
        waitpid(pid, &status, 0);

        if (bytes_read < 0) {
            perror("read");
            free(buffer);
            exit(EXIT_FAILURE);
        }

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            output.lines = malloc(MAX_OUTPUT_LINES * sizeof(char *));
            if (!output.lines) {
                perror("malloc");
                free(buffer);
                exit(EXIT_FAILURE);
            }
            
            char *line = strtok(buffer, "\n");
            while (line != NULL && output.count < MAX_OUTPUT_LINES) {
                output.lines[output.count++] = strdup(line);
                line = strtok(NULL, "\n");
            }
        }
        free(buffer);
    }
    return output;
}

void free_output(ScriptOutput *output) {
    if (output->lines) {
        for (int i = 0; i < output->count; i++) {
            free(output->lines[i]);
        }
        free(output->lines);
    }
    output->lines = NULL;
    output->count = 0;
}

void assert_output_matches(ScriptOutput output, const char *expected[], int expected_count, const char *test_name) {
    if (output.count < expected_count) {
        fprintf(stderr, "[FAIL] %s: Not enough output lines (%d < %d)\n", test_name, output.count, expected_count);
        return;
    }

    for (int i = 0; i < expected_count; i++) {
        if (strcmp(output.lines[i], expected[i]) != 0) {
            fprintf(stderr, "[FAIL] %s: Line %d mismatch\nExpected: '%s'\nGot:      '%s'\n", 
                    test_name, i, expected[i], output.lines[i]);
            return;
        }
    }
    printf("[PASS] %s\n", test_name);
}

void assert_output_ends_with(ScriptOutput output, const char *expected[], int expected_count, const char *test_name) {
    if (output.count < expected_count) {
        fprintf(stderr, "[FAIL] %s: Not enough output lines (%d < %d)\n", test_name, output.count, expected_count);
        return;
    }

    int start_index = output.count - expected_count;
    for (int i = 0; i < expected_count; i++) {
        int output_index = start_index + i;
        if (strcmp(output.lines[output_index], expected[i]) != 0) {
            fprintf(stderr, "[FAIL] %s: Line %d (output index %d) mismatch\nExpected: '%s'\nGot:      '%s'\n", 
                    test_name, i, output_index, expected[i], output.lines[output_index]);
            return;
        }
    }
    printf("[PASS] %s\n", test_name);
}

void test_insert_and_retrieve_row() {
    remove_database();
    const char *script[] = {
        "insert 1 user1 person1@example.com",
        "select",
        ".exit"
    };
    const char *expected[] = {
        "db > Executed.",
        "db > (1, user1, person1@example.com)",
        "Executed.",
        "db > "
    };
    ScriptOutput output = run_script(script, 3);
    assert_output_matches(output, expected, 4, "Insert and retrieve row");
    free_output(&output);
}

void test_persistent_data() {
    remove_database();
    const char *script1[] = {"insert 1 user1 person1@example.com", ".exit"};
    const char *script2[] = {"select", ".exit"};
    
    ScriptOutput out1 = run_script(script1, 2);
    const char *expected1[] = {"db > Executed.", "db > "};
    assert_output_matches(out1, expected1, 2, "Persistent data - initial insert");
    free_output(&out1);

    ScriptOutput out2 = run_script(script2, 2);
    const char *expected2[] = {
        "db > (1, user1, person1@example.com)",
        "Executed.",
        "db > "
    };
    assert_output_matches(out2, expected2, 3, "Persistent data - after restart");
    free_output(&out2);
}

void test_table_full() {
    remove_database();
    char **script = malloc(1402 * sizeof(char *));
    for (int i = 1; i <= 1401; i++) {
        script[i-1] = malloc(50);
        sprintf(script[i-1], "insert %d user%d person%d@example.com", i, i, i);
    }
    script[1401] = ".exit";

    ScriptOutput output = run_script((const char **)script, 1402);
    
    const char *expected[] = {"db > Executed.", "db > "};
    assert_output_ends_with(output, expected, 2, "Table full");
    
    for (int i = 0; i < 1401; i++) free(script[i]);
    free(script);
    free_output(&output);
}

void test_max_length_strings() {
    remove_database();
    char long_username[33], long_email[256];
    memset(long_username, 'a', 32);
    memset(long_email, 'b', 255);
    long_username[32] = '\0';
    long_email[255] = '\0';
    
    char insert_cmd[350];
    sprintf(insert_cmd, "insert 1 %s %s", long_username, long_email);
    
    const char *script[] = {insert_cmd, "select", ".exit"};
    const char *expected[] = {
        "db > Executed.",
        "db > (1, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb)",
        "Executed.",
        "db > "
    };
    ScriptOutput output = run_script(script, 3);
    assert_output_matches(output, expected, 4, "Max length strings");
    free_output(&output);
}

void test_too_long_strings() {
    remove_database();
    char long_username[34], long_email[257];
    memset(long_username, 'a', 33);
    memset(long_email, 'b', 256);
    long_username[33] = '\0';
    long_email[256] = '\0';
    
    char insert_cmd[350];
    sprintf(insert_cmd, "insert 1 %s %s", long_username, long_email);
    
    const char *script[] = {insert_cmd, "select", ".exit"};
    const char *expected[] = {
        "db > String is too long.",
        "db > Executed.",
        "db > "
    };
    ScriptOutput output = run_script(script, 3);
    assert_output_matches(output, expected, 3, "Too long strings");
    free_output(&output);
}

void test_negative_id() {
    remove_database();
    const char *script[] = {
        "insert -1 cstack foo@bar.com",
        "select",
        ".exit"
    };
    const char *expected[] = {
        "db > ID must be positive.",
        "db > Executed.",
        "db > "
    };
    ScriptOutput output = run_script(script, 3);
    assert_output_matches(output, expected, 3, "Negative ID");
    free_output(&output);
}

void test_duplicate_id() {
    remove_database();
    const char *script[] = {
        "insert 1 user1 person1@example.com",
        "insert 1 user1 person1@example.com",
        "select",
        ".exit"
    };
    const char *expected[] = {
        "db > Executed.",
        "db > Error: Duplicate key.",
        "db > (1, user1, person1@example.com)",
        "Executed.",
        "db > "
    };
    ScriptOutput output = run_script(script, 4);
    assert_output_matches(output, expected, 5, "Duplicate ID");
    free_output(&output);
}

void test_btree_one_node() {
    remove_database();
    const char *script[] = {
        "insert 3 user3 person3@example.com",
        "insert 1 user1 person1@example.com",
        "insert 2 user2 person2@example.com",
        ".btree",
        ".exit"
    };
    const char *expected[] = {
        "db > Executed.",
        "db > Executed.",
        "db > Executed.",
        "db > Tree:",
        "- leaf (size 3)",
        "  - 1",
        "  - 2",
        "  - 3",
        "db > "
    };
    ScriptOutput output = run_script(script, 5);
    assert_output_matches(output, expected, 9, "B-tree one node");
    free_output(&output);
}

void test_btree_three_leaf_nodes() {
    remove_database();
    const char *script[17];
    char commands[15][50];
    
    for (int i = 1; i <= 14; i++) {
        sprintf(commands[i-1], "insert %d user%d person%d@example.com", i, i, i);
        script[i-1] = commands[i-1];
    }
    script[14] = ".btree";
    script[15] = "insert 15 user15 person15@example.com";
    script[16] = ".exit";
    
    const char *expected[] = {
        "db > Tree:",
        "- internal (size 1)",
        "  - leaf (size 7)",
        "    - 1",
        "    - 2",
        "    - 3",
        "    - 4",
        "    - 5",
        "    - 6",
        "    - 7",
        "  - key 7",
        "  - leaf (size 7)",
        "    - 8",
        "    - 9",
        "    - 10",
        "    - 11",
        "    - 12",
        "    - 13",
        "    - 14",
        "db > Executed.",
        "db > "
    };
    
    ScriptOutput output = run_script(script, 17);
    assert_output_ends_with(output, expected, 21, "B-tree three leaf nodes");
    free_output(&output);
}

void test_print_constants() {
    remove_database();
    const char *script[] = {".constants", ".exit"};
    const char *expected[] = {
        "db > Constants:",
        "ROW_SIZE: 293",
        "COMMON_NODE_HEADER_SIZE: 6",
        "LEAF_NODE_HEADER_SIZE: 14",
        "LEAF_NODE_CELL_SIZE: 297",
        "LEAF_NODE_SPACE_FOR_CELLS: 4082",
        "LEAF_NODE_MAX_CELLS: 13",
        "db > "
    };
    ScriptOutput output = run_script(script, 2);
    assert_output_matches(output, expected, 8, "Print constants");
    free_output(&output);
}

void test_print_all_rows() {
    remove_database();
    char **script = malloc(17 * sizeof(char *));
    for (int i = 1; i <= 15; i++) {
        script[i-1] = malloc(50);
        sprintf(script[i-1], "insert %d user%d person%d@example.com", i, i, i);
    }
    script[15] = "select";
    script[16] = ".exit";

    const char *expected[] = {
        "db > (1, user1, person1@example.com)",
        "(2, user2, person2@example.com)",
        "(3, user3, person3@example.com)",
        "(4, user4, person4@example.com)",
        "(5, user5, person5@example.com)",
        "(6, user6, person6@example.com)",
        "(7, user7, person7@example.com)",
        "(8, user8, person8@example.com)",
        "(9, user9, person9@example.com)",
        "(10, user10, person10@example.com)",
        "(11, user11, person11@example.com)",
        "(12, user12, person12@example.com)",
        "(13, user13, person13@example.com)",
        "(14, user14, person14@example.com)",
        "(15, user15, person15@example.com)",
        "Executed.",
        "db > "
    };
    
    ScriptOutput output = run_script((const char **)script, 17);
    assert_output_ends_with(output, expected, 17, "Print all rows");
    
    for (int i = 0; i < 15; i++) free(script[i]);
    free(script);
    free_output(&output);
}

int main() {
    test_insert_and_retrieve_row();
    test_persistent_data();
    test_table_full();
    test_max_length_strings();
    test_too_long_strings();
    test_negative_id();
    test_duplicate_id();
    test_btree_one_node();
    test_btree_three_leaf_nodes();
    test_print_constants();
    test_print_all_rows();
    printf("All tests completed.\n");
    return 0;
}
