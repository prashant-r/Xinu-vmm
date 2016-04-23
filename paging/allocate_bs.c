/* allocate_bc.c - allocate_bs */

#include <xinu.h>

bsd_t	allocate_bs (
	unsigned int npages
	)
{
	int32	i;
	intmask	mask;

	/* Ensure only one process accesses bstab */
	mask = disable();

        if(PAGE_SERVER_STATUS == PAGE_SERVER_INACTIVE){
                psinit();
        }

	/* Find an unallocated store */
	for(i = 0; i < MAX_BS_ENTRIES; i++) {
		if(bstab[i].isallocated == FALSE) { /* Found an unallocated store */
			if(get_bs(i, npages) == -1) { /* Try to open it */
				continue;
			}
			/* Successfully opened, allocate it */
			bstab[i].isallocated = TRUE;
			bstab[i].usecount = 0;

			restore(mask);
			return i;
		}
	}

	restore(mask);
	return -1;
}
