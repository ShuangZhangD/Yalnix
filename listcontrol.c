#include "selfdefinedstructure.h"
#include "listcontrol.h"

//use a doublylinkedlist to track free frame


lstnode* nodeinit(int i)
{
	lstnode* node = (lstnode *)MallocCheck(sizeof(lstnode));
	if (NULL == node){
		TracePrintf(1, "Malloc Failed in nodeinit! node is NULL!\n");
		return NULL;
	}
	node->id = i;
	node->pre = NULL;
	node->next = NULL;
	return node;
}

dblist* listinit()
{
	
	dblist* list = (dblist* ) MallocCheck(sizeof(dblist));
	if (NULL == list){
		TracePrintf(1, "Malloc Failed in nodeinit! list is NULL!\n");
		return NULL;
	}

	list->head = (lstnode *) MallocCheck(sizeof(lstnode));
	if (NULL == list->head){
		TracePrintf(1, "Malloc Failed in listinit! list->head is NULL!\n");
		return NULL;
	}
	list->head->id = -1;
	list->head->pre = NULL;
	list->head->next = NULL;
	list->tail = (lstnode *) MallocCheck(sizeof(lstnode));
	if (NULL == list->tail){
		TracePrintf(1, "Malloc Failed in listinit! list->tail is NULL!\n");
		return NULL;
	}

	list->tail->id = -1;
	list->tail->pre = NULL;
	list->tail->next = NULL;
	list->size = 0;
	return list;
}

lstnode* firstnode(dblist* list)
{
	if(isemptylist(list))
	{
		TracePrintf(1, "Error! The list is empty, cannot find first node!\n");
		return NULL;
	}
	else{
		return list->head->next;
	}
}

int isemptylist(dblist* list)
{
	// TracePrintf(1, "list->size:%d\n", list->size);
	if (list->size == 0)
	{
		return 1;
	}
	return 0;
}

void insert_tail(lstnode* node, dblist* list)
{
	if (isemptylist(list))
	{	
		node->next = list->tail;
		node->pre = list->head;
		list->head->next = node;
		list->tail->pre = node;
	}
	else{
		node->pre = list->tail->pre;
		node->next = list->tail;
		list->tail->pre->next = node;
		list->tail->pre = node;
	}
	list->size++;
}

void insert_head(lstnode* node, dblist* list)
{
	if(isemptylist(list))
	{
		node->next = list->tail;
		node->pre = list->head;
		list->head->next = node;
		list->tail->pre = node;
	}
	else{
		node->next = list->head->next ;
		node->pre = list->head;	
		list->head->next->pre = node;
		list->head->next = node;
	}
	list->size++;
}

lstnode* remove_tail(dblist* list)
{
	lstnode* tnode = list->tail->pre;
	if(isemptylist(list))
	{
		printf("%s\n", "error");
		return NULL;
	}
	else if(list->tail->pre->pre == NULL){
		list->head->next = list->tail;
		list->tail->pre = list->head;
		list->size--;
		return tnode;
	}
	else{
		list->tail->pre = list->tail->pre->pre;
		list->tail->pre = list->tail;
		list->size--;
		return tnode;
	}
}

lstnode* remove_head(dblist* list)
{
	lstnode* hnode = list->head->next;
	if(isemptylist(list))
	{
		printf("%s\n", "error");
		return NULL;
	}
	else if(list->head->next->next == NULL){
		list->head->next = list->tail;
		list->tail->pre = list->head;
		list->size--;
		return hnode;
	}
	else{
		list->head->next = list->head->next->next;
		list->head->next->pre = list->head;
		list->size--;
		return hnode;
	}
}

lstnode* search_node(int i,dblist* list)
{
	lstnode *h = NULL;
	if (!isemptylist(list))
	{ 
		h = list->head;
		while (h != NULL && h->id != i)
		{
			h = h->next;
		}
	}
	return h;
}

lstnode* remove_node(int i, dblist* list)
{
	if(isemptylist(list))
	{
		TracePrintf(1, "No node to remove!\n");
	}
	else{
		lstnode *remove = search_node(i,list);
		if (NULL == remove){
			TracePrintf(1, "Cannot find node in list!\n");
			return NULL;
		}
		remove->pre->next = remove->next;
		remove->next->pre = remove->pre;
		list->size--;
		return remove;
	}
}








