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
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int node_push(Queue *q, Qnode *node)
{
        if (q==NULL || node == NULL)
        {
                printf("Queue or node is unavailable\n");
		exit(1);
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
        if (!q)
        {
                printf("Queue is unavailable");
		exit(1);
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

Qnode *node_search(Queue *q, int part_num_start, int part_num_delta)
{
        if (!q)
	{
                printf("Queue is unavailable");
		exit(1);
	}
	
	Qnode *p;

        for (p=q->head; p!=NULL; p=p->next)
        {
		if ((p->part_num_start==part_num_start) && (p->part_num_delta==part_num_delta))
		{
			return p; // found
		}
        }

	return NULL; // not found
}

int node_delete(Queue *q, Qnode *node)
{
        if (!q || !node)
                return -1;

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

Qnode *node_create(void)
{
	Qnode *node = malloc(sizeof(Qnode));
	if (!node)
	{
		printf("Error: node cannot be created\n");
		exit(1);
	}

	node->prev = node->next = NULL;

	node->part_num_start = 0;
	node->part_num_delta = 0;
	node->wcet = 0.0;

	return node;
}

Queue *queue_create(void)
{
	Queue *q = malloc(sizeof(Queue));
	if (!q)
	{
		printf("Error: q cannot be created\n");
		exit(1);
	}

        q->head = q->tail = NULL;
        q->count = 0;

        q->push = node_push;
        q->pop = node_pop;
        q->search = node_search;
        q->delete = node_delete;
        q->is_empty = queue_is_empty;

	return q;
}

void queue_free(Queue *q)
{
        Qnode *p, *r;

        for (p=q->head; p!=NULL; p=r)
        {
                r=p->next; // r is temporal
                free(p);
        }
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

