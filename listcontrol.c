#include "datastructure.h"
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
	
	lstnode* n = (lstnode *)malloc(sizeof(lstnode));
	dblist* list = (dblist* )malloc(sizeof(dblist));
	n->id = 0;
	n->pre = NULL;
	n->next = NULL;
	list->head = n;
	list->size = 0;
	list->tail = n;
	return list;
}

lstnode* firstnode(dblist* list)
{
	if(isemptylist(list))
	{
		printf("%s\n", "error");
		return NULL;
	}
	else{
		return list->head->next;
	}
}

int isemptylist(dblist* list)
{
	if (list->size == 0 && list->head->next == NULL)
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

lstnode* search_node(lstnode* node,dblist* list)
{
	lstnode *h = list->head;
	if (!isemptylist(list))
	{
		while (h != NULL && h->id != node->id)
		{
			h = h->next;
		}
	}
	return h;
}

void remove_node(lstnode* node, dblist* list)
{
	if(isemptylist(list))
	{
		printf("%s\n", "error");
	}
	else{
		lstnode *remove = search_node(node,list);
		remove->pre->next = remove->next;
		remove->next->pre = remove->pre;
		list->size--;
	}
}

void traverselist(dblist* list)
{
	lstnode *traverse = list->head;
	if(isemptylist(list))
	{
		printf("%s\n", "error");
	}
	else
		while(traverse != NULL)
		{
				traverse = traverse->next;
				printf("%s\n", traverse->id);
		}

}







