#ifndef QUERY_H
#define QUERY_H

#include "db.h"
#include "btree.h"
#include "cursor.h"

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_DUPLICATE_KEY,
} execute_result;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} statement_type;

typedef struct {
    statement_type type;
    row row_to_insert;
} statement;

execute_result execute_insert(statement *statement, table *table);
execute_result execute_select(statement *statement, table *table);
execute_result execute_statement(statement *statement, table *table);

#endif // !QUERY_H
