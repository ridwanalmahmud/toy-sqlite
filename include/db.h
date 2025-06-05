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
#define TABLE_MAX_PAGES 400
#define INVALID_PAGE_NUM UINT32_MAX
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} input_buffer;

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} row;

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

static const uint32_t ID_SIZE = size_of_attribute(row, id);
static const uint32_t USERNAME_SIZE = size_of_attribute(row, username);
static const uint32_t EMAIL_SIZE = size_of_attribute(row, email);
static const uint32_t ID_OFFSET = 0;
static const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
static const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
static const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
static const uint32_t PAGE_SIZE = 4096;

// buffer read functions
input_buffer *new_input_buffer();
void read_input(input_buffer *input_buffer);
void close_input_buffer(input_buffer *input_buffer);

// database file reader
table *db_open(const char *filename);
void db_close(table *table);

// database row functions
void print_row(row *row);
void serialize_row(row *source, void *destination);
void deserialize_row(void *source, row *destination);

#endif // !DB_H
