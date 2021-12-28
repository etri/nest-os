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
 * queue.h - queue library implementation
 *
 */

#ifndef QUEUE_H
#define QUEUE_H
#include <pthread.h>

#define NODE_NOT_FOUND	1401

typedef struct qnode
{
        struct qnode *prev;
        struct qnode *next;
        void *value;
} Qnode;

typedef struct queue
{
        Qnode *head;
        Qnode *tail;
        int count; /* count can be deleted. But it is important for debugging */
	pthread_mutex_t lock;
	pthread_cond_t cond;

	// method
	int (*push)(struct queue *q, void *node);
	Qnode *(*pop)(struct queue *q);
	int (*delete)(struct queue *q, void *node);
	int (*is_empty)(struct queue *q);
} Queue;

void queue_init(Queue *q);
void queue_free(Queue *q);

#if 0
void print_queue(Queue *q);
void print_node(Qnode *node);
#endif
#endif
