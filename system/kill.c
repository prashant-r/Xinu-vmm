/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	int32	i;			/* Index into descriptors	*/

	LOG(" Called kill ");
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);

	paging_related_destruction();

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		kprintf(" trying to kill active now %d ", currpid);
		resched();
		break;

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		kprintf(" trying to kill rec now %d ", currpid);
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		kprintf(" trying to kill wait now %d ", currpid);
		/* Fall through */
		break;
	case PR_READY:
		getitem(pid);		/* Remove from queue */
		kprintf(" trying to kill ready now %d ", currpid);
		/* Fall through */
		break;
	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}


void paging_related_destruction()
{
	// Remove any mappings that this process may have with the backing store
	backing_store_remove_mappings_for_pid(currpid);

	// Remove any frames corresponding to this process
	inverted_pagetable_remove_mappings_for_pid(currpid);

	// Remove the page directory of this process
	page_directory_frame_remove_mapping_for_pid(currpid);

}
