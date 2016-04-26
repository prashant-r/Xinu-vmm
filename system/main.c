/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

process lab5test(int v) {
    kprintf("From lab5test %d: cr3=%x  %d \n ", currpid, read_cr3());
    int* x = (int*) vgetmem(sizeof(int));
    *x = v;
    while(*x > 0) {
        //kprintf("%d's &x = %x x = %d cr3=%x\n", currpid, x, *x, cr3_read());
        kprintf("%d's x = %d\n", currpid, *x);
        sleepms(2 * (*x));
        *x -= 10;
    }
    while(TRUE)
    {

    }
    return OK;
}

process    main(void)
{
    /* Start the network */
    /* DO NOT REMOVE OR COMMENT THIS CALL */
    netstart();

    /* Initialize the page server */
    /* DO NOT REMOVE OR COMMENT THIS CALL */
    psinit();

    kprintf("From main %d: cr3=%x\n", currpid, read_cr3());

    all_paging_tests();
//
//    pid32 v = vcreate(lab5test, INITSTK, 200, INITPRIO, "lab5test", 1,
//605, NULL);
//    if(v == SYSERR) {
//        kprintf("vcreate error \n");
//    }
//    resume(v);
//
//    // sleepms(10000);
//
//    v = vcreate(lab5test, INITSTK, 200, INITPRIO, "lab5test", 1, 507, NULL);
//    if(v == SYSERR) {
//        kprintf("vcreate error \n");
//    }
//    resume(v);
//
//    // sleepms(10000);
//
////    v = vcreate(lab5test, INITSTK, 200, INITPRIO, "lab5test", 1, 404, NULL);
////    if(v == SYSERR) {
////        kprintf("vcreate error \n");
////    }
////    resume(v);
//
////    v = vcreate(lab5test, INITSTK, 200, INITPRIO, "lab5test", 1, 508, NULL);
////    if(v == SYSERR) {
////        kprintf("vcreate error \n");
////    }
////    resume(v);
//
//    v = vcreate(lab5test, INITSTK, 200, INITPRIO, "lab5test", 1, 609, NULL);
//    if(v == SYSERR) {
//        kprintf("vcreate error \n");
//    }
//    resume(v);

    // kprintf("\n...creating a shell\n");
    // recvclr();
    // resume(create(shell, 8192, 50, "shell", 1, CONSOLE));
    return OK;
}
