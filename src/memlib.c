/**
 * Author: Weiqi Feng
 * Date: July 24, 2019
 * 
 * memlib.c is an implementation of memory operation function
 * in memlib.h header files. 
 */
#include "memlib.h"

/* Private global variables */
static char *mem_heap;      /* Points to first byte of heap */
static char *mem_brk;       /* Points to last byte of heap plus 1 */
static char *mem_max_addr;  /* Max legal heap addr plus 1 */ 

/**
 * mem_init - Initialize the memory system model
 */ 
void mem_init(void)
{
    /* 
     * Initialy there is no heap space, brk points 
     * to the end of data segment
     */ 
    mem_heap = (char *)sbrk(0);
    mem_brk = mem_heap; 
}
/**
 * mem_sbrk - Simple model of the sbrk function. Extends
 * the head by incr bytes and returns the start adress of the 
 * new area. In this model, the heap cannot be shrunk. 
 */
void *mem_sbrk(int incr)
{
    char *old_brk = mem_brk;

    if((incr < 0) || (mem_brk = sbrk(incr)) == (void *)(-1))
    {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory\n");
        return (void *)-1;
    }
    /* mem_sbrk succeed! */
    mem_brk += incr;
    return (void *)old_brk;
}


/* mem_deinit - free the storage used by the system memory model */
void mem_deinit(){}

/* mem_reset_brk - reset the simulated brk pointer to make an empty heap */
void mem_reset_brk()
{
    mem_brk = (char *)mem_heap;
}

/* mem_heap_low - return address of the first heap byte */
void *mem_heap_low()
{
    return (void *)mem_heap;
}

/* mem_heap_high - return address of the last heap byte */
void *mem_heap_high()
{
    return (void *)(mem_brk - 1);
}

/* mem_heap_size - return the heap size in bytes */
size_t mem_heap_size()
{
    return (size_t)((void *)mem_brk - (void *)mem_heap);
}

/* mem_page_size - return the page size of the systen */
size_t mem_page_size()
{
    return (size_t)getpagesize();
}

/**
 * Reference:
 *  brk() system call: https://stackoverflow.com/questions/6988487/what-does-the-brk-system-call-do
 *  sbrk() man page: https://linux.die.net/man/2/sbrk
 *  Github repo: https://github.com/DreamAndDead/CSAPP-3e-Solutions/
 */
