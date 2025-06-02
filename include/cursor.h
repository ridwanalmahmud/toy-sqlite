#ifndef CURSOR_H
#define CURSOR_H

#include "db.h"
#include "btree.h"

// cursor functions
cursor *table_start(table *table);
cursor *table_end(table *table);
void *cursor_value(cursor *cursor);
void cursor_advance(cursor *cursor);

#endif // !CURSOR_H
