#ifndef _CURSOR_H
#define _CURSOR_H

#include "db.h"
#include "btree.h"

// cursor functions
Cursor *table_start(Table *table);
Cursor *table_find(Table *table, uint32_t key);
void *cursor_value(Cursor *cursor);
void cursor_advance(Cursor *cursor);

#endif // !_CURSOR_H
