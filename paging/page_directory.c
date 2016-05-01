#include <xinu.h>


pd_t * retrieve_new_page_directory(void)
{		pd_t *new_pagedirectory = NULL;
	    frame_t *new_pagedirectory_frame = NULL;
	    intmask mask;
	    mask = disable();
	    new_pagedirectory_frame = retrieve_new_frame(DIR);
	    if (NULL == new_pagedirectory_frame) {
	        restore(mask);
	        return NULL;
	    }
	    new_pagedirectory = (pd_t *)FRAMEID_TO_PHYSICALADDR(new_pagedirectory_frame->id);
        //LOG("Frame id %d address 0x%08x", new_pagedirectory_frame->id, new_pagedirectory);
        // Initialize all page directories including first four
        int tbl_id;
        for(tbl_id = 0; tbl_id<PAGEDIRECTORY_ENTRIES_SIZE; tbl_id++)
        {
            if( tbl_id < NUM_GLOBAL_PAGE_TABLES){
                new_pagedirectory->pd_pres = 1;
                new_pagedirectory->pd_write = 1;
                new_pagedirectory->pd_base = FRAME0 +1+tbl_id;
                //LOG("For tbl_id %d base is %d", tbl_id,FIRST_PAGE_TABLE_FRAME + tbl_id );
            }
            else if(tbl_id == (DEVICE_LOC))
            {
            	new_pagedirectory->pd_pres = 1;
            	new_pagedirectory->pd_write = 1;
            	new_pagedirectory->pd_base = FRAME0 +1+ NUM_GLOBAL_PAGE_TABLES;
            }
            else
            {
            	new_pagedirectory->pd_pres  = 0;       /* page table present?      */
            	new_pagedirectory->pd_write = 0;       /* page is writable?        */
            	new_pagedirectory->pd_user  = 0;       /* is user level protection? */
            	new_pagedirectory->pd_pwt   = 0;       /* write through caching for pt?*/
            	new_pagedirectory->pd_pcd   = 0;       /* cache disable for this pt?   */
            	new_pagedirectory->pd_acc   = 0;       /* page table was accessed? */
            	new_pagedirectory->pd_mbz   = 0;       /* must be zero         */
            	new_pagedirectory->pd_fmb   = 0;       /* four MB pages?       */
            	new_pagedirectory->pd_global= 0;       /* global (ignored)     */
            	new_pagedirectory->pd_avail = 0;       /* for programmer's use     */
            	new_pagedirectory->pd_base  = 0;       /* location of page table?  */

            }
            new_pagedirectory ++ ;
        }
        restore(mask);
	    return (pd_t*)FRAMEID_TO_PHYSICALADDR(new_pagedirectory_frame->id);
}

int free_page_directory(pid32 pid)
{
    intmask mask;
    mask = disable();
    struct procent * prptr;
    prptr = &proctab[pid]; // Get the process table entry
    pd_t * pagedir = prptr->pagedir;
    //TODO:
    //for (i=0; i < PAGEDIRECTORY_ENTRIES_SIZE; i++)
        //free_page_table(PA2FP(pt_t *)&pagedir[i]);
    int frame_id = PA_TO_FRAMEID((unsigned int)pagedir);
    free_frame(&frames[frame_id]);
    restore(mask);
    return OK;
}


void print_directory(pid32 pid)
{
	intmask mask = disable();
	struct procent * prptr = &proctab[pid];

int tbl_id;
	for(tbl_id = 0; tbl_id<PAGEDIRECTORY_ENTRIES_SIZE; tbl_id++)
	 {
		if(prptr->pagedir[tbl_id].pd_pres)
		{
			//kprintf(" \n For %d base is 0x%08x", tbl_id, prptr->pagedir[tbl_id].pd_base);
		}
	 }
	restore(mask);
}
