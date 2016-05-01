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

	if (prptr->vmemlist == NULL)
	{
			prptr->vmemlist = getmem(sizeof(vhmdata));
			vhmdata * vhmdatavar = prptr->vmemlist ;
			struct	memblk	*memptr;	/* Ptr to memory block		*/
			/* Initialize the free list */
			memptr = &(vhmdatavar->mlist);
			//kprintf(" Memptr is 0x%08x", memptr);
			memptr->mnext = (struct memblk *)NULL;
			memptr->mlength = 0;
			/* Initialize the memory counters */
			/*    Heap starts at the end of Xinu image */
			vhmdatavar->maxh = (uint32)((prptr->vpagestart + prptr->vpagesize)*4096);
			vhmdatavar->minh = (uint32)(prptr->vpagestart*4096);
			//LOG(" Max heap is 0x%08x \n", vhmdatavar->maxheap);
			//LOG(" Min heap is 0x%08x \n", vhmdatavar->minheap);
			memptr->mlength = (uint32)vhmdatavar->maxh - (uint32)vhmdatavar->minh;
			//LOG(" Size of mlength is %d \n", memptr->mlength);
			memptr->mnext = (struct memblk *)vhmdatavar->minh;
			memptr = memptr->mnext;
			memptr->mnext = NULL;
			memptr->mlength = (uint32)vhmdatavar->maxh - (uint32)vhmdatavar->minh;
			//LOG(" Size of mlength is %d \n", memptr->mlength);

	}
	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	memorylist = &(prptr->vmemlist->mlist);

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

	struct procent * prptr;
		struct	memblk	*memptr;	/* Ptr to memory block		*/
		prptr = &proctab[currpid];

		struct memblk * memorylist;
		if(prptr->vmemlist != NULL){
		memorylist = &(prptr->vmemlist->mlist);
		uint32 free_mem = 0;
		for (memptr = memorylist->mnext; memptr != NULL;
							memptr = memptr->mnext) {
			free_mem += memptr->mlength;
		}
		//kprintf("%10d bytes of free memory.  Free list:\n", free_mem);
		for (memptr=memorylist->mnext; memptr!=NULL;memptr = memptr->mnext) {
		    kprintf("           [0x%08X to 0x%08X]\r\n",
			(uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);
			}
		}
		else
		{
			kprintf(" No vheap data ");
		}
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
