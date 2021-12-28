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
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "share_comm.h"
#include "info.h"
#include "error.h"
#include "msgq_inf.h"
#include "sock_inf.h"

int info_get_nest_info(Info *info)
{
	MsgClientNestInfo msg_info;
	msg_info.mtype = CL_INFO;
	
	if (!info)
	{
		printf("Error: info object is null\n");
		return -1;
	}

	msg_info.mqid = info->msgq_inf->msgq_client;

        info->msgq_inf->send_msg(info->msgq_inf->msgq_server, &msg_info);

	// receive3 ack for precheck load (Server->app)
	Message rmsg;
	info->msgq_inf->recv_msg(info->msgq_inf->msgq_client, &rmsg);

	// load is progressing
	if (rmsg.mtype == ECHO_ERR)
	{
		printf("Error msg received. info get failed\n");
		return -1;
	}
        else
        {
		MsgClientNestInfoEcho *info_rmsg1 = (MsgClientNestInfoEcho *)&rmsg;
		int num_npuos = info_rmsg1->num_npuos;
		printf("num npuos = %d\n", num_npuos);

		for (int i=0; i<num_npuos; i++)
		{
			info->msgq_inf->recv_msg(info->msgq_inf->msgq_client, &rmsg);
			MsgClientNestConfigEcho *info_rmsg2 = (MsgClientNestConfigEcho *)&rmsg;
			printf("OSID: %d \t Workspace Memory: %d (Bytes) IP: %s\n", info_rmsg2->osid, info_rmsg2->wsmem_size, info_rmsg2->ip);
		}

		return num_npuos;
        }
}

int info_get_nos_info(Info *info, int osid, int queryid)
{
	MsgClientNosInfo msg_info;
	msg_info.mtype = CL_NOS_INFO;
	
	msg_info.mqid = info->msgq_inf->msgq_client;
	msg_info.osid = osid;
	msg_info.queryid = queryid;

        info->msgq_inf->send_msg(info->msgq_inf->msgq_server, &msg_info);

	Message rmsg;
	info->msgq_inf->recv_msg(info->msgq_inf->msgq_client, &rmsg);
	if (rmsg.mtype == ECHO_ERR)
	{
		printf("Error msg received. queryid%d on osid%d get info failed\n", queryid, osid);
		return -1;
	}
	else
	{
		MsgClientNosInfoEcho *info_rmsg = (MsgClientNosInfoEcho *)&rmsg;
		return (info_rmsg->query_reply);
	}
}

Info *info_create(void)
{
	Info *info;

	info = malloc(sizeof(Info));
	if (!info)
	{
		printf("Error: info object cannot be created\n");
		exit(1);
	}

	info->msgq_inf = msgq_inf_create();

	info->get_nest_info = info_get_nest_info;
	info->get_nos_info = info_get_nos_info;

	return info;
}
