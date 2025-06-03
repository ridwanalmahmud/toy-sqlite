#ifndef CURSOR_H
#define CURSOR_H

#include "db.h"
#include "btree.h"

// cursor functions
cursor *table_start(table *table);
cursor *table_find(table *table, uint32_t key);
void *cursor_value(cursor *cursor);
void cursor_advance(cursor *cursor);

#endif // !CURSOR_H
