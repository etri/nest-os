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
#include "os_sched.h"
#include "task.h"
#include "queue.h"

/* management of tables */
const unsigned int map_table[16]  = {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000};

const unsigned int unmap_table[256]  = 
		       {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
			5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
			6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
			6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
			6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
			6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};

static void cal_highest_task(OsSched *sched)
{
	unsigned int x, y;

    	// find y
    	y = sched->ready_group;
    	if (y>>8) // upper nibble
    	{
       		y = (unsigned int)(unmap_table[y>>8] + 8);
    	}
    	else
    	{
       		y = unmap_table[y & 0xff]; // lower nibble
    	}

    	// find x
    	x = sched->ready_table[y];
    	if (x>>8) // upper nibble
    	{
       		x = (unsigned int)(unmap_table[x>>8] + 8);
    	}
	else
    	{
       		x = unmap_table[x & 0xff]; // lower nibble
	}
	
	unsigned int prio;

	prio = (y << 4) + x;

	sched->highest = (Task *) sched->ready_queue[prio].head;
}

static void sched_qpush(OsSched *sched, Task *task)
{
pthread_mutex_lock(&sched->lock);
	unsigned int prio = task->priority;
	Queue *q = &sched->ready_queue[prio];

	if (q->is_empty(q))
	{	
		sched->ready_group             |= 	map_table[prio >>    4];
		sched->ready_table[prio >> 4]	|= 	map_table[prio &  0x0f];
	}

	/* enqueue error cannot occur already queue is checked if it is full */
	q->push(q, task); 
	task->state = TS_READY;
	cal_highest_task(sched);
	//sched->num_ready_tasks++;
pthread_mutex_unlock(&sched->lock);
}

static void sched_qremove(OsSched *sched, Task *task)
{
pthread_mutex_lock(&sched->lock);
	unsigned int prio = task->priority;
	Queue *q = &sched->ready_queue[prio];

	q->delete(q, task);

	if (q->is_empty(q))
	{
		if ((sched->ready_table[prio >> 4] &= (unsigned int)(~map_table[prio & 0x0f])) == 0)
		{
			sched->ready_group &= (unsigned int)(~map_table[prio >> 4]);
		}
	}
	task->state = TS_DORMANT;
	cal_highest_task(sched);
	//sched->num_ready_tasks--;
pthread_mutex_unlock(&sched->lock);
}

OsSched *os_sched_create(void)
{
	OsSched *sched;

	sched = (OsSched *) malloc(sizeof(OsSched));

	if (!sched)
	{
		printf("Error: os_sched object cannot be created\n");
		exit(1);
	}

	// init included
	sched->highest = NULL;
	sched->current = NULL;
	sched->ready_group = 0;
	for (int i=0; i<16; i++)
	{
		sched->ready_table[i] = 0;
	}

	for (int i=0; i<256; i++)
	{
		queue_init(&sched->ready_queue[i]);
	}

	sched->push = sched_qpush;
	sched->remove = sched_qremove;

	pthread_mutex_init(&sched->lock, NULL);

	return sched;
}
