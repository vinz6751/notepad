#include <mint/osbind.h>
#include "storage.h"
#include "global.h"
#include "types.h"

/* global memory pointers */

STORAGE_BLOCK *first_block; /* pointer to first storage block */
STORAGE_BLOCK *last_block;  /* pointer to last storage block */

void storage_init(void)
{
	/* create storage for one block */
	first_block = last_block = (STORAGE_BLOCK*)Malloc(sizeof(STORAGE_BLOCK));
	first_block->next = first_block->prev = NIL_BLOCK;
	first_block->size = 1;
	first_block->mem[0] = '\r';
	
	/* initialize display parameters */
	cur_index = cur_line = first_par = 0;
	cur_col = cur_row = first_line = 0;
	cur_len = 1;
}  /* end storage_init */


void storage_deinit(void)  /* release all storage space */
{
	STORAGE_BLOCK *block, *nextblock;

	block = first_block;
	while (block != NIL_BLOCK) {
		nextblock=block->next;
		Mfree(block);
		block=nextblock;
	} /* end while */

}  /* end storage_deinit */


STORAGE_BLOCK *storage_add_block(STORAGE_BLOCK *block) /* add memory to doc after block */
{
	STORAGE_BLOCK *nblock,*oblock;

	oblock = block;

	/* allocate memory for the block */
	nblock = (STORAGE_BLOCK*)Malloc(sizeof(STORAGE_BLOCK));
	if (nblock == 0) {
		form_alert(1,"[3][Out of memory.][OK]");
		return NIL_BLOCK;
	}  /* end if */

	/* chain new block into list */
	nblock->next = oblock->next;
	nblock->prev = oblock;
	if (last_block == oblock)
		last_block = nblock;
	else
		oblock->next->prev = nblock;
	oblock->next = nblock;

	/* block is initially empty */
	nblock->size = 0;

	return nblock;

}  /* end storage_add_block */


void storage_remove_block(STORAGE_BLOCK *block)   /* free a memory block */
{
	STORAGE_BLOCK *oblock;

	oblock = block;

	/* unhook the block */
	if (first_block == oblock)
		first_block = oblock->next;
	else
		oblock->prev->next = oblock->next;

	if (last_block == oblock)
		last_block = oblock->prev;
	else
		oblock->next->prev = oblock->prev;

	/* throw it away */
	Mfree(oblock);

}  /* end storage_remove_block */


BOOL storage_find_block(long position, STORAGE_BLOCK **block, long *count) {
	long nextcount;
	
	/* find the block containing the position */
	*block = first_block;
	count = 0;
	nextcount = (*block)->size;	
	while (nextcount <= position) {
		*block = (*block)->next;
		
		if (*block == NIL_BLOCK)
			return FALSE;
		
		*count = nextcount;
		nextcount += (*block)->size;
	}
	
	return TRUE;
}


int storage_insert_string(long index, long length, const char string[])  /* insert text at some point */
{
	char *source, *dest;
	STORAGE_BLOCK *block, *dblock;
	long count, nextcount;
	long newsize;

	/* find the block containing index */
	block = first_block;
	count = 0;
	nextcount = block->size;
	while (nextcount < index) {
		block = block->next;
		
		if (block == NIL_BLOCK)
			return;
		
		count = nextcount;
		nextcount += block->size;
	}  /* end while */

	dblock = block;          /* assume current block is big enough */
	newsize = block->size + length;  /* size of dest block */

	if (newsize > BLOCK_SIZE) {
		/* block spillover */
		
		if (index + length <= count + BLOCK_SIZE) {
			/* inserted text fits in original block */
			dblock = storage_add_block(dblock);
			
			if (dblock == NIL_BLOCK)
				return(0);
			
			newsize = nextcount - index;
			block->size += length - newsize;
		}  /* end if */
		else {
			/* insertion wraps past end of block */
			while (newsize > BLOCK_SIZE) {
				dblock = storage_add_block(dblock);
				
				if (dblock == NIL_BLOCK)
					return(0);
				
				newsize -= BLOCK_SIZE;
			}  /* end while */
		}  /* end else */
	}  /* end if */

	/* copy original text forward to make room for insert */
	source = block->mem + nextcount - count;
	dest = dblock->mem + newsize;
	for (count = nextcount-index; count-- > 0; *(--dest) = *(--source))
		;

	/* copy in string */
	for (count = 0; count < length; *(source++) = string[count++]) {
		if (source == block->mem + BLOCK_SIZE) {
			block->size = BLOCK_SIZE;
			block = block->next;
			source = block->mem;
		} /* end if */
	} /* end for */

	dblock->size = newsize;

	/* adjust region selection variables */
	if (sel_start != sel_end && index < sel_end) {
		sel_end += length;
		if (index < sel_start)
			sel_start += length;
	}  /* end if */

	return(1);
}  /* end storage_insert_string */


void storage_delete_string(long index, long length)       /* remove text at some point */
{
	char *source, *dest;
	STORAGE_BLOCK *block, *nextblock;
	long count, nextcount;
	long start;

	/* find the block containing index */
	block = first_block;
	count = 0;
	nextcount = block->size;
	while (nextcount <= index) {
		block = block->next;
		
		if (block == NIL_BLOCK)
			return;
		
		count = nextcount;
		nextcount += block->size;
	}  /* end while */

	start = index - count;                /* offset of copy target */
	while (nextcount <= index + length) {
		/* delete past end of block */
		nextblock = block->next;

		if (start == 0)   
			storage_remove_block(block);           /* dump whole blocks */
		else {
			block->size = start;        /* truncate partial block */
			start = 0;
		} /* end else */
		
		block = nextblock;
		if (block == NIL_BLOCK)
			return;
		count = nextcount;
		nextcount += block->size;
	}  /* end while */

	/* adjust last (or only) block */
	dest = block->mem + start;
	source = block->mem + index + length - count;
	if (dest != source) {
		block->size -= source - dest;
		/* register variable 'count' changes meaning here */
		for (count = nextcount - index - length;
			 count-- > 0;
			 *(dest++) = *(source++));
	} /* end if */
	
	/* adjust region selection variables */
	if (sel_start != sel_end && index < sel_end) {
		if (index <= sel_start) {
			if (index+length <= sel_start) {
				sel_start -= length;
				sel_end -= length;
			}  /* end if */
			else if (index+length < sel_end) {
				sel_start = index;
				sel_end -= length;
			}  /* end if */
			else
				sel_start = sel_end = 0;
		}  /* end if */
		else {
			if (index + length >= sel_end)
				sel_end = index;
			else
				sel_end -= length;
		}  /* end else */
	}  /* end if */

}  /* end storage_delete_string */


void storage_duplicate_string(long dest, long source, long length)  /* copy text at source to dest */
{
	STORAGE_BLOCK *block, *dblock, *newblock, *iblock;
	char *copyfrom, *copyto;
	long count, nextcount;
	long newsize;

	if (source + length > doc_len())
		return;

	/* find the block containing dest */
	block = first_block;
	count = 0;
	nextcount = block->size;
	while (nextcount <= dest) {
		block = block->next;
		
		if (block == NIL_BLOCK)
			return;
		count = nextcount;
		nextcount += block->size;
	}  /* end while */

	/* split document block containing dest */
	if (count == dest)
		iblock = block;
	else {
		iblock = storage_add_block(block);
		if (iblock == NIL_BLOCK)
			return;
		
		copyfrom = block->mem + dest - count;
		copyto = iblock->mem;
		while (copyfrom < block->mem + block->size)
			*(copyto++) = *(copyfrom++);
		
		block->size = dest - count;
		iblock->size = nextcount - dest;
	}  /* end else */

	/* build chain of blocks for the duplicate text */
	dblock = NIL_BLOCK;
	for (newsize = 0; newsize < length; newsize += BLOCK_SIZE) {
		newblock = (STORAGE_BLOCK*)Malloc(sizeof(STORAGE_BLOCK));
		if (newblock == 0)
			break;
		
		newblock->next = dblock;
		newblock->size = 0;
		
		if (dblock != NIL_BLOCK)
			dblock->prev = newblock;
		dblock = newblock;
	}  /* end for */
	
	if (newsize < length) {
		/* not enough mem */	
		while (dblock != NIL_BLOCK) {
			/* free what we already have, then bail out */
			newblock = dblock->next;
			Mfree(dblock);
			dblock = newblock;
		}  /* end while */
		form_alert(1,"[3][Out of memory.][OK]");
		return;
	}  /* end if */
	
	/* find the block containing source */
	block = first_block;
	count = 0;
	nextcount = block->size;
	while (nextcount <= source) {
		if ((block=block->next) == NIL_BLOCK)
			return;
		count = nextcount;
		nextcount += block->size;
	}  /* end while */

	/* copy the text to the duplicate chain */
	copyfrom = block->mem + source - count;
	copyto = newblock->mem;
	for (count = length; count-- > 0; *(copyto++) = *(copyfrom++)) {
		if (copyfrom == block->mem + BLOCK_SIZE) {
			do {
				block = block->next;
				copyfrom = block->mem;
			} while (block->size == 0);   /* skip empty blocks */
		}  /* end if */
		
		if (copyto == newblock->mem + BLOCK_SIZE) {
			newblock->size = BLOCK_SIZE;
			newblock = newblock->next;
			copyto = newblock->mem;
		}  /* end if */
	}  /* end for */
	newblock->size = copyto - newblock->mem;

	/* insert copied text into document before iblock */
	if (iblock == first_block)
		first_block = dblock;
	else
		iblock->prev->next = dblock;
	dblock->prev = iblock->prev;
	newblock->next = iblock;
	iblock->prev = newblock;

	/* adjust region selection variables */
	if (sel_start != sel_end && dest < sel_end) {
		sel_end += length;
		if (dest < sel_start)
			sel_start += length;
	}  /* end if */

}  /* end storage_duplicate_string */


long storage_search_backwards(char chr, long start)  /* reverse search for char */
{
	long count, nextcount;
	STORAGE_BLOCK *block;
	char *pnt;

	if (start < 0)
		return -1;

	/* find the block containing start */
	if (!storage_find_block(start, &block, &count))
		return -1;

	/* reverse search for char */
	for (pnt = block->mem + start - count; *pnt != chr; pnt--) {
		if (pnt == block->mem) {
			do {
				if ((block=block->prev) == NIL_BLOCK)
					return(-1);
				pnt = block->mem + block->size;
				nextcount = count;
				count -= block->size;
			} while (block->size == 0);   /* skip empty blocks */
		}  /* end if */
	}  /* end for */
	
	return (count + (pnt - block->mem));

}  /* end storage_search_backwards */


long storage_search_forwards(char chr, long start)  /* forward search for char */
{
	long count, nextcount;
	STORAGE_BLOCK *block;
	char *pnt;

	if (start < 0)
		return -1;

	/* find the block containing start */
	block = first_block;
	count = 0;
	nextcount=block->size;
	while (nextcount <= start) {
		block = block->next;
		if (block == NIL_BLOCK)
			return -1;
		count = nextcount;
		nextcount += block->size;
	}  /* end while */

	/* forward search for char */
	for (pnt = block->mem + start - count; *pnt != chr;) {
		if (++pnt == block->mem + nextcount - count) {
			do {
				block = block->next;
				if (block == NIL_BLOCK)
					return -1;
					
				pnt = block->mem;
				count = nextcount;
				nextcount += block->size;
			} while (block->size == 0);   /* skip empty blocks */
		}  /* end if */
	}  /* end for */
	
	return (count + (pnt - block->mem));

}  /* end storage_search_backwards */


long storage_get_length(void)
{
	long count, nextcount;
	STORAGE_BLOCK *block;

	/* find the last block */
	block = first_block;
	count = 0;
	nextcount = block->size;
	while (block != NIL_BLOCK) {
		block = block->next;
		count = nextcount;
		nextcount += block->size;
	}  /* end while */
	
	return nextcount;
}


void storage_compact(void)   /* crunch document storage */
{
	STORAGE_BLOCK *dblock;
	STORAGE_BLOCK *sblock;
	long   count;
	char   *source, *dest;

	dblock = first_block;

	while (dblock->next != NIL_BLOCK) {
		sblock = dblock->next;     
		
		if (dblock->size == BLOCK_SIZE) {
			/* block is full */
			dblock = sblock;
			continue;
		}
		
		/* partially filled block */
		source = sblock->mem;
		dest = dblock->mem + dblock->size;
		
		if (dblock->size + sblock->size <= BLOCK_SIZE) {
			/* merge two blocks */
			for (count = sblock->size;
				 count-- > 0;
				 *(dest++) = *(source++));   /* copy text */
			
			dblock->size += sblock->size;
			storage_remove_block(dblock->next);
		}  /* end if */
		else {
			/* doesn't all fit */
			for (count = BLOCK_SIZE - dblock->size;
				 count-- > 0;
				 *(dest++) = *(source++));   /* copy text */
			
			/* adjust sizes */
			sblock->size -= BLOCK_SIZE - dblock->size;
			dblock->size = BLOCK_SIZE;
			
			dest = sblock->mem;
			for (count = sblock->size;
				 count-- > 0;
				 *(dest++) = *(source++));   /* shuffle dest */
			
			dblock = sblock;
		}  /* end else */
	}  /* end while */
}  /* end storage_compact */
