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

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt){
    /*
    //Get the input string using TtyReceive

    char[] = str;
    tty_id = uctxt->code; 
    whlile (TtyReceive(int tty_id, void *buf, int len) != 0{
    str+=buf;
    }

    //Buffer for kernelttyread
    kernelttyread(str);
     */
}

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt){
    /*
       tty_id = uctxt->code;

    //Complete blocked process 
    kernelttywrite(int tty_id, void *buf, int len);

     */
}