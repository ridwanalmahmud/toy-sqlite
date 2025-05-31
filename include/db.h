#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} input_buffer;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
} execute_result;

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} row;

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

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} statement_type;

typedef struct {
    statement_type type;
    row row_to_insert;
} statement;

typedef struct {
    int file_descriptor;
    uint32_t file_length;
    uint32_t num_pages;
    void *pages[TABLE_MAX_PAGES];
} pager;

typedef struct {
    pager *pager;
    uint32_t root_page_num;
} table;

typedef struct {
    table *table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table;
} cursor;

const uint32_t ID_SIZE = size_of_attribute(row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE = 4096;

// btree.h
typedef enum {
    NODE_INTERNAL,
    NODE_LEAF,
} node_type;

// common node header layout
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_OFFSET;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_SIZE + IS_ROOT_OFFSET;
const uint8_t COMMON_NODE_HEADER_SIZE= NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

// leaf node header layout
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

// leaf node body layout
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_SIZE + LEAF_NODE_KEY_OFFSET;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;


// accessing leaf node fields
uint32_t* leaf_node_num_cells(void *node);
void* leaf_node_cell(void *node, uint32_t cell_num);
uint32_t* leaf_node_key(void *node, uint32_t cell_num);
void* leaf_node_value(void *node, uint32_t cell_num);
void initialize_leaf_node(void *node);
void leaf_node_insert(cursor *cursor, uint32_t key, row *value);
void print_constants();
void print_leaf_node(void *node);


// buffer functions
input_buffer *new_input_buffer();
void read_input(input_buffer *input_buffer);
void close_input_buffer(input_buffer *input_buffer);

// VM functions
meta_command_result do_meta_command(input_buffer *input_buffer, table *table);
// statement parsing functions
prepare_result prepare_statement(input_buffer *input_buffer, statement *statement);
prepare_result prepare_insert(input_buffer *input_buffer, statement *statement);
execute_result execute_statement(statement* statement, table *table);
execute_result execute_insert(statement* statement, table* table);
execute_result execute_select(statement* statement, table* table);

// database row functions
void print_row(row* row);
void serialize_row(row* source, void* destination);
void deserialize_row(void *source, row* destination);

// database file reader
table* db_open(const char *filename);
void db_close(table* table);

// pager functions
void* get_page(pager* pager, uint32_t page_num);
pager* pager_open(const char* filename);
void pager_flush(pager* pager, uint32_t page_num);

// cursor functions
cursor* table_start(table* table);
cursor* table_end(table* table);
void* cursor_value(cursor* cursor);
void cursor_advance(cursor* cursor);

#endif // !DB_H
