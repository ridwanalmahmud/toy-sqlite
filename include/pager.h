#ifndef _PAGER_H
#define _PAGER_H

#include "db.h"

// pager functions
void *get_page(Pager *pager, uint32_t page_num);
uint32_t get_unused_page_num(Pager *pager);
Pager *pager_open(const char *filename);
void pager_flush(Pager *pager, uint32_t page_num);

#endif // !_PAGER_H
