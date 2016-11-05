#include"yalnix.h"
#include"hardware.h"
#include"io.h"

int kernelttyread(UserContext *uctxt){
	int tty_id = uctxt->regs[0];
	void *buf = (void*) uctxt->regs[1];
	int len = uctxt->regs[2];
	if (len < 0)
	{
		return ERROR;
	}

	if (tty_id < 0 || tty_id >= NUM_TERMINALS)
	{
		return ERROR;
	}
	
    
}

int kernelttywrite(UserContext *uctxt){
	int tty_id = uctxt->regs[0];
	void *buf = (void*) uctxt->regs[1];
	int len = uctxt->regs[2];

    return ERROR;
}
