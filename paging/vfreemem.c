#include <xinu.h>
syscall vfreemem (char * block_ptr, int size_in_bytes)
{
	intmask mask = disable();
	struct procent * prptr;
	prptr = &proctab[currpid];
	struct	memblk	*next, *prev, *block, * memorylist;
	int nbytes = size_in_bytes;
	char * blkaddr = block_ptr;
	uint32	top;
	if ((nbytes == 0) || ((uint32) blkaddr < (uint32) prptr->vmemlist->minh)
			  || ((uint32) blkaddr > (uint32) prptr->vmemlist->maxh)) {
		restore(mask);
		return SYSERR;
	}
	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	block = (struct memblk *)blkaddr;
	memorylist = &(prptr->vmemlist->mlist);
	prev = memorylist;			/* Walk along free list	*/
	next = memorylist->mnext;
	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->mnext;
	}
	if (prev == memorylist) {		/* Compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->mlength;
	}

	/* Ensure new block does not overlap previous or next blocks	*/

	if (((prev != memorylist) && (uint32) block < top)
	    || ((next != NULL)	&& (uint32) block+nbytes>(uint32)next)) {
		restore(mask);
		return SYSERR;
	}

	memorylist->mlength += nbytes;

	/* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* Coalesce with previous block	*/
		prev->mlength += nbytes;
		block = prev;
	} else {			/* Link into list as new node	*/
		block->mnext = next;
		block->mlength = nbytes;
		prev->mnext = block;
	}

	/* Coalesce with next block if adjacent */
	if (((uint32) block + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
	}
	restore(mask);
	return OK;
}
