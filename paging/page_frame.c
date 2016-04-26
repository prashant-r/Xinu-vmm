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
	frameptr->backstore = INV_BS;
	frameptr->backstore_offset = INV_BS_OFF;
	frameptr->type = FREE;
	frameptr->dirty = FALSE;
	frameptr->vp_no = NULL;
	frameptr->refcount = 0;
	frameptr->referenced = FALSE;
	frameptr->next = NULL;
	frameptr->pid = -1;
	restore(mask);
}

void print_frame( frame_t * frameptr)
{

	intmask mask = disable();
	kprintf("\n");
	kprintf("currpid %d id %d, pid %d, bs %d, off %d, type %d vp_no %d, refcount %d",currpid, frameptr->id, frameptr->pid, frameptr->backstore, frameptr->backstore_offset, frameptr->type, frameptr->vp_no, frameptr->refcount);
	kprintf("\n");
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
		else if(policy == GLCLOCK)
			available_frame = evict_frame_using_gclock();
	}
	if(available_frame!= NULL)
	{
		if(policy == FIFO){
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
		else
		{
			initialize_frame(available_frame);
			available_frame->type = type;
			available_frame->pid = currpid;
		}
	}
	//LOG("Made it here too ");
	restore(mask);
	return available_frame;
}


frame_t * evict_frame_using_gclock(void)
{
	return NULL;
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
	//kprintf("C");
	//print_frame(frame);
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
			if(pid == currpid)
				invlpg((void *)a);
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
				kprintf(" Uh  OH");
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
				//print_frame(frame);
				if(BACKSTORE_ID_IS_VALID(frame->backstore) && BACKSTORE_OFFSET_IS_VALID(frame->backstore_offset))
				{
					//LOG("Frame %d was dirty", frame->id);
					open_bs(frame->backstore);
					write_bs(FRAMEID_TO_PHYSICALADDR(frame->id), frame->backstore, frame->backstore_offset);
					close_bs(frame->backstore);
				}
				else
				{
					print_frame(frame);
					kprintf("Fatal error: Cannot locate backstore for vpage %d to swap out page for pid %d ", vp, pid);
					kill(pid);
					initialize_frame(frame);
					restore(mask);
					return SYSERR;
				}
			}
		}



		//LOG("Got here 1");
		//10. If the page being removed belongs to the current process,
		// invalidate the TLB entry for the page vp, using the invlpg instruction (see Intel Manual, volumes II and III for more details on this instruction).
		// 11. In the inverted page table, decrement the reference count of the frame occupied by pt.



		enable_paging();
		initialize_frame(frame);
		// Update page table entries associated with this frame
		// set the frame to be free
	}
	else if(frame->type == VPTBL)
	{
		evict_from_fifo_list(frame);
		initialize_frame(frame);
	}
	else
	{
		kprintf(" UH OHHH");
		restore(mask);
		return SYSERR;
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
		kprintf("| At %d frame %d owned by %d type %d |", count++, current->id, current->pid, current->type);
		current = current->next;
	}
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

