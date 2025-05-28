#include "../include/db.h"

input_buffer *new_input_buffer() {
    input_buffer *input_buffer = malloc(sizeof(input_buffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void read_input(input_buffer *input_buffer) {
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read <= 0) {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->buffer_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(input_buffer *input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

meta_command_result do_meta_command(input_buffer *input_buffer) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

prepare_result prepare_statement(input_buffer *input_buffer, statement *statement) {
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "select", 6) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(statement* statement) {
    switch (statement->type) {
        case (STATEMENT_INSERT):
            printf("INSERT?\n");
            break;
        case (STATEMENT_SELECT):
            printf("SELECT?\n");
            break;
    }
}

int main() {
    input_buffer *input_buffer = new_input_buffer();
    while(true) {
        printf("db > ");
        read_input(input_buffer);
        if (input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_UNRECOGNIZED_COMMAND):
                    printf("Unrecognized command '%s'\n", input_buffer->buffer);
                    continue;
            }
        }

        statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'\n", input_buffer->buffer);
                continue;
        }

        execute_statement(&statement);
        printf("Executed\n");
    }
}
