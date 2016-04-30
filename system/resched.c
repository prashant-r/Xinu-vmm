/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];
	pid32 oldpid = currpid;

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	if(oldpid == 6 && ptold->prstate == PR_FREE)
		{
		 	kprintf("\n Atleast its true %d state is going to switch to %d  ", ptold->prstate, currpid);
		}
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	

	//kprintf(" old 0x%08x new 0x%08x", ptold->pagedir, ptnew->pagedir);
 	switch_page_directory(ptnew->pagedir);

 	if(oldpid == 6 && ptold->prstate == PR_FREE)
 	{
 		kprintf("\n CHECK page dir from %d: 0x%08x to %d: 0x%08x whose state is %d ",oldpid, ptold->pagedir, currpid, ptnew->pagedir, ptnew->prstate);
 	}
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	enable_paging();
	/* Old process returns here when resumed */
	if(oldpid == 6 && ptold->prstate == PR_FREE && currpid == 7)
		kprintf("\n New process returned ");
	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
