#include "testutil.h"

extern pte_t g_pageTableR0[MAX_PT_LEN];

void printUserPageTable(lstnode *p){
    TracePrintf(1, "Print User Page Table.\n");
    int i;

    pcb_t *proc = TurnNodeToPCB(p);

    for (i = 0; i < MAX_PT_LEN; i++){

        int v = proc->usrPtb[i].valid;
        int prot = proc->usrPtb[i].prot;

        char *read = NULL;
        char *write = NULL;
        char *exec = NULL;
        if (prot & PROT_READ) read = "PROT_READ";
        if (prot & PROT_WRITE) write = "PROT_WRITE";
        if (prot & PROT_EXEC) exec = "PROT_EXEC";

        int pfn = proc->usrPtb[i].pfn;

        TracePrintf(1, "Entry %d: valid:%d, PROT=%x, PROT_READ=%s PROT_WRITE=%s PROT_EXEC=%s, pageFrameNumber:%d\n",i,v,prot,read,write,exec,pfn);
    }
    return;
}

void printKernelPageTable(){
    TracePrintf(1, "Print Kernel Page Table.\n");
    int i;
    for (i = 0; i < MAX_PT_LEN; i++){
        int v = g_pageTableR0[i].valid;
        int prot = g_pageTableR0[i].prot;

        char *read = NULL;
        char *write = NULL;
        char *exec = NULL;
        if (prot & PROT_READ) read = "PROT_READ";
        if (prot & PROT_WRITE) write = "PROT_WRITE";
        if (prot & PROT_EXEC) exec = "PROT_EXEC";

        int pfn = g_pageTableR0[i].pfn;

        TracePrintf(1, "Entry %d: valid:%d, PROT_READ=%s PROT_WRITE=%s PROT_EXEC=%s, pageFrameNumber:%d\n",i,v,read,write,exec,pfn);

    }
    return;
}


void traverselist(dblist* list)
{
    lstnode *traverse = list->head;
    if(isemptylist(list))
    {
        TracePrintf(1, "%s\n", "Error");
    }
    else
        while(traverse->next != NULL)
        {
                traverse = traverse->next;
                TracePrintf(1, "List Id:%d\n", traverse->id);
        }

}