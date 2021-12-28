/*****************************************************************************
*
* Copyright Next-Generation System Software Research Group, All rights reserved.
* Future Computing Research Division, Artificial Intelligence Reserch Laboratory
* Electronics and Telecommunications Research Institute (ETRI)
*
* THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE
* WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS
* TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE
* REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL
* SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,
* IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE
* COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
*
* LICENSE file : README_LICENSE_ETRI located in the top directory
*
*****************************************************************************/
/*
 * queue.c - queue library implementation
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "error.h"

int node_push(Queue *q, void *n)
{
	Qnode *node = (Qnode *)n;
	if (q==NULL || node == NULL)
	{
		printf("OBJ unavailable\n");
		return -1;
	}

	if (q->head == NULL)
	{
		q->head = node;
		q->tail = node;
		node->prev = NULL;
		node->next = NULL;
	}
	else
	{
		node->prev = q->tail;
		node->next = NULL;
		q->tail->next = node;
		q->tail = node;
	}

	q->count++;

	return 0;
}

Qnode *node_pop(Queue *q)
{
	if (q==NULL)
	{
		printf("OBJ unavailable");
		return NULL;
	}

	Qnode *p;

	p = q->head;

	if (p) // found
	{
		q->head = p->next;

		if (p->next != NULL)
		{
			p->next->prev = p->prev;
		}
		else
		{
			q->tail = NULL;
		}

		q->count--;
	}

	return p;
}

int node_delete(Queue *q, void *n)
{
	if (!n) 
		return -1;

	Qnode *node = (Qnode *)n;

	if (node->prev != NULL && node->next != NULL)
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
		node->next = NULL;
		node->prev = NULL;
	}
	else if (node->prev == NULL && node->next != NULL)
	{
		q->head = node->next;

		node->next->prev = NULL;
		node->next = NULL;
	}
	else if (node->prev != NULL && node->next == NULL)
	{
		q->tail = node->prev;

		node->prev->next = NULL;
		node->prev = NULL;
	}
	else /* p->prev == NULL; p->next == NULL */
	{
		q->head = q->tail = NULL;
	}

	q->count--;

	return 0;
}

int queue_is_empty(Queue *q)
{
	return (!q->head);
}

void queue_init(Queue *q)
{
	q->head = q->tail = NULL;
	q->count = 0;

	q->push = node_push;
	q->pop = node_pop;
	q->delete = node_delete;
	q->is_empty = queue_is_empty;
}

void queue_free(Queue *q)
{
	Qnode *p, *r;

pthread_mutex_lock(&q->lock);
	for (p=q->head; p!=NULL; p=r)
	{
		r=p->next; // r is temporal 
		free(p);
	}	
pthread_mutex_unlock(&q->lock);
}

#if 0
/* This code is for debugging */
void print_queue(Queue *q)
{
	Qnode *p;

	for (p=q->head; p!=NULL; p = p->next)
	{
		sysLogOutMsg("value = %d\n", p->value);
	}
}

void print_node(Qnode *node)
{
	sysLogOutMsg("value = %d\n", node->value);
}
#endif

