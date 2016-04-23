#include <xinu.h>

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
	prptr->vpagestart = (STARTING_PAGE);
	prptr->vpagesize = hsize_in_pages;
	restore(mask);
	return npid;
}

//-----------------------------------------------------------------------
// Tests for vcreate API
//-----------------------------------------------------------------------
void vcreate_tests()
{
	basicTest();
}

void basicTest()
{
	int pa, pb, pc;
    pa = vcreate(vcprocA, 1024, 200, 20, "procA", 0, 0);
    pb = vcreate(vcprocB, 1024, 200, 20, "procB",0,0);
    pc = vcreate(vcprocC, 1024, 200, 20, "procC",0,0);
    //resume(pb);
    resume(pb);
    //resume(pc);
    return;
}


void vcprocA(void)
{
	char *tmp = (char *)0x01000001;
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
	*tmp = 'A';
	//addressTranslate(tmp);
	//printMemory();
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	tmp = vgetmem(200*PAGE_SIZE);
	printMemory();
	int a;

	for(a =0; a <200*PAGE_SIZE; a++)
	{
		tmp[a] = 'Y';
	}

	//vfreemem(tmp,200*PAGE_SIZE);
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
}

void vcprocB(void)
{
	char *tmp = (char *)0x01110001;
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	*tmp = 'G';
	addressTranslate(tmp);
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	tmp = vgetmem(sizeof(char));
	*tmp = 'H';
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	tmp = vgetmem(sizeof(char));
	*tmp ='I';
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	vfreemem(tmp, sizeof(char));
	kprintf("Character at address %d is %c \n",tmp, *tmp);
}
void vcprocC(void)
{
	char *tmp = (char *)0x01000001;
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	*tmp = 'G';
	addressTranslate(tmp);
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	tmp = vgetmem(sizeof(char));
	*tmp = 'H';
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	tmp = vgetmem(sizeof(char));
	*tmp ='I';
	kprintf("Character at address %d is %c \n",tmp, *tmp);
	vfreemem(tmp, sizeof(char));
	kprintf("Character at address %d is %c \n",tmp, *tmp);
}
