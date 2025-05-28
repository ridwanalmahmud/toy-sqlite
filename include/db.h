#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} input_buffer;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND,
} meta_command_result;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
} prepare_result;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} statement_type;

typedef struct {
    statement_type type;
} statement;

input_buffer *new_input_buffer();
void read_input(input_buffer *input_buffer);
void close_input_buffer(input_buffer *input_buffer);
meta_command_result do_meta_command(input_buffer *input_buffer);
prepare_result prepare_statement(input_buffer *input_buffer, statement *statement);
void execute_statement(statement* statement);
