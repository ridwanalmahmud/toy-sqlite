#ifndef _VM_H
#define _VM_H

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
meta_command_result do_meta_command(InputBuffer *input_buffer, Table *table);
prepare_result prepare_statement(InputBuffer *input_buffer,
                                 Statement *statement);
prepare_result prepare_insert(InputBuffer *input_buffer, Statement *statement);

#endif // !_VM_H
