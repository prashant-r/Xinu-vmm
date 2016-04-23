/* open_bs.c - open_bs */

#include <xinu.h>

bsd_t	open_bs (
	bsd_t	store
	)
{
	/* Sanity check on store ID */
	if(store >= MAX_BS_ENTRIES) {
		return -1;
	}

	/* Ensure only one process accesses the bstab */
	wait(bs_sem);

	/* Ensure the store is allocated */
	if(bstab[store].isallocated == FALSE) {
		signal(bs_sem);
		return -1;
	}

	/* Increase the usecount of the store */
	bstab[store].usecount++;

	signal(bs_sem);
	return store;
}
