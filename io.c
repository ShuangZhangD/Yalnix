#include "yalnix.h"
#include "hardware.h"
#include "io.h"
#include "kernel.h"
#include "testutil.h"

//Global Variable
Tty* tty[NUM_TERMINALS];
extern lstnode* currProc;
// int g_isFinised = 1;

int KernelTtyRead(UserContext *uctxt){
    TracePrintf(2,"Enter KernelTtyRead\n");
    int tty_id = uctxt->regs[0], rc;
    int len = uctxt->regs[2];
    void *buf = (void *) uctxt->regs[1];

    rc = InputSanityCheck((int *)buf);
    if (rc){
        TracePrintf(1, "Error! The buffer address:%p in KernelTtyRead is not valid!\n",buf);
        return ERROR;
    }

    if (len < 0) {
        return ERROR;
    }
    if (tty_id < 0 || tty_id >= NUM_TERMINALS) {
        return ERROR;
    }

    bzero(buf, strlen(buf)); //Empty Buf before writing data into it

    //to if buffer is empty then enwaitingqueue
    while (tty[tty_id]->LeftBufLen <= 0) {
        enreaderwaitingqueue(currProc,tty[tty_id]->readerwaiting);
        switchproc();
    }

    TracePrintf(2, "something to read\n");

    int leftlen = len;
    while (leftlen > 0 && tty[tty_id]->LeftBufLen>0){
        lstnode* bufferNode = firstnode(tty[tty_id]->bufferqueue);
        msg_t* bufMsg = (msg_t *) bufferNode->content;

        int copylen = (leftlen > bufMsg->len)? bufMsg->len: leftlen;
        int leftbuflen = bufMsg->len - copylen;

        memcpy(buf + len - leftlen, bufMsg->buf, copylen);
        TracePrintf(3, "Hello buf:%s\n", buf + len - leftlen);

        leftlen -= copylen;
        bufMsg->len -= copylen;
        tty[tty_id]->LeftBufLen -= copylen;

        if (bufMsg->len == 0){
            lstnode *node = debufferqueue(tty[tty_id]->bufferqueue);
            free(node);
            if(isemptylist(tty[tty_id]->bufferqueue)){
                len = len - leftlen;
                break;
            }
        } else if (bufMsg->len < 0){
            TracePrintf(1, "Error! copy wrong size in KernelTtyRead!\n");
            return ERROR;
        } else {
            memcpy(&bufMsg->buf,&bufMsg->buf[copylen],leftbuflen); 
        }
    }
    TracePrintf(2,"Exit KernelTtyRead\n");

    return len;

}

int KernelTtyWrite(UserContext *uctxt){
    TracePrintf(2,"Enter KernelTtyWrite\n");
    int tty_id = uctxt->regs[0];
    void *buf = (void*) uctxt->regs[1];
    int len = uctxt->regs[2];

    int rc = InputSanityCheck((int *)buf);
    if (rc){
        TracePrintf(1, "Error! The buffer address:%p in KernelTtyWrite is not valid!\n", buf);
        return;
    }

    if (len < 0) {
        return ERROR;
    }

    if (tty_id < 0 || tty_id >= NUM_TERMINALS) {
        return ERROR;
    }
    
    pcb_t* currPcb = TurnNodeToPCB(currProc);

    //Initialize shadow nodes
    int leftlen = len;	
    lstnode *shadowNode = nodeinit(currProc->id);
    enwriterwaitingqueue(shadowNode, tty[tty_id]->writerwaiting);

    //Compare the first shadow node with current process to see whether a process could write ( dynamic blocking)
    while(firstnode(tty[tty_id]->writerwaiting)->id != currProc->id){
        switchnext();
    }

    //Write data until all the data have been written
    while(leftlen > 0){
        int write_len = (len > TERMINAL_MAX_LINE) ? TERMINAL_MAX_LINE:len;
        // if (g_isFinised) {
        TtyTransmit(tty_id, buf + len - leftlen, write_len);
        //     g_isFinised = 0;
        // }
        switchnext();
        leftlen -= write_len;
    }

    //Free Shadow node
    if (!isemptylist(tty[tty_id]->writerwaiting)) {
        lstnode* node = dewriterwaitingqueue(tty[tty_id]->writerwaiting);
        free(node);
    } else {
        TracePrintf(1, "Error! There should be at least one node in dewriterwaitingqueue!\n");
        return ERROR;
    }

    TracePrintf(2,"Exit KernelTtyWrite\n");
    return len;
}

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt){
    TracePrintf(2,"Enter TrapTtyReceive\n");

    int tty_id = uctxt->code;

    while (1){
        //Get data from hardware
        int len = TtyReceive(tty_id, tty[tty_id]->receivebuf, TERMINAL_MAX_LINE);

        //If we read all the data, break the loop
        if (len<=0) break;

        //initialize message nodes
        lstnode *msgNode = nodeinit(0);
        if (NULL == msgNode){
            free(tty[tty_id]->bufferqueue);
            tty[tty_id]->bufferqueue = listinit();
            TracePrintf(1, "Error! Init node in TrapTtyReceive failed!\n");
            return;
        }

        msg_t *msg = (msg_t *) MallocCheck(sizeof(msg_t));
        if (NULL == msg){
            TracePrintf(1, "Error! Malloc nsg_t in TrapTtyReceive failed!\n");
            return;   
        }
        msg->len = len;

        //Copy the data into buffer of message node
        memcpy(msg->buf, tty[tty_id]->receivebuf, len);

        //Zero out receivebuf
        bzero(tty[tty_id]->receivebuf, TERMINAL_MAX_LINE);

        //Save message nodes into a queue
        msgNode->content = (void *) msg;
        enbufferqueue(msgNode, tty[tty_id]->bufferqueue);
        tty[tty_id]->LeftBufLen+=len;
    }

    //Pull all waiting queue out and put them into ready queue to read IO input
    while(!isemptylist(tty[tty_id]->readerwaiting)) {
        TracePrintf(3, "receive dereaderwaitingqueue\n");
        lstnode* node = dereaderwaitingqueue(tty[tty_id]->readerwaiting);
        enreadyqueue(node, readyqueue);
    }

    TracePrintf(2,"Exit TrapTtyReceive\n");
}

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt){
    TracePrintf(2,"Finish Transmit\n");
    // g_isFinised = 1;
    return;
}
