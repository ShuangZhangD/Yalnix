#include"yalnix.h"
#include"hardware.h"
#include"io.h"
#include"kernel.h"

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
		enreaderwaitingqueue(currProc,readerwaiting);
		switchnext();
	}
	
	enreaderwaitingqueue(currProc,readerwaiting);

	if (!(firstnode(readerwaiting) == currProc))
	{
		enreaderwaitingqueue(currProc,readerwaiting);
		switchnext();
	}
	else{

		if (len <= receivelen)
		{
			memcpy(buf, tty[i]->receivebuf, len);
			int leftbuflen = receivelen - len;
			int leftbuf[leftbuflen];
			memcpy(leftbuf, &tty[i]->receivebuf[len], leftbuflen);
			memcpy(tty[i]->receivebuf, leftbuf, leftbuflen);
			receivelen = 0;
			dereaderwaitingqueue(readerwaiting);
			enreadyqueue(node, readyqueue);

			return len;
		}
		else{
			memcpy(buf, tty[i]->receivebuf, receivelen);
			int len = receivelen;
			receivelen = 0;
			dereaderwaitingqueue(readerwaiting);
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

	enwriterwaitingqueue(currProc, writerwaiting);
	if (!(firstnode(readerwaiting) == currProc))
	{
		enreaderwaitingqueue(currProc,readerwaiting);
		switchnext();
	}	

	if (len < TERMINAL_MAX_LINE)
	{
		TtyTransmit(tty_id, buf, len);
		dewriterwaitingqueue(writerwaiting);
		enreadyqueue(node, readyqueue);

	}
	else{
		int leftbuflen = len;
		while(lefybuflen > 0)
		{			
			TtyTransmit(tty_id, buf+len-leftbuflen, TERMINAL_MAX_LINE);
			leftbuflen = leftbuflen - TERMINAL_MAX_LINE;
		}
		dewriterwaitingqueue(writerwaiting);
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
	receivelen = TtyReceive(tty_id, receivebuf, TERMINAL_MAX_LINE);

	if(receivelen >0)
	{
		if(!isemptylist(readerwaiting))
		{
			lstnode* node = dereaderwaitingqueue(readerwaiting);
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
	if(!isemptylist(writerwaiting))
	{
		lstnode* node = dewriterwaitingqueue(writerwaiting);
		enreadyqueue(node, readyqueue);
	}
	switchnext();


}
