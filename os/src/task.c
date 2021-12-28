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
#include "task.h"
#include "os_sched.h"

Task *task_create(void (*func)(void *args), void *args, int priority)
{
        Task *task;

        task = malloc(sizeof(Task));

        if (!task)
        {
                printf("Error: task object cannot be created\n");
                exit(1);
        }

        task->priority = priority;
        task->func = func;
        task->args = args;

        return task;
}

Task *task_message_create(void (*func)(void *args), void *args, int priority)
{
        Task *task;

        task = malloc(sizeof(Task));

        if (!task)
        {
                printf("Error: task object cannot be created\n");
                exit(1);
        }

        task->priority = priority;
        task->func = func;
	task->msg = *((SockMessage *)args);
        task->args = &task->msg;

        return task;
}

void task_free(Task *task)
{
	free(task);
}

int task_submit(OsSched *sched, Task *task)
{
	int status = 0;

	sched->push(sched, task);

	return status;
}

