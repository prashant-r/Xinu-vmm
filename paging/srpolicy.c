#include <xinu.h>

int srlpolicy = FIFO;

syscall  srpolicy (int policy)
{
	if(policy == FIFO)
	{
		srlpolicy = FIFO;
		return OK;
	}
	else if(policy == LRU)
	{
		srlpolicy = LRU;
		return OK;
	}
	return SYSERR;
}
