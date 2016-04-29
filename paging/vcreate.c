#include <xinu.h>
#include <kernel.h>
/*------------------------------------------------------------------------
 *  vcreate  - This system call creates a new XINU process.
 *  The difference from create() is that a process is provided with a private heap (i.e., not shared with other processes) that resides in virtual memory.
 *  The size of the heap (in number of pages, not bytes) is specified by hsize_in_pages.
 *------------------------------------------------------------------------
 */

syscall vcreate (int *procaddr, int ssize, int hsize_in_pages, int priority, char *name, int nargs, long args)
{
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		npid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	bsd_t nabs = EMPTY;

	mask = disable();
	if(hsize_in_pages < 1)
	{
		LOG(" Heap size in pages too small %d ", hsize_in_pages);
		restore(mask);
		return SYSERR;
	}

	/* get backend store for this process's vheap */

	if((nabs = (allocate_bs(hsize_in_pages))) == SYSERR)
	{
		LOG(" Error allocating a backend store for request. ");
		restore(mask);
		return SYSERR;
	}

	npid = create(procaddr, ssize, priority, name, nargs, args);
	if (do_bs_map(npid, STARTING_PAGE,nabs, hsize_in_pages)!=OK) {
		    LOG("Error executing bs_map()");
		    restore(mask);
		    return SYSERR;
	}
	prptr = &proctab[npid];
	kprintf(" Page directory for proc %d  is 0x%08x ", npid, prptr->pagedir );
	prptr->vpagestart = (STARTING_PAGE);
	prptr->vpagesize = hsize_in_pages;
	restore(mask);
	return npid;
}

