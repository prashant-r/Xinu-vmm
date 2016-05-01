#include <xinu.h>

int do_bs_map(int pid, uint32 vp_no, bsd_t bs_id, int npages)
{
	intmask mask ;
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)) {

		LOG(" Bad pid requested in do_bs_map: %d .", pid);
		restore(mask);
		return SYSERR;
	}
	if(vp_no < STARTING_PAGE)
	{
		LOG(" Bad virtual page no. in do_bs_map: %d", vp_no);
		restore(mask);
		return SYSERR;
	}
	if(bs_id <0 || bs_id >=BS_MAX_STORES)
	{
		LOG(" Bad bs_id requested in do_bs_map: %d", bs_id);
		restore(mask);
		return SYSERR;
	}
	if(npages < 0  || npages > BS_MAX_PAGES)
	{
		LOG(" Too many pages requested for do_bs_map. Max is 200. Requested : %d", npages);
		restore(mask);
		return SYSERR;
	}
	bstab[bs_id].isallocated = TRUE;
	bstab[bs_id].pid = pid;
	bstab[bs_id].vp_no = vp_no;
	bstab[bs_id].npages = npages;
	restore(mask);
	return OK;
}

int bs_map_check(int pid, uint32 vpage, int * store, int * page_offset_in_store )
{
	intmask mask;
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)) {

		LOG(" Bad pid requested in bs_map_check: %d .", pid);
		restore(mask);
		return SYSERR;
	}
	if(store == NULL || page_offset_in_store == NULL)
	{
		LOG( " Must set store and page_offset_in_store to some address so it can be returned. ");
		restore(mask);
		return SYSERR;
	}
	if(vpage < STARTING_PAGE)
	{
		LOG(" Bad virtual address in bs_map_check %d", vpage);
		restore(mask);
		return SYSERR;
	}
	int bs_id;
	for (bs_id = 0; bs_id < BS_MAX_STORES; ++bs_id) {

	  if (bstab[bs_id].isallocated == TRUE) {
		  if(bstab[bs_id].pid == pid && vpage >= bstab[bs_id].vp_no  && vpage <= bstab[bs_id].vp_no + bstab[bs_id].npages)
		  {
			  *store = bs_id;
			  //LOG(" vpage is %d ",vpage);
			  //LOG(" bstab vp_no %d", bstab[bs_id].vp_no);
	      	  *page_offset_in_store = vpage - bstab[bs_id].vp_no;
	      	  //LOG(" page offset in store %d ", *page_offset_in_store);
	      	  restore(mask);
	      	  return OK;
		  }

	    }
	}
	restore(mask);
	return SYSERR;

}

void backing_store_remove_mappings_for_pid(pid32 pid)
{
	int bs_id;
	intmask mask = disable();
	for (bs_id = 0; bs_id < BS_MAX_STORES; ++bs_id) {

	  if (bstab[bs_id].isallocated == TRUE) {
		  //LOG(" bstab allocated %d this pid %d pid %d vpage %d vp_no %d", bs_id,pid,  bstab[bs_id].pid,vpage,  bstab[bs_id].vp_no );
		  if(bstab[bs_id].pid == pid)
		  {
			  deallocate_bs(bs_id);
			  restore(mask);
	      	  return;
		  }

	    }
	}
	restore(mask);
	return;
}

