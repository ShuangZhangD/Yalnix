#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "processmanage.h"


/*
	1. Use Shadow node in writerwaitingqueue
	2. Use Dynamic block(process are in the ready queue, but they will not be executed unless condition satisfied.)
	3. Compare current node with first node in writerwaitingqueue
*/
int KernelTtyWrite(UserContext *uctxt);

/*
	1. Use message nodes to read input from TtyTerminals
	2. Supports a larger-than-TERMINAL_MAX_LINE input string. 
	   However, if the input string is too long, Yalnix will empty all message nodes in order to save memory
*/
int KernelTtyRead(UserContext *uctxt);

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt);

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt);

// int receivelen;
