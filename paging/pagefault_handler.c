#include <xinu.h>
syscall pagefault_handler(void)
{
	intmask mask;
	mask = disable();
	int bs_store_id;
	int bs_store_page_offset;
	unsigned int fault_address = read_cr2();

	//LOG("handler with add 0x%08x", fault_address);

	if(policy == AGING)
		update_frm_ages();

	
	//LOG(" Fault address is 0x%08x", fault_address);

	virtual_addr * vir_add  = NULL;

	vir_add = (virtual_addr *) & fault_address;


	uint32 pg_off  = (uint32) vir_add->page_offset;
	uint32 pgt_off = (uint32) vir_add->page_table_offset;
	uint32 pgd_off = (uint32) vir_add->page_directory_offset;

	if(SYSERR == bs_map_check(currpid, fault_address, &bs_store_id, &bs_store_page_offset))
	{
		LOG(" Accessed an illegal memory address 0x%08x ", fault_address);
		restore(mask);
		kill(currpid);
		return SYSERR;
	}
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	prptr = &proctab[currpid];
	frame_t * frame = NULL;
	pt_t * ptab = NULL;
	pd_t * pgdir =prptr->pagedir;
	int frame_id;
	// check if the page table is present
	if(pgdir[pgd_off].pd_pres == 1)
	{
		//LOG(" Page table is present ");
		frame_id = (pgdir[pgd_off].pd_base) - FRAME0;
		//LOG(" Page table frame id in present is %d", frame_id);
		ptab = FRAMEID_TO_PHYSICALADDR(frame_id);
	}
	else
	{
		//LOG(" Page table is NOT present");
		ptab = retrieve_new_page_table(VPTBL);
		frame = &frames[PA_TO_FRAMEID(ptab)];
		frame_id = frame->id;
		frame->vp_no = fault_address >>12;
		frame->backstore = bs_store_id;
		frame->backstore_offset = bs_store_page_offset;
		//LOG(" Frame id in absent is %d", frame_id);
	    if(ptab == NULL)
		{
			LOG(" Replacement policy not working correctly! No available frames for ptable.");
			restore(mask);
			//kill(currpid);
			return SYSERR;
		 }
		 int k;
		 for(k=0; k<1024; k++)
       	 {     
            ptab[k].pt_pres = 0;
            ptab[k].pt_write = 1;
            ptab[k].pt_base = 0;
       	 }
		 pgdir[pgd_off].pd_pres = 1;
		 pgdir[pgd_off].pd_write = 1;
		 pgdir[pgd_off].pd_base = FRAME0 + frame_id;
	}
	int pageframe_id ;
	frame_t * pageframe = NULL;

	//LOG(" PAGEFAULT GOING OK");
	// Put page information in frame
	int frame_map_check_result = frame_map_check(currpid, bs_store_id, bs_store_page_offset, &pageframe_id);
	if(frame_map_check_result == EMPTY)
	{
		//LOG(" Frame map check returned empty. ");
		pageframe = retrieve_new_frame(PAGE);
		if(pageframe == NULL)
		{
			LOG(" Replacement policy not working correctly! No available frames for pframe.");
			restore(mask);
			//kill(currpid);
			return SYSERR;
		}
		pageframe_id = pageframe->id;
		//LOG(" TRYING BACKEND NOW");
		//LOG(" Trying backend store for frame id %d, bs store id %d, bs page offset %d", pageframe_id, bs_store_id, bs_store_page_offset);
		if (SYSERR == read_bs((char *) FRAMEID_TO_PHYSICALADDR(pageframe_id), bs_store_id, bs_store_page_offset)) {
			LOG(" Reading backend store for frame id %d, bs store id %d, bs page offset %d failed", pageframe_id, bs_store_id, bs_store_page_offset);
			restore(mask);
			return SYSERR;
		}
		pageframe->backstore = bs_store_id;
		pageframe->backstore_offset = bs_store_page_offset;
		pageframe->vp_no = fault_address >>12;
	}
	else if( frame_map_check_result == OK)
	{
		//LOG(" Frame map check returned OK. ");
		pageframe = &frames[pageframe_id];
		pageframe->refcount++;

	}
	else
	{
		LOG(" Error: Frame map check executed with fatal error. ");
		restore(mask);
		kill(currpid);
		return SYSERR;
	}
	//LOG(" Page frame obtained %d", pageframe_id);
	ptab[pgt_off].pt_base = FRAME0 + pageframe_id;
	ptab[pgt_off].pt_pres = 1;
	ptab[pgt_off].pt_write = 1;

	frame->refcount ++;
	//LOG(" Going to enable paging ");
	flush_tlb();
	restore(mask);
	return OK;
}
