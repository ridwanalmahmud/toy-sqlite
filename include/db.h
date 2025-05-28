#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} input_buffer;

input_buffer *new_input_buffer();
void read_input(input_buffer *input_buffer);
void close_input_buffer(input_buffer *input_buffer);
