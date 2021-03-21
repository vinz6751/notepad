#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "types.h"

/* storage is composed of 2K blocks */
#define BLOCK_SIZE 2048

#define NIL_BLOCK (struct mem_control*)-1

/* the control structure for managing this memory */
typedef struct mem_control {
	unsigned int size;
	struct mem_control *next;
	struct mem_control *prev;
	char mem[BLOCK_SIZE];
} STORAGE_BLOCK;

extern STORAGE_BLOCK *first_block; /* pointer to first mem_control block */
extern STORAGE_BLOCK *last_block;  /* pointer */

/* Storage lifecycle */
void storage_init(void);
void storage_deinit(void);  	/* release all storage space */
void storage_compact(void);	/* crunch document storage */

/* Blocks management */
STORAGE_BLOCK *storage_add_block(STORAGE_BLOCK *block); /* add memory to doc after block */
void storage_remove_block(STORAGE_BLOCK *block);   /* free a memory block */
BOOL storage_find_block(long position, STORAGE_BLOCK **block, long *offset); /* find the block containing the position 'position'. Returned in 'block' and offset is the index within that block */

/* User functions */
int storage_insert_string(long index, long length, const char string[]);   /* insert text at some point */
void storage_delete_string(long index, long length); 		     /* remove text at some point */
void storage_duplicate_string(long dest, long source, long length);  /* copy text at source to dest */
long storage_get_length(void); 					/* Returns length of the data in storage */
long storage_search_forwards(char chr, long start);  		/* forward search for char */
long storage_search_backwards(char chr, long start);  		/* reverse search for char */


#endif
