#include <xinu.h>

void all_paging_tests()
{
	kprintf(" Running aging tests");
	//agingTest();
	//sleepms(20000);
	kprintf(" Running max page test");
	maxPageTest();

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
    pd = vcreate(vcprocA, 1024, 200, 20, "procD", 0, 0);
    pf = vcreate(vcprocA, 1024, 200, 20, "procF", 0, 0);

    LOG(" Number of free frames is %d ", get_free_frame_count());

    resume(pa);
    resume(pd);
    resume(pf);
    return;
}

void vcprocA()
{

	//print_directory(currpid);


	LOG(" In vcproc%d", currpid);

	char c = 'B';
	//addressTranslate(tmp);
	//printMemory();
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	int count = (200 * PAGE_SIZE)-8;
	char * tmp = vgetmem(count);
	//kprintf(" Address provided is 0x%08x", tmp);
	//printMemory();
	int a;
	//kprintf(" Made it here");
	for(a =0; a < count; a++)
	{
		tmp[a] = c;
	}
	//LOG("Number of free frames are  %d ", get_free_frame_count());
	int testcount = 0;
	for (a = 0; a< count; a++)
		if(tmp[a] == c)
			testcount ++ ;

	if(testcount == count)
		kprintf("\n proc%c has correct values in memset \n", c);
	else
		kprintf("\n proc%c has incorrect values in memset %d!= %d \n",c, testcount, count);

	vfreemem(tmp, count);

	testcount = 0;
	vfreemem(tmp, count);
	for (a = 0; a< count; a++)
			if(tmp[a] == c)
				testcount ++ ;
	if(testcount == 0)
			kprintf("\n proc%c has correct values in memfree \n",c);
		else
			kprintf("\n proc%c has incorrect values in memfree %d!= %d \n",c, testcount, count);
}


void rpprocA()
{
	char c = 'G';
	int count = (150 * PAGE_SIZE)-8;
	char *tmp = vgetmem(count);
	int a;
	for(a =0; a < count; a++)
		{
			tmp[a] = c;
		}

	srand(count);
	int g;
	for( g = 0; g < 9000; g++){
		tmp[rand()] = ' Y';
	}
	kprintf(" Number of faults is :" +  get_faults());
}
