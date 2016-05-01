#include <xinu.h>

void hook_ptable_create(unsigned int pagenum)
{
	kprintf("\n=== Created page table %d\n", pagenum);
}

void hook_ptable_delete(unsigned int pagenum)
{
	kprintf("\n=== Deleted page table %d\n", pagenum);
}


void hook_pfault(char *addr)
{
	kprintf("\n=== Page fault for address 0x%08x\n", addr);
}


void hook_pswap_out(unsigned int pagenum, int framenb)
{
	kprintf("\n=== Replacing frame number %d, virtual page %d", framenb, pagenum);
}

