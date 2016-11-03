#include "selfdefinedstructure.h"
#include "listcontrol.h"

//use a doublylinkedlist to track free frame


lstnode* nodeinit(int i)
{
	lstnode* node = (lstnode *)malloc(sizeof(lstnode));
	node->id = i;
	node->pre = NULL;
	node->next = NULL;
	return node;
}

dblist* listinit()
{
	
	dblist* list = (dblist* )malloc(sizeof(dblist));

	list->head = (lstnode *)malloc(sizeof(lstnode));
	list->head->id = -1;
	list->head->pre = NULL;
	list->head->next = NULL;
	list->tail = (lstnode *)malloc(sizeof(lstnode));
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
		TracePrintf(1, "%s\n", "error\n");
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








