/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define MINBLOCKSIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p)) 
#define PUT(p, val) (*(unsigned int *)(p) = (val)) 

#define GET_SIZE(p) (GET(p) & ~0x7) 
#define GET_ALLOC(p) (GET(p) & 0x1) 
#define GET_PREV_ALLOC(p) (GET(p) & 0x2)

#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp))) 
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE((char*)(bp)-DSIZE))

static char* heap_listp;

static void* extend_heap(size_t words);
static void* coalesce(void *bp);
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *bp);
void *mm_realloc(void *ptr, size_t size);
static void* find_fit(size_t asize);
static void place(void* bp, size_t asize);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1) {
    	return -1;
    }
    
    PUT(heap_listp, 0);
    PUT(heap_listp + 1 * WSIZE, PACK(DSIZE, 0x1));
    PUT(heap_listp + 2 * WSIZE, PACK(DSIZE, 0x1));
    PUT(heap_listp + 3 * WSIZE, PACK(0, 0x1));
    heap_listp += 2 * WSIZE;
    
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
    	return -1;
    }
    
    return 0;
}

static void *extend_heap(size_t words)
{
    char* bp;
    size_t size;
    
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1) {
    	return NULL;
    }
    
    size_t tmp = GET(HDRP(bp)) & 0x2;
    PUT(HDRP(bp), PACK(size, tmp));
    //PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 0x1));
    
    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char* bp;
    
    if (!size) {
    	return NULL;
    }
    
    if (size <= DSIZE) {
    	asize = 2 * DSIZE;
    } else {
    	asize = ((size + 2 * DSIZE - 1) / DSIZE) * DSIZE;
    }
    
    if ((bp = find_fit(asize)) != NULL) {
    	place(bp, asize);
    	return bp;
    }
    
    extendsize = MAX(asize, CHUNKSIZE);
    
    if ((bp = extend_heap(extendsize/WSIZE)) != NULL) {
    	place(bp, asize);
    	return bp;
    }
    
    return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    size_t tmp = GET(HDRP(ptr)) & 0x2;

    PUT(HDRP(ptr), PACK(size, tmp));
    //PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    
    coalesce(ptr);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    //size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    if (prev_alloc && next_alloc) {
    	return bp;
    } else if (prev_alloc && !next_alloc) {
    	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    	PUT(HDRP(bp), PACK(size, 0x2));
    	//PUT(HDRP(bp), PACK(size, 0x0));
    	PUT(FTRP(bp), PACK(size, 0)); //Notice here!!! It has been changed!!!
    } else if (!prev_alloc && next_alloc) {
    	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    	size_t tmp = GET(HDRP(PREV_BLKP(bp))) & 0x2;
    	//PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0x0));
    	PUT(FTRP(bp), PACK(size, 0x0));
    	PUT(HDRP(PREV_BLKP(bp)), PACK(size, tmp));
    	bp = PREV_BLKP(bp);
    } else {
	size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))));
    	size_t tmp = GET(HDRP(PREV_BLKP(bp))) & 0x2;
    	PUT(HDRP(PREV_BLKP(bp)), PACK(size, tmp));
    	//PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0x0));
    	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0x0));
    	bp = PREV_BLKP(bp);
    }
    
    return bp;
} 
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newptr;
    
    if(!size)
    {
        mm_free(ptr);
        return NULL;
    }
    
    newptr = mm_malloc(size);
    if(!ptr || !newptr)  return newptr;
    
    size_t copy_size = MIN(size,GET_SIZE(HDRP(ptr)));
    memcpy(newptr,ptr,copy_size);
    mm_free(ptr);
    return newptr;
}

static void* find_fit(size_t asize) {
    void* bp;
    
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    	if ((GET_SIZE(HDRP(bp)) >= asize) && !(GET_ALLOC(HDRP(bp)))) {
    	    return bp;
    	}
    }
    
    return NULL;
}

static void place(void* bp, size_t asize) {
    size_t size = GET_SIZE(HDRP(bp));
    
    if ((size - asize) >= 2 * DSIZE) {
        PUT(HDRP(bp), PACK(asize, 0x3));
        //PUT(HDRP(bp), PACK(asize, 0x1));
        PUT(FTRP(bp), PACK(asize, 0x1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(size - asize, 0x2));
        //PUT(HDRP(bp), PACK(size - asize, 0x0));
        PUT(FTRP(bp), PACK(size - asize, 0x0));
    } else {
        PUT(HDRP(bp), PACK(size, 0x3));
        //PUT(HDRP(bp), PACK(size, 0x1));
        PUT(FTRP(bp), PACK(size, 0x1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(GET(HDRP(NEXT_BLKP(bp))), 0x2));
    }
}












