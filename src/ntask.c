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
#include <sys/types.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <malloc.h>
#include "share_comm.h"
#include "ntask.h"
#include "bit_handler.h"

/* Ntask APIs */
void *ntask_get_memory_pointer(Ntask *nt)
{
	return nt->memory_pointer; 
} 

static void sock_portnum_init(Ntask *nt)
{
	if (!nt->msg_predict.port)
	{
		MsgClientPort msg_port = {CL_PORT, nt->msgq_inf->msgq_client};

        	nt->msgq_inf->send_msg(nt->msgq_inf->msgq_server, &msg_port);

		Message rmsg;
		nt->msgq_inf->recv_msg(nt->msgq_inf->msgq_client, &rmsg); // from server
		if (rmsg.mtype == ECHO_OK)
		{
			MsgClientPortEcho *rmsg_port = (MsgClientPortEcho *)&rmsg;

			nt->msg_predict.port = rmsg_port->port;
		}
		else
		{
			printf("Error : Port number cannot be obtained\n");
			exit(1);
		}
	}
}

static void ntask_run(Ntask *nt)
{
	/////////////////////////////////////// run input_handler first
	if (nt->input_handler)
	{
		nt->input_handler(nt);
	}
	///////////////////////////////////////

	// Step 1 : Scheduling Phase
	nt->msg_predict.mtype = CL_PREDICT_S1;
        nt->msgq_inf->send_msg(nt->msgq_inf->msgq_server, &nt->msg_predict);

	Message rmsg;
	nt->msgq_inf->recv_msg(nt->msgq_inf->msgq_client, &rmsg); // from server

	MsgClientEcho *msg = (MsgClientEcho *)&rmsg;
	if (msg->mtype == ECHO_ERR)
	{
		printf("Error msg received. Probably package is not loaded.\n");
		exit(1);
	}
	else
	{
		nt->msg_predict.osid = msg->osid;
	}

	// Step 2 : Data Communication Phase
	nt->msg_predict.mtype = CL_PREDICT_S2;

////////////////////////////////////// SEND FILE OR/AND DATA ///////////////////////////////////
	// Open Socket Port
	sock_portnum_init(nt);

	// socket interfacing with NPU OS to send image data
	int server_fd = -1;

	nt->sock_inf->server_init(&server_fd, nt->msg_predict.port);

        nt->msgq_inf->send_msg(nt->msgq_inf->msgq_server, &nt->msg_predict);

	// blocking I/O to accept a ntask connection
	int ntask_fd = nt->sock_inf->server_accept(&server_fd);

if (nt->indata_push) 
{
	//printf("indata_push\n");
	// Send input file to NOS
	if (nt->input_type & INPUT_FILE_BIT)
	{
		nt->sock_inf->send_filename(ntask_fd, nt->infile_name);
		nt->sock_inf->send_file(ntask_fd, nt->fd, nt->infile_size);
	}

	// Send input data to NOS
	if (nt->input_type & INPUT_DATA_BIT)
	{
		nt->sock_inf->send_data(ntask_fd, nt->memory_pointer+nt->indata_start_offset, nt->indata_size);
	}
}

////////////////////////////////////// RECV DATA ///////////////////////////////////
	// Receive output data from NOS
	SockMsgOSOutdataEcho smsg;
	recv(ntask_fd, &smsg, sizeof(SockMsgOSOutdataEcho), 0);
	nt->outdata_start_offset = smsg.data_offset;
	nt->outdata_size = smsg.data_size;

if (nt->outdata_pull) 
{
	//printf("outdata_pull\n");
	nt->sock_inf->recv_data(ntask_fd, nt->memory_pointer+nt->outdata_start_offset, nt->outdata_size);
}

////////////////////////////////////////////////////////////////////////////////////

	nt->msgq_inf->recv_msg(nt->msgq_inf->msgq_client, &rmsg); // from server
	if (rmsg.mtype == ECHO_ERR)
	{
		printf("Error msg received. Prediction was not made.\n");
		exit(1);
	}
	else
	{
        	close(ntask_fd);
               	close(server_fd);
		//printf("Prediction is successful.\n");
	}

	/////////////////////////////////////// run output handler last
	if (!nt->output_handler)
	{
		//printf("Output handler is not registered yet\n");
	}
	else
	{
		nt->output_handler(nt);
	}
	///////////////////////////////////////
}

static void ntask_set_data_input(Ntask *nt, int indata_start_offset, int indata_size)
{
	nt->indata_start_offset = indata_start_offset;
	nt->indata_size = indata_size;
	nt->msg_predict.indata_start_offset = indata_start_offset;
	nt->msg_predict.indata_size = indata_size;
	nt->msg_predict.input_type |= INPUT_DATA_BIT;
	nt->input_type |= INPUT_DATA_BIT;
}

static void ntask_unset_data_input(Ntask *nt)
{
	nt->input_type &= ~INPUT_DATA_BIT;
}

static void ntask_get_data_output(Ntask *nt, int *outdata_start_offset, int *outdata_size)
{
	*outdata_start_offset = nt->outdata_start_offset;
	*outdata_size = nt->outdata_size; 
}

static void ntask_set_file_input(Ntask *nt, char *filepath)
{
	int fd;

	nt->infile_name = basename(filepath);

	fd = open(filepath, O_RDONLY);

	if (fd != -1)
	{
		struct stat obj;
		stat(filepath, &obj);
		nt->infile_size = obj.st_size;
		nt->fd = fd;
		nt->msg_predict.input_type |= INPUT_FILE_BIT;
		nt->msg_predict.infile_size = nt->infile_size;
		nt->input_type |= INPUT_FILE_BIT;
	}
	else
	{
		printf("File %s is missing in the directory!\n", nt->infile_name);
		exit(1);
	}
}

static void ntask_unset_file_input(Ntask *nt)
{
	nt->input_type &= ~INPUT_FILE_BIT;
}

static unsigned int ntask_get_nos_mask(Ntask *nt)
{
	unsigned int mask;

	MsgClientPredict msg_mask;

	msg_mask.mtype = CL_GET_NOS_MASK;

	msg_mask.mqid = nt->msgq_inf->msgq_client;
	msg_mask.nnid = nt->nnid;
	msg_mask.part_num_start = nt->part_num_start;
	msg_mask.part_num_delta = nt->part_num_delta;

	nt->msgq_inf->send_msg(nt->msgq_inf->msgq_server, &msg_mask);

	Message rmsg;
	nt->msgq_inf->recv_msg(nt->msgq_inf->msgq_client, &rmsg); // from server

	MsgClientEcho *msg = (MsgClientEcho *)&rmsg;
	if (msg->mtype == ECHO_ERR)
	{
		printf("Error msg received. Probably package is not loaded.\n");
		exit(1);
	}
	else
	{
		mask = msg->mask;
	}

	return mask;
}

static void ntask_set_affinity(Ntask *nt, unsigned int mask)
{
	nt->affinity_mask = mask;
	nt->msg_predict.affinity_mask = mask;
}

static void ntask_set_priority(Ntask *nt, int priority)
{
	nt->priority = priority;
	nt->msg_predict.priority = priority;
}

static void ntask_set_memory(Ntask *nt, void *memptr)
{
	if (memptr)
	{
		if (nt->memory_pointer)
		{
			free(nt->memory_pointer); // free default memory
		}

		nt->memory_pointer = memptr;
		nt->memory_size = malloc_usable_size(memptr);
	}
	else
	{
		printf("invalid memory argument! memory pointer null\n");
		exit(1);
	}
}

static void ntask_free_memory(Ntask *nt)
{
	if (nt->memory_pointer)
	{
		free(nt->memory_pointer);
		nt->memory_pointer = NULL;
		nt->memory_size = 0;
	}
}

static void ntask_exit(Ntask *nt)
{
	if (!nt->memory_pointer)
	{
		free(nt->memory_pointer);
	}

	// destory message queue
	msgctl(nt->msg_predict.mqid, IPC_RMID, 0);

	// free ntask memory
	free(nt);
}

static int ntask_load(Ntask *nt, int osid)
{
	return (nt->loader->load(nt->loader, nt->nnid, osid));
}

static int ntask_unload(Ntask *nt, int osid)
{
	return (nt->loader->unload(nt->loader, nt->nnid, osid));
}

static void ntask_set_load_file(Ntask *nt, char *filepath)
{
	nt->loader->set_file(nt->loader, filepath);
}

static void ntask_set_partition(Ntask *nt, int part_num_start, int part_num_end)
{
	// partition valid range check
	if (part_num_start >= BIT_MAX_NUM || part_num_end >= BIT_MAX_NUM)
	{
		printf("Error : partition number cannot exceed %d\n", BIT_MAX_NUM-1);
		exit(1);
	}
	else if (part_num_start > part_num_end)
	{
		printf("Error : partition number start cannot be greater than partition number end\n");
		exit(1);
	}

	nt->part_num_start = part_num_start;
	nt->part_num_delta = part_num_end - part_num_start;
	nt->msg_predict.part_num_start = part_num_start;
	nt->msg_predict.part_num_delta = part_num_end - part_num_start;
}

static void ntask_disable_data_push(Ntask *nt)
{
	nt->indata_push = 0;
}

static void ntask_disable_data_pull(Ntask *nt)
{
	nt->outdata_pull = 0;
}

static void ntask_next_nt(Ntask *nt, Ntask *nt_next)
{
        nt->succ[nt->num_succ++] = nt_next;
        nt_next->pred[nt_next->num_pred++] = nt;
	
	// notify nprocess for graph changed
}

Ntask *ntask_create(char *name, int nnid, void (*input_handler)(Ntask *nt), void (*output_handler)(Ntask *nt))
{
	Ntask *ntask;

	ntask = malloc(sizeof(Ntask));
	if (!ntask)
	{
		printf("Error: ntask cannot be created\n");
		exit(1);
	}

	strcpy(ntask->name, name);
	pthread_mutex_init(&ntask->lock, NULL);
	ntask->nnid = nnid;
	ntask->affinity_mask = ~0;
	ntask->priority = 10; // default_priority
	ntask->input_type = 0;
	ntask->fd = 0;
	ntask->infile_size = 0;
	ntask->memory_pointer = NULL;
	ntask->indata_start_offset = 0;
	ntask->indata_size = 0;
	ntask->outdata_start_offset = 0;
	ntask->outdata_size = 0;

	ntask->indata_push = 1; // data push enable
	ntask->outdata_pull = 1; // data pull enable

	ntask->port = 0;
	ntask->part_num_start = 0;
	ntask->part_num_delta = 0;

	ntask->np = NULL;
	ntask->next = NULL;

	ntask->num_pred = 0;
 	ntask->num_succ = 0;
        ntask->num_vpred = 0;
        ntask->num_vsucc = 0;

        for (int i=0; i<MAX_PRED_NUM; i++)
        {
                ntask->pred[i] = NULL;
        }

        for (int i=0; i<MAX_SUCC_NUM; i++)
        {
                ntask->succ[i] = NULL;
        }

	ntask->loader = loader_create();

	ntask->input_handler = input_handler;
	ntask->output_handler = output_handler;
	ntask->get_memory_pointer = ntask_get_memory_pointer;
	ntask->get_nos_mask = ntask_get_nos_mask;
	ntask->set_affinity = ntask_set_affinity;
	ntask->set_priority = ntask_set_priority;
	ntask->set_memory = ntask_set_memory;
	ntask->free_memory = ntask_free_memory;
	ntask->set_data_input = ntask_set_data_input;
	ntask->unset_data_input = ntask_unset_data_input;
	ntask->get_data_output = ntask_get_data_output;
	ntask->set_file_input = ntask_set_file_input;
	ntask->unset_file_input = ntask_unset_file_input;
	ntask->run = ntask_run;
	ntask->exit = ntask_exit;
	ntask->load = ntask_load;
	ntask->unload = ntask_unload;
	ntask->set_load_file = ntask_set_load_file;
	ntask->set_partition = ntask_set_partition;
	ntask->disable_data_push = ntask_disable_data_push;
	ntask->disable_data_pull = ntask_disable_data_pull;
	ntask->next_nt = ntask_next_nt;

	// ntask & daemon server message queue channels are created
	ntask->msgq_inf = msgq_inf_create();
	ntask->sock_inf = sock_inf_create();

	MsgClientPredict *msg_predict = &ntask->msg_predict;
	msg_predict->mtype = CL_PREDICT_S1;

 	msg_predict->mqid = ntask->msgq_inf->msgq_client;
	msg_predict->nnid = ntask->nnid;
	msg_predict->input_type = ntask->input_type;
	msg_predict->infile_size = ntask->infile_size;
	msg_predict->indata_start_offset = ntask->indata_start_offset;
	msg_predict->indata_size = ntask->indata_size;
	msg_predict->indata_push = ntask->indata_push;
	msg_predict->outdata_pull = ntask->outdata_pull;
	msg_predict->part_num_start = ntask->part_num_start;
	msg_predict->part_num_delta = ntask->part_num_delta;
	msg_predict->priority = ntask->priority; // default = 10, LOAD(250), UNLOAD(200)
	msg_predict->affinity_mask = ntask->affinity_mask;
	msg_predict->port = ntask->port;
	msg_predict->part_num_start = ntask->part_num_start;
	msg_predict->part_num_delta = ntask->part_num_delta;

	// workspace memory attached
	ntask->memory_pointer = malloc(NTASK_WSMEM_SIZE);        
	if (ntask->memory_pointer==NULL)
	{
		printf("Error : memory allocation failed\n");
		exit(1);
	}
	ntask->memory_size = NTASK_WSMEM_SIZE; // 64MBytes
	
	return ntask;
}

Ntask *ntask_partition_create(char *name, int nnid, int part_num, void (*input_handler)(Ntask *nt), void (*output_handler)(Ntask *nt))
{
	// partition valid range check
	if (part_num >= BIT_MAX_NUM)
	{
		printf("Error : partition number cannot exceed %d\n", BIT_MAX_NUM-1);
		exit(1);
	}

	Ntask *ntask;

	ntask = ntask_create(name, nnid, input_handler, output_handler);
	ntask_set_partition(ntask, part_num, part_num);

	return ntask;
}

Ntask *ntask_partition_range_create(char *name, int nnid, int part_num_start, int part_num_end, void (*input_handler)(Ntask *nt), void (*output_handler)(Ntask *nt))
{
	// partition valid range check
	if (part_num_start >= BIT_MAX_NUM || part_num_end >= BIT_MAX_NUM)
	{
		printf("Error : partition number cannot exceed %d\n", BIT_MAX_NUM-1);
		exit(1);
	}
	else if (part_num_start > part_num_end)
	{
		printf("Error : partition number start cannot be greater than partition number end\n");
		exit(1);
	}

	Ntask *ntask;

	ntask = ntask_create(name, nnid, input_handler, output_handler);
	ntask_set_partition(ntask, part_num_start, part_num_end);

	return ntask;
}

/* Ntask API for deamon */
void daemon_kill(void)
{
	MsgQInf *msgq_inf = msgq_inf_create();

	// get server message queue
	//printf("mqid_server = %d\n", msgq_inf->msgq_server);

	MsgClientDmkill msg_dmkill;

	msg_dmkill.mtype = CL_DMKILL; // must be > 0
        msgq_inf->send_msg(msgq_inf->msgq_server, &msg_dmkill);

	msgq_inf_destroy(msgq_inf);
}

