#ifndef VM_H
#define VM_H

#include "db.h"
#include "query.h"

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND,
} meta_command_result;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT,
} prepare_result;

// VM functions
meta_command_result do_meta_command(input_buffer *input_buffer, table *table);
prepare_result prepare_statement(input_buffer *input_buffer,
                                 statement *statement);
prepare_result prepare_insert(input_buffer *input_buffer, statement *statement);

#endif // !VM_H
