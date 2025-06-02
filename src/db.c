#include "../include/db.h"
#include "../include/pager.h"
#include "../include/btree.h"

input_buffer *new_input_buffer() {
    input_buffer *input_buf = malloc(sizeof(input_buffer));
    input_buf->buffer = NULL;
    input_buf->buffer_length = 0;
    input_buf->input_length = 0;

    return input_buf;
}

void read_input(input_buffer *input_buffer) {
    ssize_t bytes_read =
        getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
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

void print_row(row *row) {
    printf("(%d | %s | %s)\n", row->id, row->username, row->email);
}

void serialize_row(row *source, void *destination) {
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, row *destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

table *db_open(const char *filename) {
    pager *pager = pager_open(filename);
    table *cur_table = (table *)malloc(sizeof(table));
    cur_table->pager = pager;
    cur_table->root_page_num = 0;

    if (pager->num_pages == 0) {
        void *root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
    }

    return cur_table;
}

void db_close(table *table) {
    pager *pager = table->pager;

    for (uint32_t i = 0; i < pager->num_pages; i++) {
        if (pager->pages[i] == NULL) {
            continue;
        }

        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    int result = close(pager->file_descriptor);
    if (result == -1) {
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void *page = pager->pages[i];
        if (page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }

    free(pager);
    free(table);
}
