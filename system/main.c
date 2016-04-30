/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>
process    main(void)
{
    /* Start the network */
    /* DO NOT REMOVE OR COMMENT THIS CALL */
    netstart();

    /* Initialize the page server */
    /* DO NOT REMOVE OR COMMENT THIS CALL */
    psinit();

    kprintf(" Using FIFO queue starting now! ");
    srpolicy(FIFO);

    all_paging_tests();

    sleepms(20000);
    while(TRUE)
    {

    }
    return OK;
}
