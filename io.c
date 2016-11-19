#include "yalnix.h"
#include "hardware.h"
#include "io.h"
#include "kernel.h"
#include "testutil.h"


extern lstnode* currProc;
int g_isFinised = 1;

int kernelttyread(UserContext *uctxt){
	TracePrintf(2,"Enter kernelttyread\n");
	int tty_id = uctxt->regs[0], rc;
	int len = uctxt->regs[2];
	void *buf = (void *) uctxt->regs[1];
	
	rc = InputSanityCheck((int *)buf);
	if (rc){
		TracePrintf(1, "Error! The buffer address:%p in kernelttyread is not valid!\n",buf);
		return;
	}

	if (len < 0) {
		return ERROR;
	}
	if (tty_id < 0 || tty_id >= NUM_TERMINALS) {
		return ERROR;
	}


	//to if buffer is empty then enwaitingqueue
	while (receivelen <= 0)
	{
		enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
		switchproc();
	}
	
	// enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
	TracePrintf(2, "something to read\n");
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
			TracePrintf(3, "Enter memcpy\n");
			TracePrintf(3, "PID = %d, buf: %p, ttybuffer: %s", currProc->id, buf, tty[tty_id]->receivebuf);
			memcpy(buf, (void*)tty[tty_id]->receivebuf, receivelen);
			TracePrintf(3, "Exit memcpy\n");

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
	TracePrintf(2,"Enter kernelttywrite\n");
	int tty_id = uctxt->regs[0];
	void *buf = (void*) uctxt->regs[1];
	int len = uctxt->regs[2];

	int rc = IOSanityCheck((int *)buf);
	if (rc){
		TracePrintf(1, "Error! The buffer address:%p in kernelttywrite is not valid!\n", buf);
		return;
	}

	if (len < 0) {
		return ERROR;
	}

	if (tty_id < 0 || tty_id >= NUM_TERMINALS) {
		return ERROR;
	}
	pcb_t* currPcb = TurnNodeToPCB(currProc);

	int leftlen = len;	
	lstnode *shadowNode = nodeinit(currProc->id);
	enwriterwaitingqueue(shadowNode, tty[tty_id]->writerwaiting);
	
	while(firstnode(tty[tty_id]->writerwaiting)->id != currProc->id){
		switchnext();
	}

	while(leftlen > 0){
		int write_len = (len > TERMINAL_MAX_LINE) ? TERMINAL_MAX_LINE:len;
		if (g_isFinised) {
			TtyTransmit(tty_id, buf + len - leftlen, write_len);
			g_isFinised = 0;
		}
		switchnext();
		leftlen -= write_len;
	}

	if (!isemptylist(tty[tty_id]->writerwaiting)) {
		lstnode* node = dewriterwaitingqueue(tty[tty_id]->writerwaiting);
		free(node);
	} else {
		TracePrintf(1, "Error! There should be at least one node in dewriterwaitingqueue!\n");
		return ERROR;
	}

	TracePrintf(2,"Exit kernelttywrite\n");
	return len;
}

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt){
    TracePrintf(2,"Enter TrapTtyReceive\n");
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
	TracePrintf(3, "receivelen=%d\n", receivelen);
	while (receivelen > 0 && (!isemptylist(tty[tty_id]->readerwaiting)))
	{
		
		
			TracePrintf(3, "receive dereaderwaitingqueue\n");
			lstnode* node = dereaderwaitingqueue(tty[tty_id]->readerwaiting);
			int rc;
			insert_head(node, readyqueue);
			switchproc();

			// rc = KernelContextSwitch(MyTrueKCS, (void *) currProc, (void *) node);
            // if (rc) TracePrintf(1,"MyTrueKCS in switchproc failed!\n");
			// enreadyqueue(node, readyqueue);
			
		

	}
	switchproc();
    TracePrintf(2,"Exit TrapTtyReceive\n");
}

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt){
    TracePrintf(2,"Finish Transmit\n");
    g_isFinised = 1;
	return;
}
