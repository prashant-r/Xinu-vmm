#include <xinu.h>
#include <limits.h>
frame_t frames[NFRAMES];
frame_t * fifo_head;

void initialize_all_frames(void)
{
	intmask mask;
	mask = disable();
	int frame = 0;
	fifo_head = NULL;
	frame_t *frameptr = NULL;
	for (frame = 0; frame < NFRAMES; ++frame) {
		frameptr = &frames[frame];
		initialize_frame(frameptr);
		frameptr->id = frame;
	}
	restore(mask);
	return;
}


void initialize_frame(frame_t * frameptr)
{
	intmask mask = disable();
	frameptr->backstore = INV_BS;
	frameptr->backstore_offset = INV_BS_OFF;
	frameptr->type = FREE;
	frameptr->vp_no = NULL;
	frameptr->refcount = 0;
	frameptr->next = NULL;
	frameptr->pid = -1;
	frameptr->age = 0;
	restore(mask);
}

void print_frame( frame_t * frameptr)
{

	intmask mask = disable();
	LOG("currpid %d id %d, pid %d, bs %d, off %d, type %d vp_no %d, refcount %d, age %d",currpid, frameptr->id, frameptr->pid, frameptr->backstore, frameptr->backstore_offset, frameptr->type, frameptr->vp_no, frameptr->refcount, frameptr->age);
}

frame_t * retrieve_new_frame(frame_type type)
{
	int frame = 0;
	frame_t *frameptr = NULL;
	frame_t *available_frame = NULL;
	frame_t *tmp = NULL;
	intmask mask;
	mask = disable();

	//1. Search the inverted page table for an empty frame. If one exists, stop.

	int FRAME_COUNT_LOWER_BOUND = (type == GPTBL) ? 1 : 0;
	int FRAM_COUNT_UPPER_BOUND = (type == GPTBL) ? NUM_GLOBAL_PAGE_TABLES+1 : NFRAMES-1;
	for (frame = FRAME_COUNT_LOWER_BOUND; frame <= FRAM_COUNT_UPPER_BOUND; ++frame) {
		frameptr = &frames[frame];
		if(frameptr->type == FREE){
			available_frame = frameptr;
			break;
		}
	}
	//2. Else, pick a page to replace (using the current replacement policy).

	if(available_frame == NULL)
	{
		//LOG(" No available_frames, must evict ");
		if(policy == FIFO)
			available_frame = evict_frame_using_fifo();
		else if(policy == AGING)
			available_frame = evict_frame_using_aging();
	}
	if(available_frame!= NULL)
	{
			//LOG(" available_frame type %d", available_frame->type);
			initialize_frame(available_frame);
			available_frame->type = type;
			available_frame->pid = currpid;

			//LOG(" TYPE IN ARGUMENT IS %d", type);
			//LOG(" available_frame type %d", available_frame->type);
			// Correct the fifo queue
			if(fifo_head == NULL)
			{
				fifo_head = available_frame;
				//LOG("FIFO head set to 0x%08x ", available_frame);
				//LOG(" available_frame type 23 %d", fifo_head->type);
			}
			else
			{
				frame_t * current = fifo_head;
				frame_t * previous = NULL;
				while(current)
				{
					previous = current;
					current = current->next;
				}
				previous->next = available_frame;
				//print_fifo_list();
				//LOG(" fifo_head set to what again? ", fifo_head);
			}
	}
	//LOG("Made it here too ");
	restore(mask);
	return available_frame;
}


frame_t * evict_frame_using_aging(void)
{
	intmask mask = disable();
	//LOG(" Evict frame using AGING");
	print_fifo_list();
	frame_t * frame = fifo_head;
	frame_t * selected = NULL;
		while(frame != NULL){
			if(frame->type == PAGE)
			{
				if(selected == NULL)
					selected = frame;
				else if(frame->age < selected ->age)
				{
					selected = frame;
				}
			}
			frame = frame->next;
		}
		if(selected != NULL)
		{
			//LOG("Free frame %d", frame->id);
			evict_from_fifo_list(selected);
			free_frame(selected);
		}
		restore(mask);
		//kprintf("\n Evicted id %d", selected ->id);
		return selected;
}




frame_t * evict_frame_using_fifo(void)
{
	intmask mask = disable();
	//LOG(" Evict frame using FIFO");
	frame_t * frame = fifo_head;
	while(frame != NULL){
		if(frame->type == PAGE)
		{
			evict_from_fifo_list(frame);
			break;
		}
		frame = frame->next;
	}
	if(frame != NULL)
	{
		//LOG("Free frame %d", frame->id);
		free_frame(frame);
	}
	restore(mask);
	return frame;

}

int free_frame(frame_t * frame)
{
	intmask mask;
	mask = disable();
	//LOG("Freeing");
	//print_frame(frame);
	if(frame->id <5)
	{
		LOG(" WHAT THE FUCK %d %d", frame->id, frame->type);
		restore(mask);
		return OK;
	}
	//kprintf("id %d type %d ", frame->id, frame->type);
	//print_fifo_list();
	//kprintf("\n");
	if(frame == NULL)
	{
		restore(mask);
		return SYSERR;
	}
	else if(!FRAMEID_IS_VALID(frame->id))
	{
		restore(mask);
		return SYSERR;
	}
	else if(frame->type == FREE)
	{
		restore(mask);
		return OK;
	}
	else if(frame->type == PAGE){

		//print_fifo_list();
		//LOG("Got here 0.5");
		//3. Using the inverted page table, get vp, the virtual page number of the page to be replaced.
		uint32 vp = frame->vp_no;
		//4. Let a be vp*4096 (the first virtual address on page vp).

		hook_pswap_out(vp, frame->id + FRAME0);
		uint32 a = vp*PAGE_SIZE;

		virtual_addr * virt = (virtual_addr *) &a;

		//5. Let p be the high 10 bits of a. Let q be bits [21:12] of a.
		uint32 p = virt->page_directory_offset;
		uint32 q = virt->page_table_offset;

		//6. Let pid be the process id of the process owning vp.
		pid32 pid = frame->pid;


		//7. Let pd point to the page directory of process pid.
		struct	procent	*prptr;		/* Ptr to process table entry	*/
		prptr = &proctab[pid];
		pd_t * pd = prptr->pagedir;

		if( pd == NULL)
		{
			LOG(" pd doesn't exist ");
			restore(mask);
			return SYSERR;
		}
		bool8 pt_pres = FALSE;
		pt_pres = (bool8) pd[p].pd_pres;

		bool8 pg_pres = FALSE;
		bool8 dirty = FALSE;
		if(pt_pres)
		{	//8. Let pt point to the pid's p_th page table.
			pt_t * pt = (pt_t *) ((pd[p].pd_base) * PAGE_SIZE);
			pg_pres = (bool8) pt[q].pt_pres;
			uint32 pg_base = (uint32) pt[q].pt_base;
			if(pg_pres){
				if((uint32)FRAMEID_TO_VPAGE(frame->id) == pg_base)
				{
					pg_pres = TRUE;
					dirty  = pt[q].pt_dirty;
				}
				else
				{
					pg_pres = FALSE;
				}
			}




		}
		if(pg_pres)
		{
			frame_t * pt_frame = &frames[(pd[p].pd_base) - FRAME0];
			pt_t * pt = (pt_t *) ((pd[p].pd_base) * PAGE_SIZE);
			//9. Mark the appropriate entry of pt as not present.
			pt[q].pt_pres = 0;
			if(pt_frame->type == VPTBL){
				decr_frame_refcount(pt_frame);
				if(pt_frame->refcount == 0){
					pd[p].pd_pres = 0;
					free_frame(pt_frame);

				}


				bzero((char *)&pt[q], sizeof(pt_t));
			}
			else if(pt_frame->type == GPTBL)
			{
			//	kprintf(" Uh  OH");
			}
			// If the reference count has reached zero, you should mark the appropriate entry in pd as "not present."
			// This conserves frames by keeping only page tables which are necessary.
			//LOG("Got here 1.5");

			//If the dirty bit for page vp was set in its page table, you must do the following:
			//a)	Using the backing store map, find the store and page offset within the store, given pid and a.
			//		If the lookup fails, something is wrong. Print an error message and kill the process with id pid.
			//b)	Write the page back to the backing store.

			//LOG("Got here 2");

			if(dirty){
				bsd_t bs_store_id;
				int bs_store_page_offset;
				if(SYSERR == bs_map_check(pid, vp, &bs_store_id, &bs_store_page_offset))
				{
						kprintf("FATAL :Can't find the bs_map");
						restore(mask);
						kill(currpid);
						return SYSERR;
				}
				//print_frame(frame);
				if(BACKSTORE_ID_IS_VALID(frame->backstore) && BACKSTORE_OFFSET_IS_VALID(frame->backstore_offset) && frame->backstore == bs_store_id && frame->backstore_offset == bs_store_page_offset)
				{
					//LOG("Frame %d was dirty", frame->id);
					open_bs(frame->backstore);
					write_bs(FRAMEID_TO_PHYSICALADDR(frame->id), frame->backstore, frame->backstore_offset);
					close_bs(frame->backstore);
				}
				//else
				//{
				//	print_frame(frame);
				//	kprintf("Fatal error: Cannot locate backstore for vpage %d to swap out page for pid %d ", vp, pid);
				//	kill(pid);
				//	initialize_frame(frame);
				//	restore(mask);
				//	return SYSERR;
				//}
			}

			else{
				//print_frame(frame);
			}
		}



		//LOG("Got here 1");
		//10. If the page being removed belongs to the current process,
		// invalidate the TLB entry for the page vp, using the invlpg instruction (see Intel Manual, volumes II and III for more details on this instruction).
		// 11. In the inverted page table, decrement the reference count of the frame occupied by pt.


		//LOG(" Free frame");
		//print_frame(frame);
		enable_paging();
		initialize_frame(frame);
		// Update page table entries associated with this frame
		// set the frame to be free
	}
	else if(frame->type == VPTBL)
	{
		evict_from_fifo_list(frame);
		hook_ptable_delete(frame->id + FRAME0);
		enable_paging();
		initialize_frame(frame);
	}
	else if(frame->type == DIR)
	{
		struct procent * prptrNULL = &proctab[NULLPROC];
		pd_t * null_pg_dir = prptrNULL->pagedir;
		struct	procent	*prptr;		/* Ptr to process table entry	*/
		prptr = &proctab[currpid];
		if(prptr->pagedir!= null_pg_dir)
		{
			evict_from_fifo_list(frame);
			prptr->pagedir = prptrNULL->pagedir;
			switch_page_directory(prptr->pagedir);
			enable_paging();
			initialize_frame(frame);

		}
	}
	restore(mask);
	return OK;

}

void print_fifo_list(void)
{
	frame_t * current = fifo_head;
	frame_t * previous = NULL;
	int count = 0;
	//kprintf("\n");
	while(current)
	{
		previous = current;
		//kprintf("\n | At %d frame %d owned by %d type %d age %d |", count++, current->id, current->pid, current->type, current->age);
		current = current->next;
	}
	//kprintf("\n");
}


void evict_from_fifo_list(frame_t * frameptr)
{
	intmask mask = disable();
	frame_t * previous;
	frame_t * current;
	frame_t * temp;
	previous = NULL;
	current = fifo_head;
	//LOG("NEVER CALLED THIS? 0x%08x", current);
	//LOG(" available_frame type 23 %d", fifo_head->type);

	//LOG(" BEFORE CORRECTION FIFO");
	//print_fifo_list();
	while (current != NULL)
	{
		if (current == frameptr) {
			temp = current;
			//LOG(" IN If condition, frame id %d", current->id);
			//print_fifo_list();
			if (previous != NULL) {
				previous->next = current->next;
				current = previous->next;
			} else {
				//LOG(" In else condition frame id %d", current->id);
				fifo_head = current->next;
				current = fifo_head;
			}
			//temp->type = FREE;
			break;
		}
		else{

			previous = current;
			current = current->next;
		}
	}
	restore(mask);
}

void incr_frame_refcount(frame_t * frameptr)
{
	intmask mask = disable();
	frameptr->refcount ++ ;
	restore(mask);
}

void decr_frame_refcount(frame_t * frameptr)
{
	intmask mask = disable();
	frameptr->refcount--;
	restore(mask);
}


int get_free_frame_count(void)
{
	intmask mask = disable();
	int count = 0;
	int fr_id;
	for (fr_id = 0; fr_id < NFRAMES; ++fr_id)
		if(frames[fr_id].type == FREE)
			count ++;
	restore(mask);
	return count;
}


int frame_map_check(int pid, int store, int page_offset_in_store, int * pageframe_id )
{
	intmask mask;
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)) {

		LOG(" Bad pid requested in bs_map_check: %d .", pid);
		restore(mask);
		return SYSERR;
	}
	if(store <0 || store >=BS_MAX_STORES)
	{
		LOG(" Bad store id in bs_map_check. Received : %d", store );
		restore(mask);
		return SYSERR;
	}
	if(pageframe_id == NULL)
	{
		LOG(" Bad pageframe_id. Not initialized. Must be set so it can be returned. ");
		restore(mask);
		return SYSERR;
	}
	if(page_offset_in_store <0 || page_offset_in_store > 200)
	{
		LOG(" Bad offset in bs store with id %d offset %d", store, page_offset_in_store);
		restore(mask);
		return SYSERR;
	}
	int fr_id;
	for (fr_id = 0; fr_id < NFRAMES; ++fr_id) {

		if (frames[fr_id].type == PAGE) {

			if(frames[fr_id].pid == pid && frames[fr_id].backstore == store && frames[fr_id].backstore_offset == page_offset_in_store)
			{
				//kprintf(" Was a  match %d, %d, %d, %d, %d, %d", frames[fr_id].pid, currpid, frames[fr_id].backstore, store, frames[fr_id].backstore_offset, page_offset_in_store);
				*pageframe_id = fr_id;
				restore(mask);
				return OK;
			}
		}
	}
	restore(mask);
	return EMPTY;


}


void update_frm_ages(void)
{
	  frame_t * frame;
	  frame_t * current;
	  current = fifo_head;
	  struct pentry * pptr;
	  while (current!= NULL) {
	        current->age = current->age >> 1;
	        if (frame_was_accessed(current)) {

	            current->age = current->age + 128;
	            if (current->age > INT_MAX/2)
	                current->age = INT_MAX/2;
	        }
	        current = current->next;
	    }

}


bool8 frame_was_accessed(frame_t * frame)
{
	pid32 pid = frame->pid;
	struct	procent	*prptr;
	prptr = &proctab[pid];
	pd_t * pd = prptr->pagedir;
	int page_dir_entry;
	int page_table_entry;
	bool8 accessed = FALSE;
	for(page_dir_entry = 5; page_dir_entry < PAGEDIRECTORY_ENTRIES_SIZE; page_dir_entry++){

		{
			if(pd[page_dir_entry].pd_pres)
			{
				pt_t * pt = (pt_t *) ((uint32)(pd[page_dir_entry].pd_base) << 12);
				for( page_table_entry = 0; page_table_entry < PAGETABLE_ENTRIES_SIZE; page_table_entry ++ )
				{
				//kprintf(" was accessed %d", pt[page_table_entry].pt_acc);
				 if (pt[page_table_entry].pt_pres && pt[page_table_entry].pt_acc) {
					 //kprintf(" This %d that %d", ((uint32)(&pt[page_table_entry])>>12), (FRAME0 + frame->id));
					 if(pt[page_table_entry].pt_base == (frame->id + FRAME0)){
					 accessed = TRUE;
					 pt[page_table_entry].pt_acc = 0;
				}


			}
		}
	}

		}
	}
	return accessed;
}
void inverted_pagetable_remove_mappings_for_pid(pid32 pid)
{
	intmask mask = disable();
	int i;
	frame_t * frame = NULL;
	for(i = 0; i < NFRAMES; i++)
	{
		frame = &frames[i];
		if(frame->type == FREE)
			continue;
		else if(frame->pid == pid && frame->type == VPTBL)
			free_frame(frame);
	}
	restore(mask);
	return;

}

void page_directory_frame_remove_mapping_for_pid(pid32 pid)
{
	intmask mask = disable();
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct procent *prptrNull;
	prptr = &proctab[pid];
	prptrNull = &proctab[0];
	pd_t * pd = prptr->pagedir;
	int pdframe = PA_TO_FRAMEID((uint32)pd);
	free_frame(&frames[pdframe]);
	//kprintf(" REM MAPPING dir from %d: 0x%08x to %d: 0x%08x",pid, prptr->pagedir, 0, prptrNull->pagedir);
	//kprintf(" tried to kill the page directory");
	restore(mask);
	return;
}


