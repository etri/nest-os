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
#ifndef TASK_H
#define TASK_H

#include "share_comm.h"

#define TS_DORMANT	(0x00)
#define TS_READY	(0x01)
#define TS_RUN		(0X02)

typedef struct task
{
	struct task *prev;
	struct task *next;

	struct task *this;

	int priority;
	int state;

	void (*func)(void *args);
	void *args;
	SockMessage msg;// for arg with message 
} Task;

Task *task_create(void (*func)(void *args), void *args, int priority);
Task *task_message_create(void (*func)(void *args), void *args, int priority);
void task_free(Task *task);
#endif
