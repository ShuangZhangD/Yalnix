#include"yalnix.h"
#include"hardware.h"
#include"io.h"
#include"kernel.h"
#include "testutil.h"


extern lstnode* currProc;
int transmitbusy = 0;
int transleftbuflen;
int kernelttyread(UserContext *uctxt){
	TracePrintf(1,"Enter kernelttyread\n");
	int tty_id = uctxt->regs[0];
	void *buf = (void *) uctxt->regs[1];
	int len = uctxt->regs[2];
	if (len < 0)
	{
		return ERROR;
	}

	if (tty_id < 0 || tty_id >= NUM_TERMINALS)
	{
		return ERROR;
	}
	//to if buffer is empty then enwaitingqueue
	while (receivelen <= 0)
	{
		enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
		switchproc();
	}
	
	// enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
	TracePrintf(1, "something to read\n");
	// if (!(firstnode(tty[tty_id]->readerwaiting) == currProc))
	// {
	// 	TracePrintf(1, "firstnode enreaderwaitingqueue");
	// 	enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
	// 	switchproc();
	// }
	// else{

		if (len <= receivelen)
		{
			
			memcpy(buf, tty[tty_id]->receivebuf, len);
			int leftbuflen = receivelen - len;
			int leftbuf[leftbuflen];
			memcpy(leftbuf, &tty[tty_id]->receivebuf[len], leftbuflen);
			memcpy(tty[tty_id]->receivebuf, leftbuf, leftbuflen);
			receivelen = leftbuflen;
			// lstnode* node = dereaderwaitingqueue(tty[tty_id]->readerwaiting);
			// enreadyqueue(currProc, readyqueue);

			return len;
		}
		else{
			TracePrintf(1, "Enter memcpy\n");
			TracePrintf(1, "PID = %d, buf: %p, ttybuffer: %s", currProc->id, buf, tty[tty_id]->receivebuf);
			memcpy(buf, (void*)tty[tty_id]->receivebuf, receivelen);
			TracePrintf(1, "Exit memcpy\n");

			int len = receivelen;
			receivelen = 0;

			// lstnode* node = dereaderwaitingqueue(tty[tty_id]->readerwaiting);
			// if (tty[tty_id]->readerwaiting->size > 1)
			// {
			// enreadyqueue(currProc, readyqueue);	
			// }
			

			return len;
		}

	// }
}

int kernelttywrite(UserContext *uctxt){
	TracePrintf(1,"Enter kernelttywrite\n");
	int tty_id = uctxt->regs[0];
	void *buf = (void*) uctxt->regs[1];
	int len = uctxt->regs[2];
	if (len < 0) {
		return ERROR;
	}

	if (tty_id < 0 || tty_id >= NUM_TERMINALS) {
		return ERROR;
	}

	enwriterwaitingqueue(currProc, tty[tty_id]->writerwaiting);
	if (len < TERMINAL_MAX_LINE) {	
		TtyTransmit(tty_id, buf, len);
		switchnext();
	} else{

		transleftbuflen = len;
		// tty[tty_id]->transmitbuf = (char *)malloc(sizeof(char) * leftbuflen);
		while(transleftbuflen > 0)
		{			
			// memcpy((void *)tty[tty_id]->transmitbuf+len-leftbuflen, buf+len-leftbuflen, TERMINAL_MAX_LINE);
			// if (!(firstnode(tty[tty_id]->writerwaiting) == currProc))
			// {
			// 	insert_head(currProc, tty[tty_id]->writerwaiting);
			// }

			if(transmitbusy == 0)
			{
				TracePrintf(1, "TRANSMIT\n");
				TtyTransmit(tty_id, buf+len-transleftbuflen, TERMINAL_MAX_LINE);
				transmitbusy = 1;
				transleftbuflen = transleftbuflen - TERMINAL_MAX_LINE;
			}
			switchproc(); 
			TracePrintf(1, "While loop\n");			

		}
		transleftbuflen = 0;
		// TtyTransmit(tty_id, tty[tty_id]->transmitbuf, TERMINAL_MAX_LINE);

		TracePrintf(1, "AND TRANSMIT\n");

		// lstnode *node = dewriterwaitingqueue(tty[tty_id]->writerwaiting);
		// enreadyqueue(node, readyqueue);

	}
	TracePrintf(1,"Exit kernelttywrite\n");
	return len;
}

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt){
    TracePrintf(1,"Enter TrapTtyReceive\n");
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
	int tty_id = uctxt->code;
	receivelen = TtyReceive(tty_id, tty[tty_id]->receivebuf, TERMINAL_MAX_LINE);
	TracePrintf(1, "receivelen=%d\n", receivelen);
	while (receivelen > 0 && (!isemptylist(tty[tty_id]->readerwaiting)))
	{
		
		
			TracePrintf(1, "receive dereaderwaitingqueue\n");
			lstnode* node = dereaderwaitingqueue(tty[tty_id]->readerwaiting);
			int rc;
			insert_head(node, readyqueue);
			switchproc();

			// rc = KernelContextSwitch(MyTrueKCS, (void *) currProc, (void *) node);
            // if (rc) TracePrintf(1,"MyTrueKCS in switchproc failed!\n");
			// enreadyqueue(node, readyqueue);
			
		

	}
	switchproc();
    TracePrintf(1,"Exit TrapTtyReceive\n");
}

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt){
    TracePrintf(1,"Enter TrapTtyTransmit\n");
	int tty_id = uctxt->code;
	transmitbusy = 0;
	
	// if(transleftbuflen > 0)
	// {
	// 	insert_head(currProc, tty[tty_id]->writerwaiting);
	// 	switchproc();
	// 	TracePrintf(1, "enter transleftbuflen");
	// }
	if (transleftbuflen > 0)
	{
		lstnode* node = firstnode(tty[tty_id]->writerwaiting);
		int rc;
		rc = KernelContextSwitch(MyIOKCS, (void *)currProc, (void *)node);        
		if (rc) TracePrintf(1,"MyTrueKCS in switchproc failed!\n");
	}


	if (!isemptylist(tty[tty_id]->writerwaiting) && transleftbuflen <= 0)
	{
		lstnode* node = dewriterwaitingqueue(tty[tty_id]->writerwaiting);
		enreadyqueue(node, readyqueue);
	}
	// 

	// traverselist(tty[tty_id]->writerwaiting);
		TracePrintf(1, "switchproc success");

	switchproc();

}
