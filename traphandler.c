#include <stdio.h>
#include "hardware.h"
#include "yalnix.h"
#include "kernel.h"
#include "lock.h"
#include "cvar.h"
#include "pipe.h"
#include "io.h"
#include "processmanage.h"
#include "traphandler.h"
#include "kernel.h"

extern lstnode* currProc;
extern dblist* freeframe_list;

//capture TRAP_CLOCK
void TrapKernel(UserContext *uctxt){
    TracePrintf(1, "TrapKernel called\n");
    int rc;
    switch(uctxt->code){
        case YALNIX_FORK:
            rc = kernelfork(uctxt);
            break;

        case YALNIX_EXEC: 
            rc = kernelexec(uctxt);
            break;

        case YALNIX_EXIT:
            rc = kernelexit(uctxt);
            break;

        case YALNIX_WAIT:
            rc = kernelwait(uctxt);
            break;

        case YALNIX_GETPID:
            rc = kernelgetpid(uctxt);
            break;

        case YALNIX_BRK:
            rc = kernelbrk(uctxt);
            break;

        case YALNIX_DELAY:
            rc = kerneldelay(uctxt);
            break;

        case YALNIX_TTY_READ:
            rc = kernelttyread(uctxt);
            break;

        case YALNIX_TTY_WRITE:
            rc = kernelttywrite(uctxt);
            break;

        case YALNIX_REGISTER:
            rc = kernelregister(uctxt);
            break;

        case YALNIX_SEND:    
            rc = kernelsend(uctxt);
            break;

        case YALNIX_RECEIVE:
            rc = kernelreceive(uctxt);
            break;

        case YALNIX_RECEIVESPECIFIC:
            rc = kernelreceivespecific(uctxt);
            break;

        case YALNIX_REPLY:
            rc = kernelreply(uctxt);
            break;

        case YALNIX_FORWARD:
            rc = kernelforward(uctxt);
            break;

        case YALNIX_COPY_FROM:
            rc = kernelcopyfrom(uctxt);
            break;

        case YALNIX_COPY_TO:
            rc = kernelcopyto(uctxt);
            break;

        case YALNIX_READ_SECTOR:
            rc = kernelreadsector(uctxt);
            break;

        case YALNIX_WRITE_SECTOR:
            rc = kernelwritesector(uctxt);
            break;

        case YALNIX_PIPE_INIT:
            rc = kernelpipeinit(uctxt);
            break;

        case YALNIX_PIPE_READ:
            rc = kernelpiperead(uctxt);
            break;

        case YALNIX_PIPE_WRITE:
            rc = kernelpipewrite(uctxt);
            break;

            // case YALNIX_NOP:
            //     rc = kernelnop(uctxt);
            //     break;

            // case YALNIX_LOCK_INIT:
            //     rc = kernellockinit(uctxt);
            //     break;

            // case YALNIX_LOCK_ACQUIRE:
            //     rc = kernelaquire(uctxt);
            //     break;

            // case YALNIX_LOCK_RELEASE:
            //     rc = kernelrelease(uctxt);
            //     break;

            // case YALNIX_CVAR_INIT:
            //     rc = kernelcvarinit(uctxt);
            //     break;

            // case YALNIX_CVAR_SIGNAL:
            //     rc = kernelcavrsignal(uctxt);
            //     break;

            // case YALNIX_CVAR_BROADCAST:
            //     rc = kernelcarbroadcast(uctxt);
            //     break;

            // case YALNIX_CVAR_WAIT:
            //     rc = kernelcvarwait(uctxt);
            //     break;

            // case YALNIX_RECLAIM:
            //     rc = kernelreclaim(uctxt);
            //     break;

            // case YALNIX_CUSTOM_0:
            //     rc = kernelcustom0(uctxt);
            //     break;
            // case YALNIX_CUSTOM_1:
            //     rc = kernelcustom1(uctxt);
            //     break;
            // case YALNIX_CUSTOM_2:
            //     rc = kernelcustom2(uctxt);
            //     break;

            // case YALNIX_BOOT:
            //     rc = kernelboot(uctxt);
            //     break;

        default:
            break;
    }
    //Put the return code at the first register.
    uctxt->regs[0] = rc;
}

//Capture TRAP_CLOCK
//TODO: Implement round-robin process scheduling with CPU quantum per process of 1 clock tick.
void TrapClock(UserContext *uctxt){
    /*
       int rc = 0;
       IF(pqueue.size != 0)
       rc = KernelContextSwitch(MyKCS, (void *)current_pcb, (void *)netxt_pcb); 

     */
    TracePrintf(1, "TrapClock called\n");
    // int rc = 0;
    if (!isemptylist(waitingqueue)){

        lstnode *traverse = waitingqueue->head->next;
        while(traverse != NULL && traverse->id != -1) {               
            pcb_t* proc = TurnNodeToPCB(traverse);
            if(proc->clock > 0){
                proc->clock--;
                TracePrintf(1, "proc->clock:%d\n", proc->clock);
            }
            if(proc->clock == 0){
                lstnode* traverseNode = dewaitingqueue(traverse,waitingqueue);
                enreadyqueue(traverseNode,readyqueue);
            }

            traverse = traverse->next;   
        }

        // lstnode *notclock = waitingqueue->head->next;
        // while(notclock != NULL && notclock->id != -1){

        //     pcb_t* proc = TurnNodeToPCB(notclock);
        //     if(proc->clock == 0){
        //         TracePrintf(1, "notclock->clock:%d\n", proc->clock);
        //         lstnode* notclockNode = dewaitingqueue(notclock,waitingqueue);
        //         enreadyqueue(notclockNode,readyqueue);
        //     }
        //     notclock = notclock->next;
        // }
        
    }
    if (readyqueue->size  > 1){
        lstnode * node = dereadyqueue(readyqueue);
        if (node != currProc){
            TracePrintf(1, "The first node of readyqueue should be the current process!\n");
            return;
        }
        switchproc();
    }
}

//Capture TRAP_ILLEGAL
void TrapIllegal(UserContext *uctxt){
    /*
       Abort current process

       Rearrange quque 
     */
}

//Capture TRAP_MEMORY
void TrapMemory(UserContext *uctxt){
    TracePrintf(1,"TrapMemory\n");
    pcb_t *proc = TurnNodeToPCB(currProc);
    
    TracePrintf(1, "uctxt->addr:%p\n", uctxt->addr);
    
    int rc;
    int trapCode = uctxt->code;
    unsigned int newStackPage = ((unsigned int) uctxt->addr) >> PAGESHIFT;

    switch(trapCode){
        case (YALNIX_MAPERR):
            if (newStackPage > proc->stack_limit_page){
                terminateProcess(currProc);
                return;
            }

            if (newStackPage < proc->brk_page){
                terminateProcess(currProc);
                return;
            }

            rc = GrowUserStack(currProc,newStackPage);
            if (rc){
                terminateProcess(currProc);
                return;
            }
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
            break;
        case (YALNIX_ACCERR):
            terminateProcess(currProc);
            break;
        default:
            terminateProcess(currProc);
            break;
    }

    return;

    /*
       IF  [current break of heap] < uctxt->addr < [allocated memory for the stack](uctxt->ebp)
       keep going 
       ELSE 
       not Allocate    

       IF at least on page  
       Allocate memory for stack
       ELSE
       Abort current process

       Rearrange queue 
     */
}

//Capture TRAP_MATH
void TrapMath(UserContext *uctxt){
    /*
       Abort current process
       Rearrange queue
     */  
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

//Capture TRAP_DISK
void TrapDisk(UserContext *uctxt){
    /*
       DO SOMETHING......(Not specified in manual)
     */
}

void InitInterruptTable(){

    //Allocate memory to interupt vector table 
    intrptTb = (trapvector_t *) malloc(TRAP_VECTOR_SIZE * sizeof(trapvector_t));

    //Fill interrupt vector table
    intrptTb[TRAP_KERNEL] = &TrapKernel; 
    intrptTb[TRAP_CLOCK] = &TrapClock;
    intrptTb[TRAP_ILLEGAL]= &TrapIllegal;       
    intrptTb[TRAP_MEMORY] = &TrapMemory;    
    intrptTb[TRAP_MATH] = &TrapMath; 
    intrptTb[TRAP_TTY_RECEIVE] = &TrapTtyReceive;    
    intrptTb[TRAP_TTY_TRANSMIT] = &TrapTtyTransmit; 
    intrptTb[TRAP_DISK] = &TrapDisk;

    //Write talbe into register
    WriteRegister(REG_VECTOR_BASE,(unsigned int) intrptTb);

}
