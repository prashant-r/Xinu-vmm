/* initialize.c - nulluser, sysinit, sizmem */

/* Handle system initialization and become the null process */

#include <xinu.h>
#include <string.h>

extern	void	start(void);	/* Start of Xinu code			*/
extern	void	*_end;		/* End of Xinu code			*/

/* Function prototypes */

extern	void main(void);	/* Main is the first process created	*/
extern	void xdone(void);	/* System "shutdown" procedure		*/
static	void sysinit(); 	/* Internal system initialization	*/
extern	void meminit(void);	/* Initializes the free memory list	*/

/* Declarations of major kernel variables */

struct	procent	proctab[NPROC];	/* Process table			*/
struct	sentry	semtab[NSEM];	/* Semaphore table			*/
struct	memblk	memlist;	/* List of free memory blocks		*/

/* Active system status */

int	prcount;		/* Total number of live processes	*/
pid32	currpid;		/* ID of currently executing process	*/

bool8   PAGE_SERVER_STATUS;    /* Indicate the status of the page server */
sid32   bs_init_sem;

void pagefault_interrupt(void);
int32	set_evec(uint32 xnum, uint32 handler);

/*------------------------------------------------------------------------
 * nulluser - initialize the system and become the null process
 *
 * Note: execution begins here after the C run-time environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  The code turns itself into the null process
 * after initialization.  Because it must always remain ready to execute,
 * the null process cannot execute code that might cause it to be
 * suspended, wait for a semaphore, put to sleep, or exit.  In
 * particular, the code must not perform I/O except for polled versions
 * such as kprintf.
 *------------------------------------------------------------------------
 */

void	nulluser()
{	
	struct	memblk	*memptr;	/* Ptr to memory block		*/
	uint32	free_mem;		/* Total amount of free memory	*/
	
	/* Initialize the system */
		
	sysinit();
    
    // initialize paging
    initialize_paging();
    
	kprintf("\n\r%s\n\n\r", VERSION);
	
	/* Output Xinu memory layout */
	free_mem = 0;
	for (memptr = memlist.mnext; memptr != NULL;
						memptr = memptr->mnext) {
		free_mem += memptr->mlength;
	}
	kprintf("%10d bytes of free memory.  Free list:\n", free_mem);
	for (memptr=memlist.mnext; memptr!=NULL;memptr = memptr->mnext) {
	    kprintf("           [0x%08X to 0x%08X]\r\n",
		(uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);
	}

	kprintf("%10d bytes of Xinu code.\n",
		(uint32)&etext - (uint32)&text);
	kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)&text, (uint32)&etext - 1);
	kprintf("%10d bytes of data.\n",
		(uint32)&ebss - (uint32)&data);
	kprintf("           [0x%08X to 0x%08X]\n\n",
		(uint32)&data, (uint32)&ebss - 1);

	/* Create the RDS process */

	rdstab[0].rd_comproc = create(rdsprocess, RD_STACK, RD_PRIO,
					"rdsproc", 1, &rdstab[0]);
	if(rdstab[0].rd_comproc == SYSERR) {
		panic("Cannot create remote disk process");
	}
	resume(rdstab[0].rd_comproc);

	/* Enable interrupts */

	enable();

	/* Create a process to execute function main() */

	resume (
	   create((void *)main, INITSTK, INITPRIO, "Main process", 0,
           NULL));

	/* Become the Null process (i.e., guarantee that the CPU has	*/
	/*  something to run when no other process is ready to execute)	*/

	while (TRUE) {
		;		/* Do nothing */
	}

}

/*------------------------------------------------------------------------
 *
 * sysinit  -  Initialize all Xinu data structures and devices
 *
 *------------------------------------------------------------------------
 */
static	void	sysinit()
{
	int32	i;
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct	sentry	*semptr;	/* Ptr to semaphore table entry	*/

	/* Platform Specific Initialization */

	platinit();

	/* Initialize the interrupt vectors */

	initevec();
	
	/* Initialize free memory list */
	
	meminit();

	/* Initialize system variables */

	/* Count the Null process as the first process in the system */

	prcount = 1;

	/* Scheduling is not currently blocked */

	Defer.ndefers = 0;

	pagefaults = 0;
	initialize_all_frames(); // Initialize all necessary data structures.
	pd_t * null_pg_dir = retrieve_new_page_directory();
	//kprintf(" The page dir is 0x%08x", null_pg_dir);
	for (i = 0; i < NPROC; i++) {

		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->prprio = 0;
		prptr->vpagesize = 0;
		prptr->vpagestart =0;
		prptr->pagedir = null_pg_dir;
		prptr->vmemlist = NULL;
	}


	/* Initialize process table entries free */
		prptr = &proctab[NULLPROC];
		prptr->prstate = PR_CURR;
		prptr->prprio = 0;
		strncpy(prptr->prname, "prnull", 7);
		prptr->prstkbase = getstk(NULLSTK);
		prptr->prstklen = NULLSTK;
		prptr->prstkptr = 0;
		prptr->vpagesize = 0;
		prptr->vpagestart =0;
		prptr->vmemlist = NULL;
		prptr->pagedir = null_pg_dir;
		//kprintf(" The page dir is 0x%08x", prptr->pagedir);



	/* Initialize the Null process entry */	

	currpid = NULLPROC;
	
	/* Initialize semaphores */

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->sstate = S_FREE;
		semptr->scount = 0;
		semptr->squeue = newqueue();
	}

	/* Initialize buffer pools */

	bufinit();

	/* Create a ready list for processes */

	readylist = newqueue();

	/* Initialize the real time clock */

	clkinit();

	for (i = 0; i < NDEVS; i++) {
		init(i);
	}

    PAGE_SERVER_STATUS = PAGE_SERVER_INACTIVE;
    bs_init_sem = semcreate(1);

	return;
}


void initialize_paging(void)
{
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	prptr= &proctab[NULLPROC];
    initialize_global_pagetables();//Create the page tables which map pages 0 through 4095 to the 16 MB physical address range.
                                   //We will call these global page tables.
	
	initialize_device_pagetable(); // Create a page table for mapping the device memory starting at 0x90000000.
                                   //Need to map 1024 pages starting at this address.
                                   //This means you will need to fill page directory entry at index #576 (0x90000000 >> 22) and
                                   //all the entries in the corresponding page table
	
	//kprintf(" The page dir is 0x%08x", prptr->pagedir);
	switch_page_directory((unsigned long)prptr->pagedir); // Set the PDBR register to the page directory of the null process.
	
	set_evec(14,(uint32)pagefault_interrupt); // Install the page fault interrupt service routine.

	enable_paging(); //Enable paging.
}

int32	stop(char *s)
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* Empty */;
}

int32	delay(int n)
{
	DELAY(n);
	return OK;
}
