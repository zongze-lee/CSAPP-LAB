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
#define CHUNKSIZE 0x100
#define MINBLOCKSIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
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

#define PRED(bp) ((char*)(bp) + WSIZE)
#define SUCC(bp) ((char*)bp)

#define PRED_BLKP(bp) (GET(PRED(bp)))
#define SUCC_BLKP(bp) (GET(SUCC(bp)))

static char* heap_listp;
static char* listp; //AWFUL!!!

static void* extend_heap(size_t words);
static void* coalesce(void *bp);
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *bp);
void *mm_realloc(void *ptr, size_t size);
static void* find_fit(size_t asize);
static void* best_fit(size_t asize);
static void place(void* bp, size_t asize);
static int Index(size_t asize);
static void addin(void* bp);
static void delete(void* bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(12 * WSIZE)) == (void*)-1) {
    	return -1;
    }
    
    PUT(heap_listp, NULL);
    PUT(heap_listp + 1 * WSIZE, NULL);
    PUT(heap_listp + 2 * WSIZE, NULL);
    PUT(heap_listp + 3 * WSIZE, NULL);
    PUT(heap_listp + 4 * WSIZE, NULL);
    PUT(heap_listp + 5 * WSIZE, NULL);
    PUT(heap_listp + 6 * WSIZE, NULL);
    PUT(heap_listp + 7 * WSIZE, NULL);
    PUT(heap_listp + 8 * WSIZE, NULL);
    PUT(heap_listp + 9 * WSIZE, PACK(DSIZE, 0x1));
    PUT(heap_listp + 10 * WSIZE, PACK(DSIZE, 0x1));
    PUT(heap_listp + 11 * WSIZE, PACK(0, 0x1));
    listp = heap_listp;
    heap_listp += 10 * WSIZE;
    
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
    	return -1;
    }
    
    return 0;
}

static void *extend_heap(size_t words)
{
    void* bp;
    size_t size;
    
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1) {
    	return NULL;
    }
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(PRED(bp), NULL);
    PUT(SUCC(bp), NULL);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 0x1));
    
    bp = coalesce(bp);
    addin(bp);
    
    return bp;
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
    
    asize = ((size + 2 * DSIZE - 1) / DSIZE) * DSIZE;
    
    if ((bp = find_fit(asize)) != NULL) {
    	place(bp, asize);
    	return bp;
    }
    
    /*
    if ((bp = best_fit(asize)) != NULL) {
    	place(bp, asize);
    	return bp;
    }
    */
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
    
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    
    ptr = coalesce(ptr); //Don't forget here!!!!!!! 
    addin(ptr);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    if (prev_alloc && next_alloc) {
    	return bp;
    } else if (prev_alloc && !next_alloc) {
    	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    	delete(NEXT_BLKP(bp));
    	PUT(HDRP(bp), PACK(size, 0x0));
    	PUT(FTRP(bp), PACK(size, 0x0)); //Notice here!!! It has been changed!!!
    } else if (!prev_alloc && next_alloc) {
    	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    	delete(PREV_BLKP(bp));
    	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0x0));
    	PUT(FTRP(bp), PACK(size, 0x0));
    	bp = PREV_BLKP(bp);
    } else {
	size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))));
	delete(PREV_BLKP(bp));
	delete(NEXT_BLKP(bp));
    	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0x0));
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
    size_t oldsize = GET_SIZE(HDRP(ptr));
    if(ptr == NULL) {
    	return mm_malloc(size);
    }
        
    if(oldsize == size) {
    	return ptr;
    }
        
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }
    
    if(!(newptr = mm_malloc(size))){
        return NULL;
    }
    
    if(size < oldsize) {
    	oldsize = size;
    }
        
    memcpy(newptr, ptr, oldsize);
    mm_free(ptr);
    return newptr;
}

static void* find_fit(size_t asize) {
    void* bp = NULL;
    size_t size = 0;
    size_t ind = Index(asize);
    
    for (int i = ind; i < 9; i++) {
    	bp = listp + i * WSIZE;
    	while ((bp = SUCC_BLKP(bp)) != NULL) {
    	    size = GET_SIZE(HDRP(bp));
    	    if (size >= asize && !GET_ALLOC(HDRP(bp))) {
    	    	return bp;
    	    }
    	}
    }
    
    return NULL;
}

static void* best_fit(size_t asize) {
    void* bp = NULL;
    void* res = NULL;
    size_t min = 0;
    size_t size = 0;
    size_t ind = Index(asize);
    
    for (int i = ind; i < 9; i++) {
    	bp = listp + i * WSIZE;
    	while ((bp = SUCC_BLKP(bp)) != NULL) {
    	    size = GET_SIZE(HDRP(bp));
    	    if (size >= asize && !GET_ALLOC(HDRP(bp)) && (min == 0 || size < min)) {
    	    	size = min;
    	    	res = bp;
    	    }
    	}
    	if (min != 0) {
    	    return res;
    	}
    }
    
    return res;
}

static void place(void* bp, size_t asize) {
    size_t size = GET_SIZE(HDRP(bp));
    delete(bp);
    
    if ((size - asize) >= 2 * DSIZE) {
        PUT(HDRP(bp), PACK(asize, 0x1));
        PUT(FTRP(bp), PACK(asize, 0x1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(size - asize, 0x0));
        PUT(FTRP(bp), PACK(size - asize, 0x0));
        addin(bp);
    } else {
        PUT(HDRP(bp), PACK(size, 0x1));
        PUT(FTRP(bp), PACK(size, 0x1));
    }
}

static int Index(size_t asize) {
    size_t res = 0; 
    asize >>= 5;
    while (asize && res < 8) {
    	asize >>= 1;
    	res++;
    }
    return res;
}

static void addin(void* bp) {
    size_t size = GET_SIZE(HDRP(bp));
    int ind = Index(size);
    void *root = listp + ind * WSIZE;
    
    if(SUCC_BLKP(root) != NULL){
	PUT(PRED(SUCC_BLKP(root)), bp);
	PUT(SUCC(bp), SUCC_BLKP(root));
    } else {
	PUT(SUCC(bp), NULL);
    }
    
    PUT(SUCC(root), bp);
    PUT(PRED(bp), root);
    return bp;
}

static void delete(void* bp) {
    PUT(SUCC(PRED_BLKP(bp)), SUCC_BLKP(bp));
    if (SUCC_BLKP(bp) != NULL) {
	PUT(PRED(SUCC_BLKP(bp)), PRED_BLKP(bp));
    }
}
