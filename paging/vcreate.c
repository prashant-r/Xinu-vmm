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
	prptr = &proctab[npid];
	prptr->pagedir = retrieve_new_page_directory();
	if (do_bs_map(npid, STARTING_PAGE,nabs, hsize_in_pages)!=OK) {
	    LOG("Error executing bs_map()");
	    restore(mask);
	    return SYSERR;
	}

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

	int pa, pb, pc, pd, pe, pf;
    pa = vcreate(vcprocA, 1024, 200, 20, "procA", 0, 0);
    //pb = vcreate(vcprocB, 1024, 200, 20, "procB",0,0);
    //pc = vcreate(vcprocC, 1024, 200, 20, "procC",0,0);
    pd = vcreate(vcprocD, 1024, 200, 20, "procD", 0, 0);
    //pe = vcreate(vcprocE, 1024, 200, 20, "procE", 0, 0);
    pf = vcreate(vcprocF, 1024, 200, 20, "procF", 0, 0);
    kprintf("Number of free frames are  %d ", get_free_frame_count());
    resume(pa);
    resume(pd);
    resume(pf);
    return;
}


void vcprocA(void)
{
	char *tmp = (char *)0x01000001;
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
	*tmp = 'A';
	//addressTranslate(tmp);
	//printMemory();
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	int count = (200 * PAGE_SIZE)-8;
	tmp = vgetmem(count);
	//kprintf(" Address provided is 0x%08x", tmp);
	//printMemory();
	int a;
	//kprintf(" Made it here");
	for(a =0; a < count; a++)
	{
		tmp[a] = 'N';
	}
	//LOG("Number of free frames are  %d ", get_free_frame_count());
	int testcount = 0;
	for (a = 0; a< count; a++)
		if(tmp[a] == 'N')
			testcount ++ ;

	if(testcount == count)
		kprintf("\n vcprocA has correct values \n");
	else
		kprintf("\n vcprocA has incorrect values %d!= %d \n", testcount, count);

	//vfreemem(tmp,200*PAGE_SIZE);
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
}

void vcprocB(void)
{
	char *tmp = (char *)0x01110001;
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	*tmp = 'G';
	addressTranslate(tmp);
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	tmp = vgetmem(sizeof(char));
	*tmp = 'H';
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	tmp = vgetmem(sizeof(char));
	*tmp ='I';
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	vfreemem(tmp, sizeof(char));
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
}
void vcprocC(void)
{
	char *tmp = (char *)0x01000001;
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
	*tmp = 'A';
	//addressTranslate(tmp);
	//printMemory();
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	tmp = vgetmem((200 * PAGE_SIZE)-8);
	//kprintf(" Address provided is 0x%08x", tmp);
	//printMemory();
	int a;
	//kprintf(" Made it here");
	for(a =0; a < (200 *PAGE_SIZE)-8; a++)
	{
		tmp[a] = 'Y';
	}
	//vfreemem(tmp,200*PAGE_SIZE);
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
}

void vcprocD(void)
{
	char *tmp = (char *)0x01000001;
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
	*tmp = 'A';
	//addressTranslate(tmp);
	//printMemory();
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	int count = (200 * PAGE_SIZE)-8;
	tmp = vgetmem(count);
	//kprintf(" Address provided is 0x%08x", tmp);
	//printMemory();
	int a;
	//kprintf(" Made it here");
	for(a =0; a < count; a++)
		tmp[a] = 'G';
	//LOG("Number of free frames are  %d ", get_free_frame_count());
	int testcount = 0;
	for (a = 0; a< count; a++)
		if(tmp[a]== 'G')
			testcount ++ ;
	if(testcount == count)
		kprintf("\n vcprocD has correct values \n");
	else
		kprintf("\n vcprocD has incorrect values %d!= %d \n", testcount, count);

	//vfreemem(tmp,200*PAGE_SIZE);
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
}
void vcprocE(void)
{
	char *tmp = (char *)0x01000001;
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
	*tmp = 'A';
	//addressTranslate(tmp);
	//printMemory();
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	int count = (200 * PAGE_SIZE)-8;
	tmp = vgetmem(count);
	//kprintf(" Address provided is 0x%08x", tmp);
	//printMemory();
	int a;
	//kprintf(" Made it here");
	for(a =0; a < count; a++)
		tmp[a] = 'T';
	//LOG("Number of free frames are  %d ", get_free_frame_count());
	int testcount = 0;
	for (a = 0; a< count; a++)
		if(tmp[a]== 'T')
			testcount ++ ;
	if(testcount == count)
		kprintf("\n vcprocE has correct values \n");
	else
		kprintf("\n vcprocE has incorrect values %d!= %d \n", testcount, count);

	//vfreemem(tmp,200*PAGE_SIZE);
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
}

void vcprocF(void)
{
	char *tmp = (char *)0x01000001;
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
	*tmp = 'A';
	//addressTranslate(tmp);
	//printMemory();
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	int count = (200 * PAGE_SIZE)-8;
	tmp = vgetmem(count);
	//kprintf(" Address provided is 0x%08x", tmp);
	//printMemory();
	int a;
	//kprintf(" Made it here");
	for(a =0; a < count; a++)
		tmp[a] = 'Y';
	//LOG("Number of free frames are  %d ", get_free_frame_count());
	int testcount = 0;
	for (a = 0; a< count; a++)
		if(tmp[a]== 'Y')
			testcount ++ ;
	if(testcount == count)
		kprintf("\n vcprocF has correct values \n");
	else
		kprintf("\n vcprocF has incorrect values %d!= %d \n", testcount, count);

	//vfreemem(tmp,200*PAGE_SIZE);
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
}



