/* deallocate_bs.c - deallocate_bs */

#include <xinu.h>

bsd_t	deallocate_bs (
	unsigned int store
	)
{
	intmask	mask;

	/* Sanity check on store ID */
	if(store >= MAX_BS_ENTRIES) {
		return -1;
	}

	/* Ensure only one process accesses bstab */
	mask = disable();

	/* Ensure this store is allocated */
	if(bstab[store].isallocated == FALSE) {
		restore(mask);
		return -1;
	}

	/* Ensure no one is using this store */
	if(bstab[store].usecount > 0) {
		restore(mask);
		return -1;
	}

	/* Release the store */
	release_bs(store);

	bstab[store].isallocated = FALSE;
	bstab[store].usecount = 0;

	restore(mask);
	return store;
}
