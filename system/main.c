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

    recvclr();

    int bs_id;
    for( bs_id =0; bs_id < MAX_BS_ENTRIES; bs_id ++)
    	open_bs(bs_id);
    srpolicy(FIFO);

    sleepms(2500);
    recvclr();

    all_paging_tests();

    while(TRUE)
    {
    	recvclr();
    	sleepms(3000);
    }
    return OK;
}
