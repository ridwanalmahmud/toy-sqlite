#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

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
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];
} table;

const uint32_t ID_SIZE = size_of_attribute(row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

input_buffer *new_input_buffer();
void read_input(input_buffer *input_buffer);
void close_input_buffer(input_buffer *input_buffer);
meta_command_result do_meta_command(input_buffer *input_buffer, table *table);
prepare_result prepare_statement(input_buffer *input_buffer, statement *statement);
execute_result execute_statement(statement* statement, table *table);
void print_row(row* row);
void serialize_row(row* source, void* destination);
void deserialize_row(void *source, row* destination);
void* row_slot(table* table, uint32_t row_num);
table* new_table();
void free_table(table* table);
execute_result execute_insert(statement* statement, table* table);
execute_result execute_select(statement* statement, table* table);
