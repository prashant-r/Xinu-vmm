#include <xinu.h>
#include <kernel.h>
/*------------------------------------------------------------------------
 *  vcreate  - This system call creates a new XINU process.
 *  The difference from create() is that a process is provided with a private heap (i.e., not shared with other processes) that resides in virtual memory.
 *  The size of the heap (in number of pages, not bytes) is specified by hsize_in_pages.
 *------------------------------------------------------------------------
 */

#define	roundew(x)	( (x+3)& ~0x3)

syscall vcreate (int *procaddr, int ssize, int hsize_in_pages, int priority, char *name, uint32 nargs, ...)
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

	//kprintf("whose creating stack %d ", currpid );
	uint32		savsp, *pushsp;
		int32		i;
		uint32		*a;		/* Points to list of args	*/
		uint32		*saddr;		/* Stack address		*/
		if (ssize < MINSTK)
				ssize = MINSTK;
			ssize = (uint32) roundew(ssize);
			if (((saddr = (uint32 *)getstk(ssize)) ==
			    (uint32 *)SYSERR ) ||
			    (npid=newpid()) == SYSERR || priority < 1 ) {
				restore(mask);
				return SYSERR;
			}

			prcount++;
			prptr = &proctab[npid];

			/* Initialize process table entry for new process */

			prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
			prptr->prprio = priority;
			prptr->prstkbase = (char *)saddr;
			prptr->prstklen = ssize;
			prptr->prname[PNMLEN-1] = NULLCH;
			for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
				;
			prptr->prsem = -1;
			prptr->prparent = (pid32)getpid();
			prptr->prhasmsg = FALSE;
			/* Set up stdin, stdout, and stderr descriptors for the shell	*/
			prptr->prdesc[0] = CONSOLE;
			prptr->prdesc[1] = CONSOLE;
			prptr->prdesc[2] = CONSOLE;

			/* Initialize stack as if the process was called		*/

			*saddr = STACKMAGIC;
			savsp = (uint32)saddr;

			/* Push arguments */
			a = (uint32 *)(&nargs + 1);	/* Start of args		*/
			a += nargs -1;			/* Last argument		*/
			for ( ; nargs > 0 ; nargs--)	/* Machine dependent; copy args	*/
				*--saddr = *a--;	/*   onto created process' stack*/
			*--saddr = (long)INITRET;	/* Push on return address	*/

			/* The following entries on the stack must match what ctxsw	*/
			/*   expects a saved process state to contain: ret address,	*/
			/*   ebp, interrupt mask, flags, registerss, and an old SP	*/

			*--saddr = (long)procaddr;	/* Make the stack look like it's*/
							/*   half-way through a call to	*/
							/*   ctxsw that "returns" to the*/
							/*   new process		*/
			*--saddr = savsp;		/* This will be register ebp	*/
							/*   for process exit		*/
			savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
			*--saddr = 0x00000200;		/* New process runs with	*/
							/*   interrupts enabled		*/

			/* Basically, the following emulates an x86 "pushal" instruction*/

			*--saddr = 0;			/* %eax */
			*--saddr = 0;			/* %ecx */
			*--saddr = 0;			/* %edx */
			*--saddr = 0;			/* %ebx */
			*--saddr = 0;			/* %esp; value filled in below	*/
			pushsp = saddr;			/* Remember this location	*/
			*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
			*--saddr = 0;			/* %esi */
			*--saddr = 0;			/* %edi */
			*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
			// For demand paging
	if (do_bs_map(npid, STARTING_PAGE,nabs, hsize_in_pages)!=OK) {
		    LOG("Error executing bs_map()");
		    restore(mask);
		    return SYSERR;
	}
	prptr = &proctab[npid];
	prptr->vpagestart = (STARTING_PAGE);
	prptr->vpagesize = hsize_in_pages;
	prptr->pagedir = retrieve_new_page_directory();
	//LOG(" Page directory for proc %d  is 0x%08x ", npid, prptr->pagedir );
	restore(mask);
	return npid;
}
