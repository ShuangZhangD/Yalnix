#include"yalnix.h"
#include"hardware.h"
#include"io.h"
#include"kernel.h"

extern lstnode* currProc;

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
	//to if buffer is empty then enwaitingqueue
	if (receivelen <= 0)
	{
		enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
		switchnext();
	}
	
	enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);

	if (!(firstnode(tty[tty_id]->readerwaiting) == currProc))
	{
		enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
		switchnext();
	}
	else{

		if (len <= receivelen)
		{
			memcpy(buf, tty[tty_id]->receivebuf, len);
			int leftbuflen = receivelen - len;
			int leftbuf[leftbuflen];
			memcpy(leftbuf, &tty[tty_id]->receivebuf[len], leftbuflen);
			memcpy(tty[tty_id]->receivebuf, leftbuf, leftbuflen);
			receivelen = 0;
			lstnode* node = dereaderwaitingqueue(tty[tty_id]->readerwaiting);
			enreadyqueue(node, readyqueue);

			return len;
		}
		else{
			memcpy(buf, tty[tty_id]->receivebuf, receivelen);
			int len = receivelen;
			receivelen = 0;
			lstnode* node = dereaderwaitingqueue(tty[tty_id]->readerwaiting);
			enreadyqueue(node, readyqueue);

			return len;
		}

	}



}

int kernelttywrite(UserContext *uctxt){
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

	enwriterwaitingqueue(currProc, tty[tty_id]->writerwaiting);
	if (!(firstnode(tty[tty_id]->readerwaiting) == currProc))
	{
		enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
		switchnext();
	}	

	if (len < TERMINAL_MAX_LINE)
	{
		TtyTransmit(tty_id, buf, len);
		lstnode *node = dewriterwaitingqueue(tty[tty_id]->writerwaiting);
		enreadyqueue(node, readyqueue);

	}
	else{
		int leftbuflen = len;
		while(leftbuflen > 0)
		{			
			TtyTransmit(tty_id, buf+len-leftbuflen, TERMINAL_MAX_LINE);
			leftbuflen = leftbuflen - TERMINAL_MAX_LINE;
		}
		lstnode *node = dewriterwaitingqueue(tty[tty_id]->writerwaiting);
		enreadyqueue(node, readyqueue);

	}

	return len;
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
	int tty_id = uctxt->code;
	receivelen = TtyReceive(tty_id, tty[tty_id]->receivebuf, TERMINAL_MAX_LINE);

	if(receivelen >0)
	{
		if(!isemptylist(tty[tty_id]->readerwaiting))
		{
			lstnode* node = dereaderwaitingqueue(tty[tty_id]->readerwaiting);
			enreadyqueue(node, readyqueue);
			
		}

	}
	switchnext();
}

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt){
    /*
       tty_id = uctxt->code;

    //Complete blocked process 
    kernelttywrite(int tty_id, void *buf, int len);

     */
	int tty_id = uctxt->code;
	if(!isemptylist(tty[tty_id]->writerwaiting))
	{
		lstnode* node = dewriterwaitingqueue(tty[tty_id]->writerwaiting);
		enreadyqueue(node, readyqueue);
	}
	switchnext();


}
