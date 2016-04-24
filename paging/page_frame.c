#include <xinu.h>

frame_t frames[NFRAMES];

void initialize_all_frames(void)
{
	int frame = 0;
	frame_t *frameptr = NULL;
	intmask mask;
	mask = disable();
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
    restore(mask);
}

frame_t * retrieve_new_frame(frame_type type)
{
	int frame = 0;
	frame_t *frameptr = NULL;
	frame_t *available_frame = NULL;
	intmask mask;
	mask = disable();
    
	if(type == PPTBLCAT2)
	{
		frameptr = &frames[DEVICE_FRAME];
        frameptr->type = type;
        frameptr->pid = currpid;
        frameptr->vp_no = 0;
        frameptr->dirty = 0;
        frameptr->backstore = -1;
        frameptr->backstore_offset=0;
		restore(mask);
		return frameptr;
	}
    else{
        int FRAME_COUNT_LOWER_BOUND = (type == PPTBLCAT1) ? 1 : 0;
        int FRAM_COUNT_UPPER_BOUND = (type == PPTBLCAT1) ? NUM_GLOBAL_PAGE_TABLES : NFRAMES-1;
        for (frame = FRAME_COUNT_LOWER_BOUND; frame <= FRAM_COUNT_UPPER_BOUND; ++frame) {
            frameptr = &frames[frame];
            if(frameptr->type == FREE){
                    available_frame = frameptr;
                    available_frame->type = type;
                    frameptr->pid = currpid;
                    frameptr->vp_no = 0;
                    frameptr->dirty = 0;
                    frameptr->backstore = -1;
                    frameptr->backstore_offset = 0;
                    break;
                }
            }
        }
	restore(mask);
	return available_frame;
}


int free_frame(int frame_id)
{
    intmask mask;
    mask = disable();
    if(!FRAMEID_IS_VALID(frame_id))
    {
        restore(mask);
        return SYSERR;
    }
    frame_t * frameptr = &frames[frame_id];
    if(frameptr->type == FREE)
    {
        restore(mask);
        return OK;
    }
    // Update page table entries associated with this frame


    // set the frame to be free
    initialize_frame(frameptr);
    frameptr->type = FREE;
    return OK;

}

syscall frame_map_check(int pid, int store, int page_offset_in_store, int * pageframe_id )
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
			  kprintf(" Was a  match %d, %d, %d, %d, %d, %d", frames[fr_id].pid, currpid, frames[fr_id].backstore, store, frames[fr_id].backstore_offset, page_offset_in_store);
			  *pageframe_id = fr_id;
	      	  restore(mask);
	      	  return OK;
		  }

	    }
	}
	restore(mask);
	return EMPTY;


}



