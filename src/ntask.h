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
#ifndef NTASK_H
#define NTASK_H

#include <pthread.h>
#include "support_model.h"
#include "msgq_inf.h"
#include "sock_inf.h"
#include "loader.h"

// affinity
#define MASK(n)       (1<<n)
#define MAX_NAME_LENGTH		20
#define INPUT_HANDLER_NONE	NULL
#define OUTPUT_HANDLER_NONE	NULL

#define MAX_PRED_NUM    10
#define MAX_SUCC_NUM    10

typedef struct ntask
{
	char name[MAX_NAME_LENGTH];
	pthread_mutex_t lock;
	int nnid; // neural net model id
	int affinity_mask;
	int priority;
	int input_type;
	int fd; // file description if input is file
	char *infile_name;
	int infile_size;
	void *memory_pointer;	
	int memory_size;
	int indata_start_offset;
	int indata_size;
	int outdata_start_offset;
	int outdata_size;
	int indata_push;
	int outdata_pull;
	int port;
	int part_num_start; // partition number
	int part_num_delta;

	void *np; // the process that contains this ntask
	volatile pthread_t tid;
	int num_pred;
        int num_succ;
        int num_vpred;
        int num_vsucc;
        struct ntask *pred[MAX_PRED_NUM];
        struct ntask *succ[MAX_SUCC_NUM];

	struct ntask *next;

	// communication 
	MsgClientPredict msg_predict; // message
	MsgQInf 	*msgq_inf; // message interface (MSGQ)
	SockInf		*sock_inf; // socket interface

	Loader *loader;

	void (*input_handler)(struct ntask *nt);
	void (*output_handler)(struct ntask *nt);
	void *(*get_memory_pointer)(struct ntask *nt);
	void (*set_data_input)(struct ntask *nt, int indata_start_offset, int indata_size);
	void (*unset_data_input)(struct ntask *nt);
	void (*get_data_output)(struct ntask *nt, int *outdata_start_offset, int *outdata_size);
	void (*set_file_input)(struct ntask *nt, char *filename);
	void (*unset_file_input)(struct ntask *nt);
	unsigned int (*get_nos_mask)(struct ntask *nt);
        void (*set_affinity)(struct ntask *nt, unsigned int mask);
	void (*set_priority)(struct ntask *nt, int priority);
	void (*set_memory)(struct ntask *nt, void *memptr);
	void (*free_memory)(struct ntask *nt);
   	void (*run)(struct ntask *nt);
        void (*exit)(struct ntask *nt);
	int  (*load)(struct ntask *nt, int osid);
	int  (*unload)(struct ntask *nt, int osid);
	void (*set_load_file)(struct ntask *nt, char *filepath);
	void (*set_partition)(struct ntask *nt, int part_num_start, int part_num_end);
	void (*disable_data_push)(struct ntask *nt);
	void (*disable_data_pull)(struct ntask *nt);
	void (*next_nt)(struct ntask *nt, struct ntask *nt_next);
} Ntask;

Ntask *ntask_create(char *name, int nnid, void (*input_handler)(Ntask *nt), void (*output_handler)(Ntask *nt));
Ntask *ntask_partition_create(char *name, int nnid, int part_num, void (*input_handler)(Ntask *nt), void (*output_handler)(Ntask *nt));
Ntask *ntask_partition_range_create(char *name, int nnid, int part_num_start, int part_num_end, void (*input_handler)(Ntask *nt), void (*output_handler)(Ntask *nt));

void daemon_kill(void);

#endif
