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
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <malloc.h>
#include "error.h"
#include "share_comm.h"
#include "npuos.h"
#include "task.h"
#include "nn_object.h"
#include "nn_object_list.h"
#include "support_model.h"
#include "util_time.h"
#include "sock_handler.h"
#include "os_shm.h"

#define NEST_DBG	0

static void npuos_set_id(NpuOS *npuos, int id)
{
	npuos->id = id;
}

static void npuos_set_hwtype(NpuOS *npuos, int type)
{
	npuos->hwtype = type;
}

static void npuos_set_npu(NpuOS *npuos, void *npu)
{
	npuos->npu = npu;
}

static void npuos_set_support(NpuOS *npuos, int nnid)
{
	int i = nnid / 32;
	int j = nnid % 32;

	npuos->support[i] |= (1 << j);
}

static void npuos_set_serverip(NpuOS *npuos, char *ipaddr)
{
	if (!npuos->server_ip)
	{
		npuos->server_ip = malloc(sizeof(char)*20); // 20 bytes
		if (!npuos->server_ip)
		{
			printf("Error: npuos->server_ip cannot be created\n");
			exit(1);
		}
	}

	if (!ipaddr)
	{
		printf("Error: no ip address given\n");
	}
	else
	{
		strcpy(npuos->server_ip, ipaddr);
	}
}

static void npuos_close(NpuOS *npuos)
{
	npuos_sock_close(npuos);
}


static void idle_task(void *args)
{
	NpuOS *npuos = (NpuOS *)args;
	volatile OsSched *sched = npuos->sched;

	//printf("Enter idle task on %d\n", npuos->id);
	while (sched->highest==sched->current && !npuos->shutdown)
	{
		//printf("I am idle task on NPU %d : close=%d\n", npuos->id, os_done);
		//sleep(1);
	}
	//printf("Exit idle task on %d\n", npuos->id);
}

static void task_executor(NpuOS *npuos)
{
	// preprocess if any

	// execute this task until end
	if (!npuos->shutdown)
	{
		OsSched *sched = npuos->sched;
		Task *task = sched->current;

		task->state = TS_RUN;
		task->func(task->args);

		// delete this task if not idle task
		if (task != sched->idle)
		{		
			sched->remove(sched, task);
			task_free(task); // deallocate the task
			npuos->ntasks--;
		}
	}

	// post-process if nay

}

static void *npuos_function(void *args)
{
	NpuOS *npuos = (NpuOS *)args;
	OsSched *sched = npuos->sched;

	// State1 : OS ready
	npuos->state = OS_READY;

	// create the message queue for this NPU OS
	npuos->init(npuos);

	// Register Neural-Net Objects (NnObject)
	npuos->object_register(npuos);

	// connect to the server to send the os information
	npuos->connect(npuos);

	// submit idle tasks
	Task *task;
	task = task_create(idle_task, args, 0); // priority = 0
	sched->idle = task; // record idle task on sched object
	task_submit(sched, task);

	// create handler thread to accept incoming requests. e.g. LOAD, PREDICT
	pthread_t handler;

	if (pthread_create(&handler, 0, sock_npuos_handler, args))
       	{
       		fprintf(stderr,"sock handler thread creation failed\n");
       	}

	// State2 : OS running
	npuos->state = OS_RUN;
	while (!npuos->shutdown)
	{
		if (sched->current != sched->highest)
		{
			sched->current = sched->highest;
			task_executor(npuos);
		}
	}

	printf("OS shutdown!\n");

	// State3 : OS shutdown
	npuos->state = OS_SHUTDOWN;

	// uninstall all nn_objects: find all installed NN, scan from 0 too 255
	for (int i=0; i<256; i++)
	{
		if (npuos->nn_objects[i])
		{
			NnObject *nn_object = npuos->nn_objects[i];

			if (nn_object->loaded)
			{
				// TO DO : unload all partitions if any

				//NetInfo info;
				//info.part_num_start = part_num_start;
                        	//info.part_num_delta = part_num_delta;
				//nn_object->unload(nn_object, &info);
				printf("Unload NNID %d\n", i);
				nn_object->loaded = 0;
			}

			// delete the load file if any
			if (nn_object->load_filename_buf)
			{
				char *filename = nn_object->load_filename_buf;
				remove(filename);
				printf("Unload File : %s\n", filename);
				free(filename);
				nn_object->load_filename_buf = NULL;
			}

			// free created nn_object
			nn_object_free(nn_object);
		}
	}

#if (NEST_DBG==1)
	printf("join the OS handler\n");
#endif
	// join the OS handler thread
	int status = pthread_join(handler, 0);
	if (status)
	{
		 printf("pthread join for handler is failed for NPU OS%d\n", npuos->id);
	}

#if (NEST_DBG==1)
	printf("free OS message queue\n");
#endif

	// free OS message queue
	npuos_close(npuos);

	return NULL;
}

static void npuos_init(NpuOS *npuos)
{
	npuos_sock_init(npuos);	 // socket interface init
}

static void npuos_object_register(NpuOS *npuos)
{
	int nn_object_num = sizeof(obj_list)/sizeof(NnObjectList);
	//printf("nn_object_num = %d\n", nn_object_num);

	for (int i=0; i<nn_object_num; i++)
	{
		NnObject *nnobj = nn_object_create();
		NnObjectList nnobj_l = obj_list[i];
		*nnobj = *(nnobj_l.object); // copy NN Object
		int nnid = nnobj_l.nnid;

		npuos->nn_objects[nnid] = nnobj;
		npuos->nn_objects[nnid]->npu = npuos->npu;
		npuos_set_support(npuos, nnid);		
	}
}

static void npuos_connect(NpuOS *npuos)
{
	npuos_sock_connect(npuos); // socket interface connect
}

static void npuos_start(NpuOS *npuos)
{
	npuos_function(npuos);
}

void super_task(void *value)
{
	MsgServerDmkill *msg_dmkill = (MsgServerDmkill *)value;
	NpuOS *npuos = (NpuOS *) msg_dmkill->npuos;

#if (NEST_DBG==1)
	printf("Super task enters on NPU %d.\n", npuos->id);
#endif

	npuos->shutdown = 1;

	//free(msg_dmkill);
}

NpuOS *npuos_create(void *wsmem)
{
	NpuOS *npuos = malloc(sizeof(NpuOS));

	if (!npuos)
	{
		perror("Memory is insufficient!\n");
		return NULL;
	}

#if 1
	if (!wsmem)
	{
		printf("Default memory is set\n");
		npuos->wsmem = malloc(NOS_WSMEM_SIZE);
		if (!npuos->wsmem)
		{
			perror("Memory is insufficient! (DEFAULT_SHM_SIZE)\n");
			return NULL;
		}
	}
	else
#endif
	{
		npuos->wsmem = wsmem;
		printf("Shared Memory : %p\n", wsmem);
	}

	npuos->wsmem_size = NOS_WSMEM_SIZE;

	npuos->init = npuos_init;
	npuos->object_register = npuos_object_register;
	npuos->connect = npuos_connect;
	npuos->set_id = npuos_set_id;
	npuos->set_hwtype = npuos_set_hwtype;
	npuos->set_npu = npuos_set_npu;
	npuos->set_serverip = npuos_set_serverip;
	npuos->start = npuos_start;

	npuos->id = -1; // not determined yet, in this case Server distributes it
	npuos->version = VERSION;
	npuos->baseos = BASEOS;

	npuos->state = OS_DORMANT;
	npuos->ntasks = 0;

	npuos->shutdown = 0;

	// internal scheduler
	npuos->sched = os_sched_create();

	for (int i=0; i<8; i++) // 8*4 bytes = 256 bits 
	{
		npuos->support[i] = 0;
	}

	for (int i=0; i<256; i++) // 256 NN_OBJECTs are supported
	{
		npuos->nn_objects[i] = NULL;
	}

	npuos->server_ip = NULL;

	return npuos;
}

void npuos_destroy(NpuOS *npuos)
{
	free(npuos);
}
