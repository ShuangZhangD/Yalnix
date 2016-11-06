#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "processmanage.h"

int kernelttywrite(UserContext *uctxt);
int kernelttywrite(UserContext *uctxt);

Tty* tty[NUM_TERMINALS];
int receivelen;

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt);

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt);
