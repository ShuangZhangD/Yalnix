#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "processmanage.h"

int KernelTtyWrite(UserContext *uctxt);
int KernelTtyRead(UserContext *uctxt);

Tty* tty[NUM_TERMINALS];
int receivelen;

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt);

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt);
