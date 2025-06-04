#ifndef PAGER_H
#define PAGER_H

#include "db.h"

// pager functions
void *get_page(pager *pager, uint32_t page_num);
uint32_t get_unused_page_num(pager *pager);
pager *pager_open(const char *filename);
void pager_flush(pager *pager, uint32_t page_num);

#endif // !PAGER_H
