#include <xinu.h>

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
	frameptr->backstore = EMPTY;
	frameptr->backstore_offset = EMPTY;
	frameptr->type = FREE;
	frameptr->dirty = FALSE;
	frameptr->vp_no = NULL;
	frameptr->refcount = 0;
	frameptr->referenced = FALSE;
	frameptr->next = NULL;
	restore(mask);
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
	if(type == PPTBLCAT2)
	{
		frameptr = &frames[DEVICE_FRAME];
		available_frame = frameptr;
	}
	else{
		int FRAME_COUNT_LOWER_BOUND = (type == PPTBLCAT1) ? 1 : 0;
		int FRAM_COUNT_UPPER_BOUND = (type == PPTBLCAT1) ? NUM_GLOBAL_PAGE_TABLES : NFRAMES-1;
		for (frame = FRAME_COUNT_LOWER_BOUND; frame <= FRAM_COUNT_UPPER_BOUND; ++frame) {
			frameptr = &frames[frame];
			if(frameptr->type == FREE){
				available_frame = frameptr;
				break;
			}
		}
	}
	//2. Else, pick a page to replace (using the current replacement policy).

	if(available_frame == NULL)
	{
		LOG(" No available_frames, must evict ");
		if(policy == FIFO)
			available_frame = evict_frame_using_fifo();
		else if(policy == AGING)
			available_frame = evict_frame_using_aging();
	}
	if(available_frame!= NULL)
	{
		initialize_frame(available_frame);
		//LOG(" available_frame type %d", available_frame->type);
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
	return NULL;
}

frame_t * evict_frame_using_fifo(void)
{
	LOG(" Evict frame using FIFO");
	frame_t * frame = fifo_head;
	while(frame != NULL){
		if(frame->type == PAGE)
			break;
		frame = frame->next;
	}
	if(frame != NULL)
	{
		LOG("Free frame %d", frame->id);
		free_frame(frame);
	}
	return frame;
}

int free_frame(frame_t * frame)
{
	intmask mask;
	mask = disable();
	LOG("\n Request to free frame with %d \n", frame->id);
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
		frame->type = FREE;
		correct_fifo_list();
		LOG("Got here 0.5");
		//3. Using the inverted page table, get vp, the virtual page number of the page to be replaced.
		uint32 vp = frame->vp_no;
		//4. Let a be vp*4096 (the first virtual address on page vp).
		uint32 a = vp *PAGE_SIZE;

		virtual_addr * virt = &a;

		//5. Let p be the high 10 bits of a. Let q be bits [21:12] of a.
		uint32 p = virt->page_directory_offset;
		uint32 q = virt->page_table_offset;

		//6. Let pid be the process id of the process owning vp.
		pid32 pid = frame->pid;

		//7. Let pd point to the page directory of process pid.
		struct	procent	*prptr;		/* Ptr to process table entry	*/
		prptr = &proctab[pid];
		pd_t * pd = prptr->pagedir;

		//8. Let pt point to the pid's p_th page table.
		pt_t * pt = (pt_t *) ((pd[p].pd_base) * PAGE_SIZE);

		//9. Mark the appropriate entry of pt as not present.
		pt[q].pt_pres = 0;
		LOG("Got here 1");
		//10. If the page being removed belongs to the current process,
		// invalidate the TLB entry for the page vp, using the invlpg instruction (see Intel Manual, volumes II and III for more details on this instruction).
		if(pid == currpid)
		{
			invlpg((void *)a);
		}

		// 11. In the inverted page table, decrement the reference count of the frame occupied by pt.
		frame_t * pt_frame = &frames[(pd[p].pd_base) - FRAME0];
		//decr_frame_refcount(pt_frame);

		// If the reference count has reached zero, you should mark the appropriate entry in pd as "not present."
		// This conserves frames by keeping only page tables which are necessary.
		//if(pt_frame->refcount == 0)
		//{
			//pd[p].pd_pres = 0;
			//flush_tlb();
		//}
		LOG("Got here 1.5");

		//If the dirty bit for page vp was set in its page table, you must do the following:
		//a)	Using the backing store map, find the store and page offset within the store, given pid and a.
		//		If the lookup fails, something is wrong. Print an error message and kill the process with id pid.
		//b)	Write the page back to the backing store.
		bool8 dirty = FALSE;
		if (pt[q].pt_dirty)
			dirty = TRUE;

		bzero((char *)&pt[q], sizeof(pt_t));
		LOG("Got here 2");
		if(dirty)
			if(BACKSTORE_ID_IS_VALID(frame->backstore))
			{
				LOG("Frame %d was dirty", frame->id);
				write_bs(FRAMEID_TO_PHYSICALADDR(frame->id), frame->backstore, frame->backstore_offset);
				initialize_frame(frame);
				restore(mask);
				return OK;
			}
			else
			{
				LOG("Fatal error: Cannot locate backstore for vpage %d to swap out page for pid %d ", vp, pid);
				kill(pid);
				initialize_frame(frame);
				restore(mask);
				return OK;
			}
		// Update page table entries associated with this frame
		// set the frame to be free
	}
	else{
		frame->type = FREE;
		correct_fifo_list();
		initialize_frame(frame);
	}
	restore(mask);
	return OK;

}

void print_fifo_list(void)
{
	frame_t * current = fifo_head;
	frame_t * previous = NULL;
	int count = 0;
	while(current)
	{
		previous = current;
		kprintf("At %d frame %d owned by %d type %d \n", count++, current->id, current->pid, current->type);
		current = current->next;
	}
}


void correct_fifo_list(void)
{
	frame_t * previous;
	frame_t * current;
	previous = NULL;
	current = fifo_head;
	//LOG("NEVER CALLED THIS? 0x%08x", current);
	//LOG(" available_frame type 23 %d", fifo_head->type);

	//LOG(" BEFORE CORRECTION FIFO");
	//print_fifo_list();
	while (current != NULL)
	{
		if (current->type == FREE) {
			LOG(" IN If condition, frame id %d", current->id);
			//print_fifo_list();
			if (previous != NULL) {
				previous->next = current->next;
				current = previous->next;
			} else {
			LOG(" In else condition frame id %d", current->id);
				fifo_head = current->next;
				current = fifo_head;
			}
		}
		else{

			previous = current;
			current = current->next;
		}
	}
	//LOG("AFTER CORECTION FIFO");
	//print_fifo_list();
}

void incr_frame_refcount(frame_t * frameptr)
{
	frameptr->refcount ++ ;
}

void decr_frame_refcount(frame_t * frameptr)
{
	frameptr->refcount--;
	kprintf(" Its a %d", frameptr->type);
	kprintf(" Current ref count is %d", frameptr->refcount);
	if(frameptr->type == VPTBL)
	{
		kprintf(" Its a page table");
		kprintf(" Current ref count is %d", frameptr->refcount);
	}
	else if(frameptr->type ==PAGE)
	{
		kprintf(" Its a page");
		kprintf(" Current ref count is %d", frameptr->refcount);
	}
	if(frameptr->refcount == 0)
		free_frame(frameptr);
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

			if(frames[fr_id].pid == currpid && frames[fr_id].backstore == store && frames[fr_id].backstore_offset == page_offset_in_store)
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

}

