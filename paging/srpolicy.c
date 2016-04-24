#include <xinu.h>

int policy = FIFO;

syscall  srpolicy (int srpolicy)
{
	if(srpolicy == FIFO)
	{
		policy = FIFO;
		return OK;
	}
	else if(srpolicy == AGING)
	{
		policy = AGING;
		return OK;
	}
	return SYSERR;
}
