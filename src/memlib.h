/**
 * Author: Weiqi Feng
 * Date: July 24, 2019
 * 
 * memlib.h is the header file contains some memory operation function. 
 */
#ifndef MEMLIB_H
#define MEMLIB_H
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

/* mem_init - Initialize the memory system model */
void mem_init(void);

/**
 * mem_sbrk - Simple model of the sbrk function. Extends
 * the head by incr bytes and returns the start adress of the 
 * new area. In this model, the heap cannot be shrunk. 
 */
void *mem_sbrk(int incr);

/* mem_deinit - free the storage used by the system memory model */
void mem_deinit();

/* mem_reset_brk - reset the simulated brk pointer to make an empty heap */
void mem_reset_brk();

/* mem_heap_low - return address of the first heap byte */
void *mem_heap_low();

/* mem_heap_high - return address of the last heap byte */
void *mem_heap_high();

/* mem_heap_size - return the heap size in bytes */
size_t mem_heap_size();

/* mem_page_size - return the page size of the systen */
size_t mem_page_size();
#endif