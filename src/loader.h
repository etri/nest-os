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
#ifndef LOADER_H
#define LOADER_H

#include "msgq_inf.h"
#include "sock_inf.h"

typedef struct loader
{
	int fd;
	char *file_name;
	int file_size;
	int port;
	int part_num_start;
        int part_num_delta;

	MsgQInf	 *msgq_inf;
	SockInf  *sock_inf;

        int (*load)(struct loader *loader, int nnid, int osid);
        int (*unload)(struct loader *loader, int nnid, int osid);
	void (*set_file)(struct loader *loader, char *filepath);
	void (*set_partition)(struct loader *loader, int part_num_start, int part_num_end);
	int (*load_partition)(struct loader *loader, int nnid, int part_num, int osid);
	int (*load_partition_range)(struct loader *loader, int nnid, int part_num_start, int part_num_end, int osid);
	int (*unload_partition)(struct loader *loader, int nnid, int part_num, int osid);
	int (*unload_partition_range)(struct loader *loader, int nnid, int part_num_start, int part_num_end, int osid);
} Loader;

/* loader APIs */
Loader *loader_create(void);

#endif
