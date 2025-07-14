#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <string>

using namespace std;

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        remove("test.db");
    }

    void TearDown() override {
        remove("test.db");
    }

    vector<string> run_script(const vector<string> &commands) {
        int stdin_pipe[2], stdout_pipe[2];
        pid_t pid;

        if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            close(stdin_pipe[1]);
            close(stdout_pipe[0]);

            dup2(stdin_pipe[0], STDIN_FILENO);
            dup2(stdout_pipe[1], STDOUT_FILENO);
            dup2(stdout_pipe[1], STDERR_FILENO);

            close(stdin_pipe[0]);
            close(stdout_pipe[1]);

            execl("build/db", "db", "test.db", nullptr);
            perror("execl");
            exit(EXIT_FAILURE);
        } else { // Parent process
            close(stdin_pipe[0]);
            close(stdout_pipe[1]);

            FILE *child_stdin = fdopen(stdin_pipe[1], "w");
            if (!child_stdin) {
                perror("fdopen");
                exit(EXIT_FAILURE);
            }

            for (const auto &cmd : commands) {
                fprintf(child_stdin, "%s\n", cmd.c_str());
            }
            fclose(child_stdin);

            char buffer[4096];
            string output;
            ssize_t bytes_read;
            while ((bytes_read =
                        read(stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                output += buffer;
            }
            close(stdout_pipe[0]);

            int status;
            waitpid(pid, &status, 0);

            if (bytes_read < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            vector<string> lines;
            size_t start = 0;
            while (start < output.size()) {
                size_t end = output.find('\n', start);
                if (end == string::npos) {
                    lines.push_back(output.substr(start));
                    break;
                }
                lines.push_back(output.substr(start, end - start));
                start = end + 1;
            }
            return lines;
        }
    }
};

TEST_F(DatabaseTest, InsertAndRetrieveRow) {
    vector<string> script = {
        "insert 1 user1 person1@example.com", "select", ".exit"};

    auto output = run_script(script);

    ASSERT_GE(output.size(), 4);
    EXPECT_EQ(output[0], "db > Executed.");
    EXPECT_EQ(output[1], "db > (1, user1, person1@example.com)");
    EXPECT_EQ(output[2], "Executed.");
    EXPECT_EQ(output[3], "db > ");
}

TEST_F(DatabaseTest, PersistentData) {
    // First run - insert data
    vector<string> script1 = {"insert 1 user1 person1@example.com", ".exit"};
    auto output1 = run_script(script1);

    ASSERT_GE(output1.size(), 2);
    EXPECT_EQ(output1[0], "db > Executed.");
    EXPECT_EQ(output1[1], "db > ");

    // Second run - verify data persists
    vector<string> script2 = {"select", ".exit"};
    auto output2 = run_script(script2);

    ASSERT_GE(output2.size(), 3);
    EXPECT_EQ(output2[0], "db > (1, user1, person1@example.com)");
    EXPECT_EQ(output2[1], "Executed.");
    EXPECT_EQ(output2[2], "db > ");
}

TEST_F(DatabaseTest, MaxLengthStrings) {
    string long_username(32, 'a');
    string long_email(255, 'b');

    string insert_cmd = "insert 1 " + long_username + " " + long_email;
    string expected_row = "(1, " + long_username + ", " + long_email + ")";

    vector<string> script = {insert_cmd, "select", ".exit"};
    auto output = run_script(script);

    ASSERT_GE(output.size(), 4);
    EXPECT_EQ(output[0], "db > Executed.");
    EXPECT_EQ(output[1], "db > " + expected_row);
    EXPECT_EQ(output[2], "Executed.");
    EXPECT_EQ(output[3], "db > ");
}

TEST_F(DatabaseTest, TooLongStrings) {
    string long_username(33, 'a');
    string long_email(256, 'b');

    string insert_cmd = "insert 1 " + long_username + " " + long_email;

    vector<string> script = {insert_cmd, "select", ".exit"};
    auto output = run_script(script);

    ASSERT_GE(output.size(), 3);
    EXPECT_EQ(output[0], "db > String is too long.");
    EXPECT_EQ(output[1], "db > Executed.");
    EXPECT_EQ(output[2], "db > ");
}

TEST_F(DatabaseTest, NegativeId) {
    vector<string> script = {"insert -1 cstack foo@bar.com", "select", ".exit"};

    auto output = run_script(script);

    ASSERT_GE(output.size(), 3);
    EXPECT_EQ(output[0], "db > ID must be positive.");
    EXPECT_EQ(output[1], "db > Executed.");
    EXPECT_EQ(output[2], "db > ");
}

TEST_F(DatabaseTest, DuplicateId) {
    vector<string> script = {"insert 1 user1 person1@example.com",
                             "insert 1 user1 person1@example.com",
                             "select",
                             ".exit"};

    auto output = run_script(script);

    ASSERT_GE(output.size(), 5);
    EXPECT_EQ(output[0], "db > Executed.");
    EXPECT_EQ(output[1], "db > Error: Duplicate key.");
    EXPECT_EQ(output[2], "db > (1, user1, person1@example.com)");
    EXPECT_EQ(output[3], "Executed.");
    EXPECT_EQ(output[4], "db > ");
}

TEST_F(DatabaseTest, BtreeOneNode) {
    vector<string> script = {"insert 3 user3 person3@example.com",
                             "insert 1 user1 person1@example.com",
                             "insert 2 user2 person2@example.com",
                             ".btree",
                             ".exit"};

    auto output = run_script(script);

    ASSERT_GE(output.size(), 9);
    EXPECT_EQ(output[0], "db > Executed.");
    EXPECT_EQ(output[1], "db > Executed.");
    EXPECT_EQ(output[2], "db > Executed.");
    EXPECT_EQ(output[3], "db > Tree:");
    EXPECT_EQ(output[4], "- leaf (size 3)");
    EXPECT_EQ(output[5], "  - 1");
    EXPECT_EQ(output[6], "  - 2");
    EXPECT_EQ(output[7], "  - 3");
    EXPECT_EQ(output[8], "db > ");
}

TEST_F(DatabaseTest, PrintConstants) {
    vector<string> script = {".constants", ".exit"};
    auto output = run_script(script);

    ASSERT_GE(output.size(), 8);
    EXPECT_EQ(output[0], "db > Constants:");
    EXPECT_EQ(output[1], "ROW_SIZE: 293");
    EXPECT_EQ(output[2], "COMMON_NODE_HEADER_SIZE: 6");
    EXPECT_EQ(output[3], "LEAF_NODE_HEADER_SIZE: 14");
    EXPECT_EQ(output[4], "LEAF_NODE_CELL_SIZE: 297");
    EXPECT_EQ(output[5], "LEAF_NODE_SPACE_FOR_CELLS: 4082");
    EXPECT_EQ(output[6], "LEAF_NODE_MAX_CELLS: 13");
    EXPECT_EQ(output[7], "db > ");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
