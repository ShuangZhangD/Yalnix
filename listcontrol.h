#include "selfdefinedstructure.h"

lstnode* nodeinit(int i);
dblist* listinit();
lstnode* firstnode(dblist* list);
int isemptylist(dblist* list);
void insert_tail(lstnode* node, dblist* list);
void insert_head(lstnode* node, dblist* list);
lstnode* remove_tail(dblist* list);
lstnode* remove_head(dblist* list);
lstnode* search_node(int i,dblist* list);
lstnode* remove_node(int i,dblist* list);
void traverselist(dblist* list);

