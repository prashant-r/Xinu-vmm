/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

process	main(void)
{

	/* Start the network */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	netstart();

	/* Initialize the page server */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	psinit();
	
	// Run tests
	all_paging_tests();
	/* Wait for shell to exit and recreate it */
	sleepms(200);
	kprintf("\n\nMain process recreating shell\n\n");
	resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	while (TRUE) {
		receive();
	}
	return OK;
}
