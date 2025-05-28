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

int main(int argc, char *argv[]) {
    input_buffer *input_buffer = new_input_buffer();
    while(true) {
        printf("db > ");
        read_input(input_buffer);
        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
        }
    }
}
