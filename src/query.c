#include "../include/query.h"

execute_result execute_insert(statement *statement, table *table) {
    void *node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = (*leaf_node_num_cells(node));

    row *row_to_insert = &(statement->row_to_insert);
    uint32_t key_to_insert = row_to_insert->id;
    cursor *cursor = table_find(table, key_to_insert);

    if (cursor->cell_num < num_cells) {
        uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
        if (key_at_index == key_to_insert) {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    leaf_node_insert(cursor, row_to_insert->id, row_to_insert);

    return EXECUTE_SUCCESS;
}

execute_result execute_select(statement *statement, table *table) {
    cursor *cursor = table_start(table);
    row row;
    while (!(cursor->end_of_table)) {
        deserialize_row(cursor_value(cursor), &row);
        print_row(&row);
        cursor_advance(cursor);
    }

    free(cursor);

    return EXECUTE_SUCCESS;
}

execute_result execute_statement(statement *statement, table *table) {
    switch (statement->type) {
    case (STATEMENT_INSERT):
        return execute_insert(statement, table);
    case (STATEMENT_SELECT):
        return execute_select(statement, table);
    }
}
