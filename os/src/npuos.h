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
#ifndef NPUOS_H
#define NPUOS_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "os_sched.h"
#include "nn_object.h"
#include "sock_inf.h"

#define OS_DORMANT	(0x00)
#define OS_READY	(0x01)
#define OS_RUN		(0x02)
#define OS_SHUTDOWN	(0X04)

#define ACC_CPU	 (-1)
#define ACC_NPU0 (0)
#define ACC_NPU1 (1)

////////////// NOS BASE OS /////////////////
#define VERSION 	10 // 1.0
#define BASEOS		LINUX

#define MAX_SHM_PAGE	8

typedef struct npu_os
{
        int id; // os_index
	int hwtype; // ETRI_STC, LG_DQ1, XILINX_PYNQ_Z1
	int version; // 1.0
	int baseos;

	int inf_type; // ipc or socket
	SockInf *sock_inf_os;
	char *server_ip;

	int state; // OS_RUN, OS_WAIT
	int ntasks; // number of tasks requested 
	int shutdown;
	OsSched *sched;
	unsigned int support[8]; // 8*4bytes = 32bits*8 = 256bits
	NnObject *nn_objects[256];
	void *npu; // attach a specific NPU
	void *wsmem;
	int wsmem_size;

	void (*init)(struct npu_os *nos);
	void (*object_register)(struct npu_os *nos);
	void (*connect)(struct npu_os *nos);
	void (*set_id)(struct npu_os *nos, int id);
	void (*set_hwtype)(struct npu_os *nos, int type);
	void (*set_npu)(struct npu_os *nos, void *npu);
	void (*set_serverip)(struct npu_os *nos, char *ipaddr);
	void (*start)(struct npu_os *nos);
} NpuOS;

NpuOS *npuos_create(void *wsmem);
void npuos_destroy(NpuOS *nos);

NpuOS *npuos_self(void);
#endif
