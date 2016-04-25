#include <xinu.h>

struct bs_entry bstab[MAX_BS_ENTRIES];
sid32   bs_sem;

int psinit ()
{
        int i=0;
        /* To make sure there is atmost only one psinit() opening the disk */
        wait(bs_init_sem);

        if(PAGE_SERVER_STATUS == PAGE_SERVER_ACTIVE){
                return OK;
        }

        kprintf("Trying to open the RDISK\r\n");
        if(open(RDISK, "backing_store", "rw") == SYSERR){
                panic("Could not initialize the page server, opening the remote disk failed\r\n");
        }

        //kprintf("Trying to create the semaphore\r\n");
        if((bs_sem = semcreate(1)) == SYSERR){
                panic("Could not initialize the page server, creating semaphore failed\r\n");
        }

        for(i=0; i < MAX_BS_ENTRIES; i++){
                bstab[i].isopen = FALSE;
                bstab[i].isallocated = FALSE;
                bstab[i].usecount = 0;
                bstab[i].npages = 0;
        }
        
        PAGE_SERVER_STATUS = PAGE_SERVER_ACTIVE;

        signal(bs_init_sem);

	return OK;
}
