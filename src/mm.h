/**
 * Author: Weiqi Feng
 * Date: July 24, 2019
 * 
 * mm.h is the header file contains malloc() and free() functions. 
 */
#ifndef MM_H
#define MM_H
#include <unistd.h>

/* mm_init - Initialize the memory manager */
int mm_init(void);

/* mm_malloc - Allocate a block with at least size bytes of payload */
void *mm_malloc(size_t size);

/* mm_free - Free a block of given pointer */
void mm_free(void *ptr);

/* check_heap - Minimal check of the heap for consistency */
void check_heap(int verbose);
#endif