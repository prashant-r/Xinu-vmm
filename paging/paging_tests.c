#include <xinu.h>
void all_paging_tests()
{
	kprintf("\n Running MYPOLICY tests");

	kprintf("\n FIFO queue policy: ");
	pagefaults = 0;
	srpolicy(FIFO);
	agingTest();
	sleepms(10000);

	kprintf("\n Resetting number of faults");

	pagefaults = 0;
	kprintf("\n AGING queue policy: ");
	srpolicy(AGING);
	agingTest();
	sleepms(6000);

	kprintf("\n Running max page test");
	maxPageTest();
	while(TRUE)
	{
		recvclr();
		sleepms(1500);
	}

}


void agingTest()
{
	int pa, pb, pc, pd, pe, pf;
	pa = vcreate(rpprocA, 1024, 200, 20, "rprocA", 0, 0);
	resume(pa);
	return;
}

void maxPageTest()
{
	int pa, pb, pc, pd, pe, pf;
    pa = vcreate(vcprocA, 1024, 200, 20, "procA", 0, 0);
    pb = vcreate(vcprocA, 1024, 200, 20, "procA", 0, 0);
    pc = vcreate(vcprocA, 1024, 200, 20, "procA", 0, 0);
    pd = vcreate(vcprocA, 1024, 200, 20, "procA", 0, 0);
    pe = vcreate(vcprocA, 1024, 200, 20, "procA", 0, 0);
    pf = vcreate(vcprocA, 1024, 200, 20, "procA", 0, 0);


    resume(pa);
    resume(pb);
    resume(pc);
}

void vcprocA()
{

	//print_directory(currpid);
	char c = 'B';
	//addressTranslate(tmp);
	//printMemory();
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	struct procent * prptr = &proctab[currpid];
	int count = (200 * PAGE_SIZE)-8;
	//LOG(" Size before before is %d" ,  prptr->vmemlist.mlist.mlength)  ;
	char * tmp = vgetmem(count);
	//kprintf(" Address provided is 0x%08x", tmp);
	//printMemory();
	int a;
	//kprintf(" Made it here");
	for(a =0; a < count; a++)
	{
		tmp[a] = c;
	}
//	LOG(" made it here");
	//LOG("Number of free frames are  %d ", get_free_frame_count());
	int testcount = 0;
	for (a = 0; a< count; a++)
		if(tmp[a] == c)
			testcount ++ ;
//	LOG(" trying");
	if(testcount == count)
		kprintf("\n proc%c has correct values in memset \n", c);
	else
		kprintf("\n proc%c has incorrect values in memset %d!= %d \n",c, testcount, count);
	while(TRUE)
	{

	}
}


void rpprocA()
{
	char c = 'G';
	unsigned long count = (200 * NBPG)-8;
	char *tmp = vgetmem(count);
	int a;
	srand(20000*currpid);
	int g;
	for( g = 0; g < 200000; g++){
		a = rand()%count;
		tmp[a] = ' Y';
	}
	kprintf(" Number of faults is : %d" ,  get_faults());
}
