/* close_bs.c - close_bs */

#include <xinu.h>

bsd_t	close_bs (
	unsigned int store
	)
{
	/* Sanity check on store ID */
	if(store >= MAX_BS_ENTRIES) {
		return -1;
	}

	/* Ensure only one process accesses bstab */
	wait(bs_sem);

	/* Ensure the store is allocated */
	if(bstab[store].isallocated == FALSE) {
		signal(bs_sem);
		return -1;
	}

	/* Decrease the usecount of the store */
	bstab[store].usecount--;

	signal(bs_sem);
	return store;
}
