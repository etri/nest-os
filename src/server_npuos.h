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
#ifndef SERVER_NPUOS_H
#define SERVER_NPUOS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "queue.h"

#define OS_DORMANT      (0x00)
#define OS_READY        (0x01)
#define OS_RUN          (0x02)
#define OS_SHUTDOWN     (0X04)

#define DEFAULT_MAXTIME	(600.0) // 600 secs = 10 minute

typedef struct server_npu_os
{
        int id; // os_index
	char ip[20]; // ipv4 address of npu os
        int state; // OS_RUN, OS_WAIT
        int ntasks; // number of tasks requested
	Queue *q[256]; // partition queue contains the worst case expected prediction time taken for each model
	double ewt; // (ewt): expected waiting time

	int wsmem_size;
	struct sockaddr_in os_addr;

	void (*update_wcet)(struct server_npu_os *nos, int nnid, int part_num_start, int part_num_delta, double new_wcet);
	double (*get_wcet)(struct server_npu_os *nos, int nnid, int part_num_start, int part_num_delta);
} ServerNpuOS;

ServerNpuOS *server_npuos_create(void);
void server_npuos_free(ServerNpuOS *nos);

#endif
