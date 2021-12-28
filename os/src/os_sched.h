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
#ifndef OS_TABLE_H
#define OS_TABLE_H

#include <pthread.h>
#include "task.h"
#include "queue.h"

typedef struct os_sched
{
	unsigned int ready_table[16];
	unsigned int ready_group;
	Queue ready_queue[256];
	Task *highest;
	Task *current;
	Task *idle;
	pthread_mutex_t lock;
	//unsigned int num_ready_tasks;

	void (*push)(struct os_sched *sched, Task *task);
	void (*remove)(struct os_sched *sched, Task *task);
} OsSched;

OsSched *os_sched_create(void);
int task_submit(OsSched *sched, Task *task);
#endif
