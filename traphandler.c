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
#include "listcontrol.h"


extern lstnode* currProc;
extern dblist* freeframe_list;
extern dblist* lockqueue;
extern dblist* cvarqueue;
extern dblist* pipequeue;

//capture TRAP_CLOCK
void TrapKernel(UserContext *uctxt){
    TracePrintf(1, "TrapKernel called\n");
    TurnNodeToPCB(currProc)->uctxt = *uctxt;
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

        case YALNIX_PIPE_INIT:
            rc = kernelpipeinit(uctxt);
            break;

        case YALNIX_PIPE_READ:
            rc = kernelpiperead(uctxt);
            break;

        case YALNIX_PIPE_WRITE:
            rc = kernelpipewrite(uctxt);
            break;

        case YALNIX_LOCK_INIT:
            rc = kernellockinit(uctxt);
            break;

        case YALNIX_LOCK_ACQUIRE:
            rc = kernelaquire(uctxt);
            break;

        case YALNIX_LOCK_RELEASE:
            rc = kernelrelease(uctxt);
            break;

        case YALNIX_CVAR_INIT:
            rc = KernelCvarInit(uctxt);
            break;

        case YALNIX_CVAR_SIGNAL:
            rc = KernelCvarSignal(uctxt);
            break;

        case YALNIX_CVAR_BROADCAST:
            rc = KernelCvarBroadcast(uctxt);
            break;

        case YALNIX_CVAR_WAIT:
            rc = KernelCvarWait(uctxt);
            break;

        case YALNIX_RECLAIM:
            rc = kernelreclaim(uctxt);
            break;

        default:
            break;
    }
    //Put the return code at the first register.
    uctxt->regs[0] = rc;
}


//Capture TRAP_ILLEGAL
void TrapIllegal(UserContext *uctxt){
    /*
       Abort current process

       Rearrange quque 
     */
    kernelexit(uctxt);

}


//Capture TRAP_MATH
void TrapMath(UserContext *uctxt){
    /*
       Abort current process
       Rearrange queue
     */ 
    kernelexit(uctxt);

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
