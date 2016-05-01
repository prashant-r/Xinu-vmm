#include <xinu.h>

int initialize_global_pagetables(void)
{
	int global_page_table_index;
	int pagetable_entry;
    pt_t *new_pagetable = NULL;
    intmask mask;
    mask = disable();
    for (global_page_table_index = 0; global_page_table_index < NUM_GLOBAL_PAGE_TABLES; ++global_page_table_index) {
        frame_t *frame = NULL;
        frame = retrieve_new_frame(GPTBL);
        if(frame == NULL)
        {
            LOG("Error: Failed to create new global page tables no free frames");
            restore(mask);
            return SYSERR;
        }
        new_pagetable = (pt_t *) FRAMEID_TO_PHYSICALADDR(frame->id);
        for (pagetable_entry = 0; pagetable_entry < PAGETABLE_ENTRIES_SIZE; ++pagetable_entry) {
            bzero((pt_t *)new_pagetable, sizeof(pt_t));
        	new_pagetable->pt_pres = 1;
        	new_pagetable->pt_write = 1;
        	new_pagetable->pt_base = ((global_page_table_index * PAGETABLE_ENTRIES_SIZE) +pagetable_entry);
            new_pagetable->pt_global = 1;
        	new_pagetable ++;
        }
    }


    LOG("Initialized global pagetables.");
    restore(mask);
    return OK;
}

int initialize_device_pagetable(void)
{
	int pagetable_entry;
	intmask mask;
	mask = disable();
	pt_t *new_pagetable = NULL;
	frame_t * frame = NULL;
	frame = retrieve_new_frame(GPTBL);
	new_pagetable = (pt_t *) FRAMEID_TO_PHYSICALADDR(frame->id);
    LOG("Frame id %d address 0x%08x", frame->id, new_pagetable);
	for (pagetable_entry = 0; pagetable_entry < PAGETABLE_ENTRIES_SIZE; ++pagetable_entry) {
            bzero((char *)new_pagetable, sizeof(pt_t));
            new_pagetable->pt_pres = 1;
		    new_pagetable->pt_write = 1;
            new_pagetable->pt_base = (((DEVICE_LOC) * PAGETABLE_ENTRIES_SIZE)+pagetable_entry);
            new_pagetable->pt_global = 1;
            new_pagetable++;
	}
	LOG("Initialized device pagetable.");
	restore(mask);
	return OK;
}

pt_t * retrieve_new_page_table(void)
{
    int pagetable_entry;
    pt_t * new_pagetable;
    frame_t * frame;
    intmask mask = disable();
    frame = retrieve_new_frame(VPTBL);
    hook_ptable_create(frame->id + FRAME0);
    if(frame == NULL)
    {
        LOG("Error: Failed to create new page tables no free frames available");
        restore(mask);
        return NULL;
    }
    new_pagetable = (pt_t *) FRAMEID_TO_PHYSICALADDR(frame->id);
    for (pagetable_entry = 0; pagetable_entry < PAGETABLE_ENTRIES_SIZE; ++pagetable_entry) {
    	 bzero((char *)new_pagetable, sizeof(pt_t));
    	 new_pagetable++;
    }
    restore(mask);
    return (pt_t *) FRAMEID_TO_PHYSICALADDR(frame->id);
}

