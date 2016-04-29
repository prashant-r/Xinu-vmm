#include <xinu.h>

void all_paging_tests()
{

	maxPageTest();


}
void maxPageTest()
{

	int pa, pb, pc, pd, pe, pf;
    pa = vcreate(vcprocA, 1024, 200, 20, "procA", 1, 'A');
    pd = vcreate(vcprocA, 1024, 200, 20, "procD", 1, 'D');
    pf = vcreate(vcprocA, 1024, 200, 20, "procF", 1, 'F');
    resume(pa);
    sleepms(15);
    resume(pd);
    sleepms(15);
    resume(pf);
    return;
}


void vcprocA(char c)
{
	char *tmp = (char *)0x01000001;
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	//printMemory();
	*tmp = 'A';
	//addressTranslate(tmp);
	//printMemory();
	//kprintf("Character at address %d is %c \n",tmp, *tmp);
	int count = (200 * PAGE_SIZE)-8;
	tmp = vgetmem(count);
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

	vfreemem(tmp,count-8);
	testcount = 0;
	for (a = 0; a< count; a++)
			if(tmp[a] == c)
				testcount ++ ;

	if(testcount == 0)
			kprintf("\n proc%c has correct values in memfree \n",c);
		else
			kprintf("\n proc%c has incorrect values in memfree %d!= %d \n",c, testcount, count);


}
