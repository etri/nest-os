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
#ifndef SERVER_COMM_H
#define SERVER_COMM_H

#include "server_npuos.h"
#include "scheduler.h"

typedef struct server
{
	Scheduler *scheduler;
        int *npuos_num;
        ServerNpuOS *nos[MAX_NOS_NUM];

        void (*open)(struct server *);
        void (*handler)(struct server *);
        void (*close)(struct server *);
	void (*set_scheduler)(struct server *, int (*scheduler_function)(Scheduler *sched, int nnid, int part_num_start, int part_num_delta, unsigned int affinity_mask));
} Server;

/* server APIs */
Server *server_create(void);

#endif
