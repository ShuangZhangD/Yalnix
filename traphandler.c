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
#include "semaphore.h"

extern lstnode* currProc;
extern dblist* freeframe_list;
extern dblist* lockqueue;
extern dblist* cvarqueue;
extern dblist* pipequeue;

//capture TRAP_CLOCK
void TrapKernel(UserContext *uctxt){
    TracePrintf(2, "TrapKernel called\n");
    TurnNodeToPCB(currProc)->uctxt = *uctxt;
    int rc;
    switch(uctxt->code){
        case YALNIX_FORK:
            rc = KernelFork(uctxt);
            break;

        case YALNIX_EXEC: 
            rc = KernelExec(uctxt);
            break;

        case YALNIX_EXIT:
            rc = KernelExit(uctxt);
            break;

        case YALNIX_WAIT:
            rc = KernelWait(uctxt);
            break;

        case YALNIX_GETPID:
            rc = KernelGetPid(uctxt);
            break;

        case YALNIX_BRK:
            rc = KernelBrk(uctxt);
            break;

        case YALNIX_DELAY:
            rc = KernelDelay(uctxt);
            break;

        case YALNIX_TTY_READ:
            rc = KernelTtyRead(uctxt);
            break;

        case YALNIX_TTY_WRITE:
            rc = KernelTtyWrite(uctxt);
            break;

        case YALNIX_PIPE_INIT:
            rc = KernelPipeInit(uctxt);
            break;

        case YALNIX_PIPE_READ:
            rc = KernelPipeRead(uctxt);
            break;

        case YALNIX_PIPE_WRITE:
            rc = KernelPipeWrite(uctxt);
            break;

        case YALNIX_LOCK_INIT:
            rc = KernelLockInit(uctxt);
            break;

        case YALNIX_LOCK_ACQUIRE:
            rc = KernelLockAcquire(uctxt);
            break;

        case YALNIX_LOCK_RELEASE:
            rc = KernelLockRelease(uctxt);
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
        case YALNIX_SEM_INIT:
            rc = KernelSemInit(uctxt);
            break;
        case YALNIX_SEM_UP:
            rc = KernelSemUp(uctxt);
            break;
        case YALNIX_SEM_DOWN:
            rc = KernelSemDown(uctxt);
            break;
        case YALNIX_RECLAIM:
            rc = KernelReclaim(uctxt);
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
     */
    TracePrintf(2, "Enter TrapIllegal\n");
    ProcessExit();
    return;
}


//Capture TRAP_MATH
void TrapMath(UserContext *uctxt){
    /*
       Abort current process
     */ 
    TracePrintf(2, "Enter TrapMath\n");
    ProcessExit();
    return;
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
