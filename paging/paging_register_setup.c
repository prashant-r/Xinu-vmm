#include <xinu.h>
/* some example asm code */

unsigned int tmp;

void set_cr0(unsigned int n) {

  /* we cannot move anything directly into
     %cr0. This must be done via %eax. Therefore
     we save %eax by pushing it then pop
     it at the end.
  */
  intmask ps;
  ps=disable();
  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");		/* mov (move) value at tmp into %eax register.
					   "l" signifies int (see docs on gas assembler)	*/
  asm("movl %eax, %cr0");
  asm("popl %eax");

  restore(ps);

}


unsigned int read_cr0(void) {
  intmask ps;
  unsigned int local_tmp;

  ps=disable();

  asm("pushl %eax");
  asm("movl %cr0, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(ps);

  return local_tmp;
}


unsigned int read_cr2(void) {
  intmask ps;
  unsigned int local_tmp;

  ps=disable();

  asm("pushl %eax");
  asm("movl %cr2, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(ps);

  return local_tmp;
}

void set_cr3(unsigned int n) {

  /* we cannot move anything directly into
     %cr3. This must be done via %eax. Therefore
     we save %eax by pushing it then pop
     it at the end.
  */
  intmask ps;
  ps=disable();
  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");		/* mov (move) value at tmp into %eax register.
					   "l" signifies int (see docs on gas assembler)	*/
  asm("movl %eax, %cr3");
  asm("popl %eax");

  restore(ps);

}

unsigned int read_cr3(void) {
  intmask ps;
  unsigned int local_tmp;

  ps=disable();

  asm("pushl %eax");
  asm("movl %cr3, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(ps);

  return local_tmp;
}

void enable_paging(){
	//LOG("Enabling paging");
	asm volatile (
        "movl %%cr4, %%eax   				;"
        "orl  $0x80, %%eax					;"
    	"movl %%eax, %%cr4   				;"
        "movl %%cr0, %%eax   				;"
        "orl  $0x80000001, %%eax 			;"
        "movl %%eax, %%cr0					"
        : /* no outputs */
        : /* no inputs */
        : "eax"
        );
	LOG("Enabled paging");
}

void disable_paging(void) {
	LOG("DISABLED PAGING");
    unsigned int  cr0;
    __asm__ __volatile__ ("movl %%cr0, %0" : "=r"(cr0));
    __asm__ __volatile__ ("movl %0, %%cr0" : : "r"(cr0 & 0x7fffffff));

}

void switch_page_directory(unsigned int address)
{

	asm volatile (
		"movl %0, %%eax						;"
		"andl $0xFFFFFFE7, %%eax			;"
		"movl %%eax, %%cr3					"
			:/* no output */
			: "g" (address)
			: "eax"
	);

	//LOG(" switch page directory to 0x%08x", address);
}


void flush_tlb()
{
   __asm__ __volatile__ ("mov %%cr3, %%eax\n"
                         "mov %%eax, %%cr3\n"
                         :
                         :
                         : "memory", "eax");
   LOG("Flushed TLB cache");
}

