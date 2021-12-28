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
#include "support_model.h"
#include "util_time.h"

#define NEST_DBG	0

void super_task(void *value); // in npuos.c

static int is_preload_check(NnObject *nn_object, int part_num_start, int part_num_delta)
{
	if (nn_object->loaded)
	{
		BitString bs;
		convert_to_bitstring(part_num_start, part_num_start+part_num_delta, &bs);
		if (partblock_is_partbit_set(&nn_object->bs, &bs))
                         return 1;
	}

	return 0;
}

static void load_task(void *value)
{
	SockMsgServerLoad *msg_load = (SockMsgServerLoad *) value;
	NpuOS *npuos = (NpuOS *) msg_load->npuos;
	SockInf *sock_inf_os = npuos->sock_inf_os;
	SockMsgOSEcho echo_msg;
	echo_msg.mtype = OS_LOAD_ECHO;

	NnObject *nn_object = npuos->nn_objects[msg_load->nnid];

	if (!nn_object) // load failed
	{
		printf("Load Failed\n");
		echo_msg.rtype = ECHO_ERR;
		echo_msg.mqid = msg_load->mqid;
	}
	else
	{
		//if (!nn_object->loaded) // if not loaded yet
		if (!is_preload_check(nn_object, msg_load->part_num_start, msg_load->part_num_delta))
		{
			if (msg_load->filesize)
			{
				/* Load File */
				printf("Load File Here\n");

				// attach the shared memory for prediction, where the address of input and output image is located
				char *server_ip;

				if (!npuos->server_ip)
					server_ip = DEFAULT_SERVER_IP;
				else
					server_ip = npuos->server_ip;

				int server_fd = sock_inf_os->client_connect(server_ip, msg_load->port); // ip & port

				if (!nn_object->load_filename_buf)
				{
					nn_object->load_filename_buf = malloc(sizeof(char)*100);
					if (!nn_object->load_filename_buf)
					{
						printf("Error: load_filename_buf cannot be created\n");
						exit(1);
					}
				}

				char *filename = nn_object->load_filename_buf;
				sock_inf_os->recv_filename(server_fd, filename);

				int file_fd;
				file_fd = open(filename, O_CREAT | O_WRONLY, 0666);
				if (file_fd == -1)
				{
					printf("File Open Error!\n");
					exit(1);
				}
				sock_inf_os->recv_file(server_fd, file_fd, msg_load->filesize);
				close(file_fd);
			}

			NetInfo info;
			info.part_num_start = msg_load->part_num_start;
			info.part_num_delta = msg_load->part_num_delta;

			nn_object->load(nn_object, &info);
			nn_object->loaded = 1;
	
			BitString bs;
			convert_to_bitstring(info.part_num_start, info.part_num_start+info.part_num_delta, &bs);
			partblock_set_bit(&nn_object->bs, &bs);

			printf("NNID %d (PID: %d-%d) is loaded\n", nn_object->nnid, info.part_num_start, info.part_num_start+info.part_num_delta);

			echo_msg.part_num_start = info.part_num_start;
			echo_msg.part_num_delta = info.part_num_delta;

			/* workspace memory is ready */
			nn_object->wsmem = npuos->wsmem;
			echo_msg.rtype = ECHO_OK;
		}
		else
		{
			printf("NNID %d(%d-%d) is NOT loaded\n", nn_object->nnid, msg_load->part_num_start, msg_load->part_num_start+msg_load->part_num_delta);
			echo_msg.rtype = ECHO_ERR;
		}

		/* send the message to the client */
		echo_msg.mqid = msg_load->mqid;
		echo_msg.nnid = msg_load->nnid;
		echo_msg.osid = npuos->id;
	}

	sock_inf_os->send_msg(sock_inf_os->sockfd, &echo_msg, sizeof(echo_msg), (struct sockaddr *) &sock_inf_os->server_addr);

	//free(msg_load);
}

static void unload_task(void *value)
{
	SockMsgServerUnload *msg_unload = (SockMsgServerUnload *) value;
	NpuOS *npuos = (NpuOS *) msg_unload->npuos;
	SockInf *sock_inf_os = npuos->sock_inf_os;
	SockMsgOSEcho echo_msg;
	echo_msg.mtype = OS_UNLOAD_ECHO;

	if (!npuos->nn_objects[msg_unload->nnid]) // install Not found
	{
		echo_msg.rtype = ECHO_ERR;
		echo_msg.mqid = msg_unload->mqid;
		sock_inf_os->send_msg(sock_inf_os->sockfd, &echo_msg, sizeof(echo_msg), (struct sockaddr *) &sock_inf_os->server_addr);
		free(msg_unload);
		return;
	}
	else // install found
	{
		NnObject *nn_object;

		nn_object = npuos->nn_objects[msg_unload->nnid];

		/* attention : extra private memory free for sock comm */
		if (nn_object->loaded)
		{
			NetInfo info;
			info.part_num_start = msg_unload->part_num_start;
			info.part_num_delta = msg_unload->part_num_delta;

			nn_object->unload(nn_object, &info);
			printf("NNID %d (PID: %d-%d) is unloaded\n", nn_object->nnid, info.part_num_start, info.part_num_start+info.part_num_delta);

			BitString bs;
			convert_to_bitstring(info.part_num_start, info.part_num_start+info.part_num_delta, &bs);
			partblock_clr_bit(&nn_object->bs, &bs);

			if (partblock_is_allbit_zero(&nn_object->bs))
			{
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
			echo_msg.part_num_start = info.part_num_start;
			echo_msg.part_num_delta = info.part_num_delta;
			echo_msg.rtype = ECHO_OK;
		}
		else
		{
			printf("object is not loaded yet\n");
			echo_msg.rtype = ECHO_ERR;
		}

		/* send the message to the client */
		echo_msg.mqid = msg_unload->mqid;
		echo_msg.nnid = msg_unload->nnid;
		echo_msg.osid = npuos->id;
	}

	sock_inf_os->send_msg(sock_inf_os->sockfd, &echo_msg, sizeof(echo_msg), (struct sockaddr *) &sock_inf_os->server_addr);
	
	//free(msg_unload);
}

static void predict_task(void *value)
{
	SockMsgServerPredict *msg_predict = (SockMsgServerPredict *)value;
	NpuOS *npuos = (NpuOS *) msg_predict->npuos;
	SockInf *sock_inf_os = npuos->sock_inf_os;

	NnObject *nn_object = npuos->nn_objects[msg_predict->nnid];

	if (!nn_object) // not install NN in this OS
	{
		printf("Error: NN %d is not supported!\n", msg_predict->nnid);

		SockMsgOSEcho msg_echo = {OS_PREDICT_ECHO, msg_predict->mqid, msg_predict->nnid, npuos->id, ECHO_ERR};
		sock_inf_os->send_msg(sock_inf_os->sockfd, &msg_echo, sizeof(msg_echo), (struct sockaddr *) &sock_inf_os->server_addr);
	}
	else
	{

////////////////////////////// RECV FILE OR/AND DATA ////////////////////////
		// attach the shared memory for prediction, where the address of input and output image is located
		char *server_ip;

		if (!npuos->server_ip)
			server_ip = DEFAULT_SERVER_IP;
		else
			server_ip = npuos->server_ip;

		int server_fd = sock_inf_os->client_connect(server_ip, msg_predict->port); // ip & port

		Input input;
		input.filename = NULL;

if (msg_predict->indata_push)
{
	//printf("nos:indata_push\n");
		if (msg_predict->input_type & INPUT_FILE_BIT) // means "input is FILE"
		{
			if (!input.filename)
			{
				input.filename = malloc(sizeof(char)*100);
				if (!input.filename)
				{
					printf("Error: input.filename cannot be created\n");
					exit(1);
				}
			}
			char *filename = input.filename;
			sock_inf_os->recv_filename(server_fd, filename);

			int file_fd;
			file_fd = open(filename, O_CREAT | O_WRONLY, 0666);
			if (file_fd == -1)
			{
				printf("File Open Error!\n");
				exit(1);
			}
			sock_inf_os->recv_file(server_fd, file_fd, msg_predict->infile_size);
			close(file_fd);
		}

		if (msg_predict->input_type & INPUT_DATA_BIT) // means "input is FILE"
		{
			//printf("open touch shmptr:%p\n", nn_object->shmptr);
			sock_inf_os->recv_data(server_fd, nn_object->wsmem + msg_predict->indata_start_offset, msg_predict->indata_size);
		}
		else
		{
			msg_predict->indata_start_offset = -1; // means FILE
		}
}
////////////////////////////// RECV DATA/FILE END ////////////////////////////

		NetInfo info;
		info.part_num_start = msg_predict->part_num_start;
		info.part_num_delta = msg_predict->part_num_delta;

		input.type = msg_predict->input_type;
		input.data_offset = msg_predict->indata_start_offset;
		input.data_size = msg_predict->indata_size;

		nn_object->set_input(nn_object, &info, &input);

		double start_time = what_time_is_it_now();
		// prediction is made
		nn_object->predict(nn_object, &info);

		Output output;

		nn_object->get_output(nn_object, &info, &output);

		double time_duration = what_time_is_it_now() - start_time;

////////////////////////////// SEND DATA ///////////////////////
		SockMsgOSOutdataEcho echo_msg = {OS_OUTDATA_ECHO, msg_predict->nnid, npuos->id, output.data_offset, output.data_size};
	
		send(server_fd, &echo_msg, sizeof(echo_msg), MSG_CONFIRM);
if (msg_predict->outdata_pull)
{
	//printf("nos:outdata_pull\n");
		sock_inf_os->send_data(server_fd, nn_object->wsmem + output.data_offset, output.data_size);
}
////////////////////////////// SEND DATA END ///////////////////

		// notify server the task is finished
		SockMsgOSPredictEcho msg_echo = {OS_PREDICT_ECHO, msg_predict->mqid, msg_predict->nnid, msg_predict->part_num_start, msg_predict->part_num_delta, npuos->id, msg_predict->prev_wcet, time_duration, ECHO_OK};
		sock_inf_os->send_msg(sock_inf_os->sockfd, &msg_echo, sizeof(msg_echo), (struct sockaddr *) &sock_inf_os->server_addr);

		close(server_fd);

		if (msg_predict->input_type & INPUT_FILE_BIT) // input is file, remove it locally
		{
			// delete the input file
			char *filename = input.filename;
			remove(filename);
				//printf("Remove Input File : %s\n", filename);
			free(filename);
			input.filename = NULL;
		}
	}

	//free(msg_predict);
}

static void nos_info_task(void *value)
{
	SockMsgServerNosInfo *msg_nos_info = (SockMsgServerNosInfo *)value;
	NpuOS *npuos = (NpuOS *) msg_nos_info->npuos;
	SockInf *sock_inf_os = npuos->sock_inf_os;

	SockMsgOSNosInfoEcho echo_msg;
	echo_msg.mtype = OS_NOS_INFO_ECHO;

	/* send the message to the client */
	echo_msg.rtype = ECHO_OK;
	echo_msg.mqid = msg_nos_info->mqid;
	echo_msg.osid = npuos->id;

	switch (msg_nos_info->queryid)
	{
		case NOS_VERSION :
			echo_msg.query_reply = npuos->version;
			break;
		case NOS_BASEOS :
			echo_msg.query_reply = npuos->baseos;
			break;
		default:
			echo_msg.query_reply = 0;
			break;
	}

	sock_inf_os->send_msg(sock_inf_os->sockfd, &echo_msg, sizeof(echo_msg), (struct sockaddr *) &sock_inf_os->server_addr);

	//free(msg_nos_info);
}

void *sock_npuos_handler(void *args)
{
	NpuOS *npuos = (NpuOS *)args;
	OsSched *sched = npuos->sched;
	volatile int handler_done = 0;
	SockInf *sock_inf_os = npuos->sock_inf_os;

	printf("[NOS]: socket handler %d is ready\n", npuos->id);

	while (!handler_done)
	{
		SockMessage msg;

		sock_inf_os->recv_msg(sock_inf_os->sockfd, &msg, (struct sockaddr *) &sock_inf_os->server_addr, &sock_inf_os->server_addr_len);
		
		switch (msg.mtype)
		{
			case CL_PREDICT :
			{
#if (NEST_DBG==1)
				printf("sock npuos_handler %d : PREDICT\n", npuos->id);
#endif
				SockMsgServerPredict *msg_predict = (SockMsgServerPredict *)&msg;

				// message is copied to the msg member in task data structure
				Task *task = task_message_create(predict_task, &msg, msg_predict->priority);

				msg_predict = (SockMsgServerPredict *)&task->msg;
				msg_predict->npuos = npuos;

				task_submit(sched, task);

				npuos->ntasks++;
			}
				break;

			case CL_LOAD :
			{
#if (NEST_DBG==1)
				printf("sock npuos_handler %d : LOAD\n", npuos->id);
#endif
				//SockMsgServerLoad *msg_load = (SockMsgServerLoad *)&msg;

				Task *task = task_message_create(load_task, &msg, 250);

				SockMsgServerLoad *msg_load = (SockMsgServerLoad *)&task->msg;
				msg_load->npuos = npuos;

				task_submit(sched, task);

				npuos->ntasks++;

			}
				break;

			case CL_UNLOAD :
			{
#if (NEST_DBG==1)
				printf("sock npuos_handler %d : UNLOAD\n", npuos->id);
#endif
				//SockMsgServerUnload *msg_unload = (SockMsgServerUnload *)&msg;

				Task *task = task_message_create(unload_task, &msg, 200);

				SockMsgServerUnload *msg_unload = (SockMsgServerUnload *)&task->msg;
				msg_unload->npuos = npuos;

				task_submit(sched, task);

				npuos->ntasks++;
			}
				break;

			case CL_NOS_INFO :
			{
#if (NEST_DBG==1)
				printf("sock npuos_handler %d : NOS_INFO\n", npuos->id);
#endif
				//SockMsgServerNosInfo *msg_nos_info = (SockMsgServerNosInfo *)&msg;

				Task *task = task_message_create(nos_info_task, &msg, 50);

				SockMsgServerNosInfo *msg_nos_info = (SockMsgServerNosInfo *)&task->msg;
				msg_nos_info->npuos = npuos;

				task_submit(sched, task);

				npuos->ntasks++;
			}
				break;

			case CL_DMKILL :
			{
#if (NEST_DBG==1)
				printf("sock npuos_handler %d : DMKILL\n", npuos->id);
#endif
				//SockMsgServerDmkill *msg_dmkill = (SockMsgServerDmkill *)&msg;

				Task *task = task_message_create(super_task, &msg, 255);

				SockMsgServerDmkill *msg_dmkill = (SockMsgServerDmkill *)&task->msg;
				msg_dmkill->npuos = npuos;

				task_submit(sched, task);

				npuos->ntasks++;

				handler_done = 1;
			}
				break;

			default :
			{
#if (NEST_DBG==1)
				printf("Unknown message is arrived!! Please check the system!\n");
#endif
			}
				break;
		}
	}

#if (NEST_DBG==1)
	printf("OS handler %d exits!\n", npuos->id);
#endif

	return 0;
}

void npuos_sock_init(NpuOS *npuos)
{
        if (!npuos->server_ip)
        {
                npuos->server_ip = DEFAULT_SERVER_IP;
        }
        else
        {
                npuos->server_ip = npuos->server_ip;
        }

	SockInf *sock_inf_os = sock_inf_create();

        memset(&sock_inf_os->server_addr, 0, sizeof(sock_inf_os->server_addr));

        sock_inf_os->server_addr.sin_family = AF_INET;
	sock_inf_os->server_addr.sin_addr.s_addr = inet_addr(npuos->server_ip);
        sock_inf_os->server_addr.sin_port = htons(SERVER_PORT);

        // server socket (for UDP client)
        if ((sock_inf_os->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) //
        {
                fprintf(stderr, "Server : socket creation failed\n");
                exit(1);
        }

	npuos->sock_inf_os = sock_inf_os;
}

void npuos_sock_connect(NpuOS *npuos)
{
	SockInf *sock_inf_os = npuos->sock_inf_os;

        SockMsgOSConnect msg_connect = {OS_CONN, npuos->id, npuos->wsmem_size, npuos->state};
        sock_inf_os->send_msg(sock_inf_os->sockfd, &msg_connect, sizeof(msg_connect), (struct sockaddr *) &sock_inf_os->server_addr);

        SockMessage msg;
        socklen_t src_len = sizeof(sock_inf_os->server_addr);
        sock_inf_os->recv_msg(sock_inf_os->sockfd, &msg, (struct sockaddr *) &sock_inf_os->server_addr, &src_len);
        SockMsgClientEcho *echo_msg = (SockMsgClientEcho *)&msg;

        if (echo_msg->mtype!=ECHO_OK)
        {
                printf("os connect error !!\n");
        }
        else
        {
                npuos->id = echo_msg->osid;
                printf("[NOS]: NPU OS ID %d is connected to the server\n", npuos->id);
        }
}

void npuos_sock_close(NpuOS *npuos)
{
	close(npuos->sock_inf_os->sockfd);
}

