#include <xinu.h>
#include <stdio.h>

WORD *vgetmem (int nbytes)
{
	intmask mask = disable();
	struct procent * prptr;
	prptr = &proctab[currpid];
	struct	memblk	*prev, *curr, *leftover, * memorylist;
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	memorylist = &(prptr->vmemlist.mlist);
	prev = memorylist;
	curr = memorylist->mnext;
	while (curr != NULL) {			/* Search free list	*/

		if (curr->mlength == nbytes) {	/* Block is exact match	*/
			prev->mnext = curr->mnext;
			memorylist->mlength -= nbytes;
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { /* Split big block	*/
			leftover = (struct memblk *)((uint32) curr +
					nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			memorylist->mlength -= nbytes;
			restore(mask);
			return (char *)(curr);
		} else {			/* Move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}
	restore(mask);
	return (char *)SYSERR;
}


void printMemory()
{

}

void * addressTranslate ( uint32 address)
{
	//kprintf (" Address received 0x%08x ", address);
	virtual_addr * virt = (virtual_addr* )&address;
	//kprintf(" Page offset 0x%08x \n", virt->page_offset);
	//kprintf(" Page table offset 0x%08x \n", virt-> page_table_offset);
	//kprintf(" Page directory offset 0x%08x \n", virt->page_directory_offset);
	unsigned int page_offset = virt->page_offset;
	unsigned int page_table_offset = virt->page_table_offset;
	unsigned int page_directory_offset = virt->page_directory_offset;
	struct procent * prptr;
	prptr = & proctab[currpid];
	pd_t * pagedir = prptr->pagedir;
	//kprintf("whats at page dir 0x%08x \n ",pagedir[page_directory_offset].pd_base);
	//kprintf(" where's page table 0x%08x \n", VPAGE_TO_VADDRESS(pagedir[page_directory_offset].pd_base));
	pt_t * pagetab = 	VPAGE_TO_VADDRESS(pagedir[page_directory_offset].pd_base);
	//kprintf("whats at page table 0x%08x \n" , pagetab[page_table_offset].pt_base);
	//kprintf(" where's page  0x%08x \n", VPAGE_TO_VADDRESS(pagetab[page_table_offset].pt_base));
	unsigned int page = VPAGE_TO_VADDRESS(pagetab[page_table_offset].pt_base);
	//kprintf(" what's physical address 0x%08x \n", page + page_offset);
	return page+page_offset;

}
