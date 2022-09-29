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
#include "loader.h"
#include "error.h"
#include "msgq_inf.h"
#include "sock_inf.h"
#include "bit_handler.h"
//#include "util_time.h"

static void sock_portnum_init(Loader *loader)
{
        if (!loader->port)
        {
                MsgClientPort msg_port = {CL_PORT, loader->msgq_inf->msgq_client};

                loader->msgq_inf->send_msg(loader->msgq_inf->msgq_server, &msg_port);

                Message rmsg;
                loader->msgq_inf->recv_msg(loader->msgq_inf->msgq_client, &rmsg); // from server
                if (rmsg.mtype == ECHO_OK)
                {
                        MsgClientPortEcho *rmsg_port = (MsgClientPortEcho *)&rmsg;

                        loader->port = rmsg_port->port;
                }
                else
                {
                        printf("Error : Port number cannot be obtained\n");
                        exit(1);
                }
        }
}

static int loader_load(Loader *loader, int nnid, int osid)
{
	MsgClientLoad msg_load;
	msg_load.mtype = CL_LOAD;
	
	msg_load.mqid = loader->msgq_inf->msgq_client;
	msg_load.nnid = nnid;
	msg_load.osid = osid;
	msg_load.filesize = loader->file_size;
	msg_load.part_num_start = loader->part_num_start;
	msg_load.part_num_delta = loader->part_num_delta;

	// open loader port
	int server_fd = -1;

	if (loader->file_size)
	{
		sock_portnum_init(loader);

		//sock_server_init(&server_fd, msg_load.port);
		loader->sock_inf->server_init(&server_fd, loader->port);
	}

	// precheck if load can be made. if kernel is already loaded, load cannot be done
//double start = what_time_is_it_now();
	msg_load.port = loader->port; 
        loader->msgq_inf->send_msg(loader->msgq_inf->msgq_server, &msg_load);

	// receive3 ack for precheck load (Server->app)
	Message rmsg;
	loader->msgq_inf->recv_msg(loader->msgq_inf->msgq_client, &rmsg);
//double elapsed = what_time_is_it_now() - start;
//printf("ipc round trip time = %f\n", elapsed);

	// load is progressing
	if (rmsg.mtype == ECHO_ERR)
	{
		printf("Error msg received. nnid%d(%d-%d) on osid%d load failed\n", nnid, loader->part_num_start, loader->part_num_start + loader->part_num_delta, osid);
		if (loader->file_size) // in case of file load
		{
			close(server_fd);
		}
		return -1;
	}
        else if (loader->file_size) // in case of file load
        {
		char client_addr[SOCKADDR_LEN];
		int loader_fd = loader->sock_inf->server_accept(&server_fd, client_addr);

               	loader->sock_inf->send_filename(loader_fd, loader->file_name);
               	loader->sock_inf->send_file(loader_fd, loader->fd, loader->file_size);
        }

	// receive ack for this load (OS->app)
	loader->msgq_inf->recv_msg(loader->msgq_inf->msgq_client, &rmsg);
	if (rmsg.mtype == ECHO_ERR)
	{
		printf("Error msg received. nnid%d on osid%d load failed\n", nnid, osid);
		if (loader->file_size)
		{
			close(server_fd);
		}

		return -1;
	}

	return 0;
}

static int loader_unload(Loader *loader, int nnid, int osid)
{
	MsgClientUnload msg_unload;
	msg_unload.mtype = CL_UNLOAD;
	
	msg_unload.mqid = loader->msgq_inf->msgq_client;
	msg_unload.nnid = nnid;
	msg_unload.osid = osid;
	msg_unload.filesize = loader->file_size;
	msg_unload.part_num_start = loader->part_num_start;
	msg_unload.part_num_delta = loader->part_num_delta;

        loader->msgq_inf->send_msg(loader->msgq_inf->msgq_server, &msg_unload);

	Message rmsg;
	loader->msgq_inf->recv_msg(loader->msgq_inf->msgq_client, &rmsg);
	if (rmsg.mtype == ECHO_ERR)
	{
		printf("Error msg received. nnid%d on osid%d load failed\n", nnid, osid);
		return -1;
	}
	return 0;
}

static void loader_set_file(Loader *loader, char *filepath)
{
	loader->file_name = basename(filepath);

	loader->fd = open(filepath, O_RDONLY);

	if (loader->fd != -1)
        {
                struct stat obj;
                stat(filepath, &obj);
                loader->file_size = obj.st_size;
        }
        else
        {
                printf("File %s is missing in the directory!\n", loader->file_name);
                exit(1);
        }
}

static void loader_set_partition(Loader *loader, int part_num_start, int part_num_end)
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

        loader->part_num_start = part_num_start;
        loader->part_num_delta = part_num_end - part_num_start;
}

static int loader_load_partition(Loader *loader, int nnid, int part_num, int osid)
{
	 // partition valid range check
        if (part_num >= BIT_MAX_NUM)
        {
                printf("Error : partition number cannot exceed %d\n", BIT_MAX_NUM-1);
                exit(1);
        }

	loader_set_partition(loader, part_num, part_num);
	return (loader_load(loader, nnid, osid));
}

static int loader_load_partition_range(Loader *loader, int nnid, int part_num_start, int part_num_end, int osid)
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

	loader_set_partition(loader, part_num_start, part_num_end);
	return(loader_load(loader, nnid, osid));
}

static int loader_unload_partition(Loader *loader, int nnid, int part_num, int osid)
{
	 // partition valid range check
        if (part_num >= BIT_MAX_NUM)
        {
                printf("Error : partition number cannot exceed %d\n", BIT_MAX_NUM-1);
                exit(1);
        }

	loader_set_partition(loader, part_num, part_num);
	return (loader_unload(loader, nnid, osid));
}

static int loader_unload_partition_range(Loader *loader, int nnid, int part_num_start, int part_num_end, int osid)
{
	loader_set_partition(loader, part_num_start, part_num_end);
	return(loader_unload(loader, nnid, osid));
}

Loader *loader_create(void)
{
	Loader *loader;

	loader = malloc(sizeof(Loader));

	if (!loader)
	{
		printf("Error: loader object cannot be created\n");
		exit(1);
	}

	loader->fd = 0;
	loader->file_name = NULL;
	loader->file_size = 0;
	loader->port = 0;
	loader->part_num_start = 0;
	loader->part_num_delta = 0;

	loader->msgq_inf = msgq_inf_create();
	loader->sock_inf = sock_inf_create();

	loader->load = loader_load;
	loader->unload = loader_unload;
	loader->set_file = loader_set_file;
	loader->set_partition = loader_set_partition;
	loader->load_partition = loader_load_partition;
	loader->load_partition_range = loader_load_partition_range;
	loader->unload_partition = loader_unload_partition;
	loader->unload_partition_range = loader_unload_partition_range;

	return loader;
}
