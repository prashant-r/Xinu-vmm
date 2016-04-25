#include <xinu.h>

//struct bs_entry bstab[MAX_BS_ENTRIES];

syscall get_bs(bsd_t bs_id, unsigned int npages)
{

        if(PAGE_SERVER_STATUS == PAGE_SERVER_INACTIVE){
                psinit();
        }

        char buf[RD_BLKSIZ] = {0};
        memset(buf, 0, RD_BLKSIZ);
        
        /*
	int pagedev, ret, try = 0;
	struct ps_header req, resp;
        */
	if (bs_id > MAX_ID || npages == 0 || npages > MAX_PAGES_PER_BS)
		return SYSERR;

        wait(bs_sem);
        if(bstab[bs_id].isopen == TRUE){
                signal(bs_sem);
                return SYSERR;
        }
        bstab[bs_id].isopen = TRUE;
        bstab[bs_id].npages = npages;
        signal(bs_sem);

        return npages;

}
