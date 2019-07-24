/**
 * Author: Weiqi Feng
 * Date: July 24, 2019
 * 
 * mm.c is the source file implements malloc() and free() functions. 
 */
#include "mm.h"
#include "memlib.h"
/* ------------------------------------------------- */
/*          Basic constants and macros               */
/* ------------------------------------------------- */
#define WSIZE 4             /* Word and header/footer suze (bytes) */
#define DSIZE 8             /* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block pointer bp, compute address of bits header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block pointer bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

/* ------------------------------------------------- */
/*              Global variables                     */
/* ------------------------------------------------- */
static char *heap_listp = 0;        /* Pointer to first block */

/* ------------------------------------------------- */
/* Function prototypes for internal helper routines  */
/* ------------------------------------------------- */

/* extend_heap - Extend heap with free block and return its block pointer */
static void *extend_heap(size_t words);

/* coalsce - Boundary tag coalscing. Return pointer to coalesced block */
static void *coalesce(void *bp);

/* place - place block of asize bytes at start of free block bp and split it */
static void place(void *bp, size_t asize);

/* find_fit - Find a fit for a block with asize bytes */
static void *find_fit(size_t asize);

/* print_block - Print the information of bp */
static void print_block(void *bp);

/* check_block - Minimal check of the block */
static void check_block(void *bp);

/* ------------------------------------------------- */
/* Implenmentation of functions declared in mm.h     */
/* ------------------------------------------------- */
/* mm_init - Initialize the memory manager */
int mm_init(void)
{
    /* Create the initial empty heap */
    mem_init();

    /* mem_sbrk failure */
    if((heap_listp = mem_sbrk(4*WSIZE)) ==  (void *)-1)
        return -1;
    
    PUT(heap_listp, 0);                             /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));        /* Epilogue header */
    heap_listp += 2 * WSIZE;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if(!extend_heap(CHUNKSIZE / WSIZE))
        return -1;
    return 0;
} 

/* mm_malloc - Allocate a block with at least size bytes of payload */
void *mm_malloc(size_t size)
{
    size_t asize;               /* Adjusted block size */
    size_t extendSize;          /* Amount to extend heap if no fit */
    char *bp;                   /* Block pointer */
    /* Init memory system first */
    if(!heap_listp)
        mm_init();
    
    /* Ignore spurious requests */
    if(size == 0)
        return NULL;

    /* Rounded to multiple of 8 bytes */
    if(size <= DSIZE)
        asize = DSIZE;
    else
        asize = DSIZE * ((size + 2 * DSIZE - 1) / DSIZE);

    if(bp = find_fit(asize))
    {
        place(bp, asize);
        return bp;
    }
    
    /* No fit found. Get more memory and place the block */
    extendSize = MAX(asize, CHUNKSIZE);
    if(!(bp = extend_heap(extendSize/WSIZE)))
        return NULL;
    place(bp, size);
    return bp;

}

/* mm_free - Free a block of given pointer */
void mm_free(void *ptr)
{
    if(ptr == NULL)
        return;

    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/* extend_heap - Extend heap with free block and return its block pointer */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    /* Initalize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));           /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));           /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header */

    return coalesce(bp);
}

/* coalsce - Boundary tag coalscing. Return pointer to coalesced block */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc)
    {
        /* Case 1 */
        return bp;
    }
    else if (prev_alloc && !next_alloc)
    {
        /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc)
    {
        /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else {
        /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}
/* place - place block of asize bytes at start of free block bp and split it */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    if(csize - asize >= 2 * DSIZE)
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else 
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

/* find_fit - Find a fit for a block with asize bytes */
static void *find_fit(size_t asize)
{
    void *bp;
    /* First fit search */
    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) 
            return bp;
    }
    /* No fit */
    return NULL;
}

/* print_block - Print the information of bp */
static void print_block(void *bp)
{
    size_t hsize, halloc, fsize, falloc;
    hsize = GET_SIZE(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    falloc = GET_ALLOC(FTRP(bp));
    if(hsize == 0)
    {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp,
    hsize, (halloc ? 'a' : 'f'),
    fsize, (falloc ? 'a' : 'f'));
}

/* check_heap - Minimal check of the heap for consistency */
void check_heap(int verbose)
{
    char *bp = heap_listp;
    if(verbose)
        printf("Heap (%p):\n", heap_listp);
    
    if(GET_SIZE(HDRP(heap_listp)) != DSIZE || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    
    check_block(heap_listp);

    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if(verbose)
            print_block(bp);
        check_block(bp);
    }

    if(verbose)
        print_block(bp);
    if((GET_SIZE(HDRP(bp)) != 0) || !GET_ALLOC(HDRP(bp)))
        printf("Bad epilogue header\n");
}

/* check_block - Minimal check of the block */
static void check_block(void *bp)
{
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}