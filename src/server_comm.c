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
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "error.h"
#include "server_comm.h"
#include "share_comm.h"
#include "scheduler.h"
#include "server_npuos.h"
#include "msgq_inf.h"
#include "sock_inf.h"

#define NEST_DBG	0

// Message Queue for Service Thread
static volatile int server_done = 0;
static MsgQInf *msgq_inf_server;
static SockInf *sock_inf_server;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void server_msgq_create(void)
{
	/* Message Queue for Server : Fixed */
	msgq_inf_server = msgq_inf_create();
}

static void server_sock_create(void)
{
	sock_inf_server = sock_inf_create();

	memset(&sock_inf_server->server_addr, 0, sizeof(sock_inf_server->server_addr));
	memset(&sock_inf_server->os_addr, 0, sizeof(sock_inf_server->os_addr));

	sock_inf_server->server_addr.sin_family = AF_INET;
	sock_inf_server->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_inf_server->server_addr.sin_port = htons(SERVER_PORT);

	// server socket
	if ((sock_inf_server->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) //
	{
		fprintf(stderr, "Server : socket creation failed\n");
		exit(1);
	}

	// bind
	if (bind(sock_inf_server->sockfd, (struct sockaddr *)&sock_inf_server->server_addr, sizeof(sock_inf_server->server_addr)) < 0)
	{
		fprintf(stderr, "Server : socket bind failed\n");
		exit(1);
	}
}

// socket communication handler
void *sockcomm_handler(void *args)
{
	Server *server = (Server *)args;
	SockMessage msg;
	printf("[NEST]: Server SOCK daemon is ready!\n");

	while (!server_done)
	{
		sock_inf_server->recv_msg(sock_inf_server->sockfd, &msg, (struct sockaddr *) &sock_inf_server->os_addr, &sock_inf_server->os_addr_len);

		switch (msg.mtype)
		{
			case OS_CONN :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: os_conn signal received!\n");
#endif
				SockMsgOSConnect *msg_connect = (SockMsgOSConnect *)&msg;
				ServerNpuOS *nos;
				SockMsgClientEcho echo_msg;
				int osid;

				if (msg_connect->id == -1)
				{
					osid = server->scheduler->npuos_num;
				}
				else if (msg_connect->id > -1)
				{
					osid = msg_connect->id;
				}
				else
				{
					printf("OSID Error: osid is less than -1");
					exit(1);
				}


				if (server->scheduler->npuos_num >= MAX_NOS_NUM)
				{
					printf("Error: maximum # of NPU OS is %d. So connection refused!\n", MAX_NOS_NUM);
					echo_msg.mtype = ECHO_ERR;
				}
				else if (!server->scheduler->nos[osid])
				{
					// create ServerNpuOS info
					nos = server_npuos_create();
					nos->id = osid;
					nos->wsmem_size = msg_connect->wsmem_size;
					nos->state = msg_connect->state;
					nos->os_addr = sock_inf_server->os_addr;

					inet_ntop(AF_INET, &sock_inf_server->os_addr.sin_addr.s_addr, nos->ip, sizeof(nos->ip));

					server->scheduler->nos[osid] = nos;

					server->scheduler->npuos_num++;
					printf("[NEST]: %d NPU OS (ID: %d, IP: %s) is running\n", server->scheduler->npuos_num, osid, nos->ip);
					echo_msg.mtype = ECHO_OK;
					echo_msg.osid = osid;
				}
				else
				{
					printf("Error: NPU OS ID %d is already running\n", msg_connect->id);
					echo_msg.mtype = ECHO_ERR;
				}
				sock_inf_server->send_msg(sock_inf_server->sockfd, &echo_msg, sizeof(echo_msg), (struct sockaddr *) &sock_inf_server->os_addr);
			}
				break;

			case OS_DISCONN :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: os_disconn signal received!\n");
#endif
				SockMsgOSDisconnect *msg_disconnect = (SockMsgOSDisconnect *)&msg;

				SockMsgClientEcho echo_msg;
				if (server->scheduler->nos[msg_disconnect->id])
				{
					ServerNpuOS *nos = server->scheduler->nos[msg_disconnect->id];

					// free ServerNpuOS info
					server_npuos_free(nos);

					server->scheduler->npuos_num--;

					server->scheduler->nos[msg_disconnect->id] = NULL;
					printf("%d NPU OS is running\n", server->scheduler->npuos_num);
					echo_msg.mtype = ECHO_OK;
				}
				else
				{
					printf("Error: NPU OS ID %d is absent\n", msg_disconnect->id);
					echo_msg.mtype = ECHO_ERR;
				}

				msgq_inf_server->send_msg(msg_disconnect->msgq, &echo_msg);
			}
				break;

			case OS_LOAD_ECHO :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: os_load_echo signal received!\n");
#endif
				SockMsgOSEcho *msg_echo = (SockMsgOSEcho *) &msg;
				MsgClientEcho echo_cli;
				if (msg_echo->rtype == ECHO_OK)
				{ 
					int nnid = msg_echo->nnid;
					int osid = msg_echo->osid;
					int part_num_start = msg_echo->part_num_start;
					int part_num_delta = msg_echo->part_num_delta;

					NnPos *nnpos = server->scheduler->nnpos[nnid];
					nnpos->set_nos_bit(nnpos, osid, part_num_start, part_num_delta);
				}
				echo_cli.mtype = msg_echo->rtype;
				msgq_inf_server->send_msg(msg_echo->mqid, &echo_cli);
			}
				break;

			case OS_UNLOAD_ECHO :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: os_unload_echo signal received!\n");
#endif
				SockMsgOSEcho *msg_echo = (SockMsgOSEcho *) &msg;
				MsgClientEcho echo_cli;
				if (msg_echo->rtype == ECHO_OK)
				{ 
					int nnid = msg_echo->nnid;
					int osid = msg_echo->osid;
					int part_num_start = msg_echo->part_num_start;
					int part_num_delta = msg_echo->part_num_delta;

					NnPos *nnpos = server->scheduler->nnpos[nnid];
					nnpos->clr_nos_bit(nnpos, osid, part_num_start, part_num_delta);
				}
				echo_cli.mtype = msg_echo->rtype;
				msgq_inf_server->send_msg(msg_echo->mqid, &echo_cli);
			}
				break;

			case OS_PREDICT_ECHO :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: os_predict_echo signal received!\n");
#endif
				SockMsgOSPredictEcho *msg_echo = (SockMsgOSPredictEcho *) &msg;

				if (server->scheduler->function == server->scheduler->mwct)
				{
					// update the wcet[nnid(part_num_start, part_num_delta)] in the server_npuos
					ServerNpuOS *nos = server->scheduler->nos[msg_echo->osid];
					nos->update_wcet(nos, msg_echo->nnid, msg_echo->part_num_start, msg_echo->part_num_delta, msg_echo->curr_wcet);

					if (msg_echo->prev_wcet > 0)
					{
						nos->ewt -= msg_echo->prev_wcet;
					}
				}
				else if (server->scheduler->function == server->scheduler->mnt)
				{
					pthread_mutex_lock(&mutex);
					server->scheduler->nos[msg_echo->osid]->ntasks--; // one task is finished in NpuOS osid
					pthread_mutex_unlock(&mutex);
				}

				MsgClientEcho echo_msg;
				echo_msg.mtype = msg_echo->rtype;
				msgq_inf_server->send_msg(msg_echo->mqid, &echo_msg);
			}
				break;

			case OS_NOS_INFO_ECHO :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: os_nos_info_echo signal received!\n");
#endif
				SockMsgOSNosInfoEcho *msg_echo = (SockMsgOSNosInfoEcho *) &msg;
				MsgClientNosInfoEcho echo_msg;
				echo_msg.mtype = msg_echo->rtype;
				echo_msg.query_reply = msg_echo->query_reply;
				msgq_inf_server->send_msg(msg_echo->mqid, &echo_msg);
			}
				break;

			default :
				break;
		}
	}

	close(sock_inf_server->sockfd);

	return NULL;
}

// To Do : port number distributor must be more elaborated later!
static int client_portnum = DATA_SERVER_PORT;

static int client_tcp_portnum_get(void)
{
	client_portnum++;
	if (client_portnum > DATA_SERVER_PORT_MAX)
	{
		client_portnum = DATA_SERVER_PORT;
	}

	return client_portnum;
}

// server communication receiver only channel
static void server_handler(Server *server)
{
	Message msg;

	printf("[NEST]: Server IPC daemon is ready!\n");

	pthread_t sockcomm_thread;

	if (pthread_create(&sockcomm_thread, 0, sockcomm_handler, server))
	{
		fprintf(stderr, "sockcomm_thread creation failed\n");
		exit(1);
	}

	// IPC message handling
        while (!server_done)
        {
		msgq_inf_server->recv_msg(msgq_inf_server->msgq_server, &msg);
	
		switch (msg.mtype)
		{
			// inference client handler
			case CL_PREDICT_S1 :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: predict1 signal received!\n");
#endif

				MsgClientPredict *msg_predict = (MsgClientPredict *) &msg;

				// determine the NPU OS
				Scheduler *scheduler = server->scheduler;

				int osid = scheduler->function(scheduler, msg_predict->nnid, msg_predict->part_num_start, msg_predict->part_num_delta, msg_predict->affinity_mask);
							
#if (NEST_DBG==1)
				printf("[NEST_DBG]: scheduled osid = %d\n", osid);
#endif


				// send echo to the client
				MsgClientEcho echo_msg;

				if (osid == -1) // package is not loaded
				{
#if (NEST_DBG==1)
					printf("ERROR : Scheduler can't find any NOS for the request (Probably package is not loaded)\n");
#endif
					echo_msg.mtype = ECHO_ERR;
					msgq_inf_server->send_msg(msg_predict->mqid, &echo_msg);
				}
				else
				{
#if (NEST_DBG==1)
					if (!server->scheduler->nos[osid]) // this must not happen!
					{
						printf("PREDICT Error: NpuOS does not exist\n");
						exit(1);
					}
#endif

					// send echo to the client
					MsgClientEcho echo_msg;
					echo_msg.mtype = ECHO_OK;
					//echo_msg.inf_type = server->scheduler->nos[osid]->inf_type;
					echo_msg.osid = osid;
			
					msgq_inf_server->send_msg(msg_predict->mqid, &echo_msg);
				}
			}
				break;

			case CL_PREDICT_S2 :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: predict2 signal received!\n");
#endif

				MsgClientPredict *msg_predict = (MsgClientPredict *) &msg;

				int osid = msg_predict->osid;

				// if it is selected, then queue it

				SockMsgServerPredict msg_server_predict;
				msg_server_predict.mtype = CL_PREDICT;
				msg_server_predict.mqid = msg_predict->mqid;
				msg_server_predict.nnid = msg_predict->nnid;
				msg_server_predict.input_type = msg_predict->input_type;
				msg_server_predict.infile_size = msg_predict->infile_size;
				msg_server_predict.indata_start_offset = msg_predict->indata_start_offset;
				msg_server_predict.indata_size = msg_predict->indata_size;
				msg_server_predict.indata_push = msg_predict->indata_push;
				msg_server_predict.outdata_pull = msg_predict->outdata_pull;
				msg_server_predict.part_num_start = msg_predict->part_num_start;
				msg_server_predict.part_num_delta = msg_predict->part_num_delta;
				msg_server_predict.priority = msg_predict->priority;
				msg_server_predict.port = msg_predict->port;

				if (server->scheduler->function == server->scheduler->mwct) // based on expected completion time
				{
					ServerNpuOS *nos = server->scheduler->nos[osid];
					double wcet = nos->get_wcet(nos, msg_predict->nnid, msg_predict->part_num_start, msg_predict->part_num_delta);
					nos->ewt += wcet;
					msg_server_predict.prev_wcet = wcet;
				}
				else if (server->scheduler->function == server->scheduler->mnt) // based on # of tasks
				{
					pthread_mutex_lock(&mutex);
					server->scheduler->nos[osid]->ntasks++;
					pthread_mutex_unlock(&mutex);
				}

				sock_inf_server->send_msg(sock_inf_server->sockfd, &msg_server_predict, sizeof(msg_server_predict), (struct sockaddr *) &server->scheduler->nos[osid]->os_addr);
			}
				break;

			case CL_GET_NOS_MASK :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: get_nos_mask signal received!\n");
#endif

				MsgClientPredict *msg_predict = (MsgClientPredict *) &msg;

				// determine the NPU OS
				NnPos *nnpos = server->scheduler->nnpos[msg_predict->nnid];

				// send echo to the client
				MsgClientEcho echo_msg;

				if (!nnpos) // package for the nnid is not loaded
				{
					echo_msg.mtype = ECHO_ERR;
					msgq_inf_server->send_msg(msg_predict->mqid, &echo_msg);
				}
				else
				{
					unsigned int mask = nnpos->mask_nos_bit_match(nnpos, msg_predict->part_num_start, msg_predict->part_num_delta);

					// send echo to the client
					MsgClientEcho echo_msg;
					echo_msg.mtype = ECHO_OK;
					echo_msg.mask = mask;
			
					msgq_inf_server->send_msg(msg_predict->mqid, &echo_msg);
				}
			}
				break;

			// loader client handler
			case CL_LOAD :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: load signal received!\n");
#endif
				MsgClientLoad *msg_load = (MsgClientLoad *) &msg;
				int osid = msg_load->osid;
				int nnid = msg_load->nnid;
				MsgClientEcho echo_msg;
				NnPos *nnpos = server->scheduler->nnpos[nnid];
				int part_num_start = msg_load->part_num_start;
				int part_num_delta = msg_load->part_num_delta;

				if (!server->scheduler->nos[osid])
				{ 
					printf("LOAD Error: NpuOS does not exist\n");

					echo_msg.mtype = ECHO_ERR;
					msgq_inf_server->send_msg(msg_load->mqid, &echo_msg);
					break;
				}
				else if (nnpos->is_nos_bit(nnpos, osid, part_num_start, part_num_delta)) // check load is already made
				{
					printf("LOAD Error: kernel already exist, so cannot install\n");

					echo_msg.mtype = ECHO_ERR;
					msgq_inf_server->send_msg(msg_load->mqid, &echo_msg);
					break;
				}

				echo_msg.mtype = ECHO_OK;
				msgq_inf_server->send_msg(msg_load->mqid, &echo_msg);
				SockMsgServerLoad msg_server_load = {CL_LOAD, msg_load->mqid, msg_load->nnid, msg_load->filesize, msg_load->port, msg_load->part_num_start, msg_load->part_num_delta, NULL};
				sock_inf_server->send_msg(sock_inf_server->sockfd, &msg_server_load, sizeof(msg_server_load), (struct sockaddr *) &server->scheduler->nos[osid]->os_addr);

			}
				break;

			case CL_UNLOAD :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: unload signal received!\n");
#endif
				MsgClientUnload *msg_unload = (MsgClientUnload *) &msg;
				int osid = msg_unload->osid;
				int nnid = msg_unload->nnid;
				NnPos *nnpos = server->scheduler->nnpos[nnid];
				int part_num_start = msg_unload->part_num_start;
				int part_num_delta = msg_unload->part_num_delta;

				if (!server->scheduler->nos[osid])
				{ 
					printf("UNLOAD Error: NpuOS does not exist\n");

					MsgClientEcho echo_msg;
					echo_msg.mtype = ECHO_ERR;
					msgq_inf_server->send_msg(msg_unload->mqid, &echo_msg);
					break;
				}
				else if (!nnpos->is_nos_bit_match(nnpos, osid, part_num_start, part_num_delta)) // check load is already made
				{
					printf("UNLOAD Error: kernel was not installed yet\n");

					MsgClientEcho echo_msg;
					echo_msg.mtype = ECHO_ERR;
					msgq_inf_server->send_msg(msg_unload->mqid, &echo_msg);
					break;
				}

				SockMsgServerUnload msg_server_unload = {CL_UNLOAD, msg_unload->mqid, msg_unload->nnid, msg_unload->filesize, msg_unload->part_num_start, msg_unload->part_num_delta, NULL};
				sock_inf_server->send_msg(sock_inf_server->sockfd, &msg_server_unload, sizeof(msg_server_unload), (struct sockaddr *) &server->scheduler->nos[osid]->os_addr);
			}
				break;

			// dmkill client handler
			case CL_DMKILL :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: dmkill signal received!\n");
#endif

				server_done = 1;
			}
				break;

			case CL_PORT :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: port signal received!\n");
#endif
				MsgClientPort *msg_port = (MsgClientPort *) &msg;
				
				MsgClientPortEcho echo_msg;
				echo_msg.mtype = ECHO_OK;
				echo_msg.port = client_tcp_portnum_get();

				// send echo message to the client
				msgq_inf_server->send_msg(msg_port->mqid, &echo_msg);
			}
				break;

			case CL_INFO :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: info signal received!\n");
#endif
				MsgClientNestInfo *msg_info = (MsgClientNestInfo *) &msg;
				
				MsgClientNestInfoEcho echo_msg1;
				echo_msg1.mtype = ECHO_OK;
				echo_msg1.num_npuos = server->scheduler->npuos_num;

				// send echo message to the client
				msgq_inf_server->send_msg(msg_info->mqid, &echo_msg1);

				for (int i=0; i<server->scheduler->npuos_num; i++)
				{
					MsgClientNestConfigEcho echo_msg2;
                                        echo_msg2.mtype = ECHO_OK;
					echo_msg2.osid = i;
					echo_msg2.wsmem_size = server->scheduler->nos[i]->wsmem_size;
					strcpy(echo_msg2.ip, server->scheduler->nos[i]->ip);
					
					msgq_inf_server->send_msg(msg_info->mqid, &echo_msg2);
				}
			}
				break;

			case CL_NOS_INFO :
			{
#if (NEST_DBG==1)
				printf("[NEST_DBG]: nos info signal received!\n");
#endif
				MsgClientNosInfo *msg_info = (MsgClientNosInfo *) &msg;
				int osid = msg_info->osid;
				int queryid = msg_info->queryid;

				if (!server->scheduler->npuos_num) // No NPUOS exists, exit
				{
					MsgClientNosInfoEcho echo_msg;
					echo_msg.mtype = ECHO_ERR;
					msgq_inf_server->send_msg(msg_info->mqid, &echo_msg);
					break;
				}

				SockMsgServerNosInfo msg_server_nos_info = {CL_NOS_INFO, msg_info->mqid, queryid, NULL};
				sock_inf_server->send_msg(sock_inf_server->sockfd, &msg_server_nos_info, sizeof(msg_server_nos_info), (struct sockaddr *) &server->scheduler->nos[osid]->os_addr);
			}
				break;

			default :
				break;
		}
        }

	pthread_cancel(sockcomm_thread);
}

static void server_open(Server *server)
{	
	// server message queue is created
	server_msgq_create();
	server_sock_create();
}

static void server_close(Server *server)
{
#if (NEST_DBG==1)
	printf("join all the service thread list\n");
#endif
	for (int i=0; i<server->scheduler->npuos_num; i++)
	{
		MsgServerDmkill msg_server_dmkill = {CL_DMKILL, NULL};

		sock_inf_server->send_msg(sock_inf_server->sockfd, &msg_server_dmkill, sizeof(msg_server_dmkill), (struct sockaddr *) &server->scheduler->nos[i]->os_addr);
	}

	// kill all npu_os()
	for (int i=0; i<server->scheduler->npuos_num; i++)
	{
		server_npuos_free(server->scheduler->nos[i]);
	}

	// remove the server message queue
	//msgctl(mqid_server, IPC_RMID, 0);
	msgctl(msgq_inf_server->msgq_server, IPC_RMID, 0);
}

static void server_set_scheduler(Server *server, int (scheduler_function)(Scheduler *sched, int nnid, int part_num_start, int part_num_delta, unsigned int affinity_mask))
{
	server->scheduler->function = scheduler_function;
	if (scheduler_function == server->scheduler->mwct)
	{
		printf("MWCT (Minimum Worst-case Completion Time) Scheduler is Running\n");
	}
	else if (scheduler_function == server->scheduler->mnt)
	{
		printf("MNT (Minimum Number of Tasks) Scheduler is Running\n");
	}
	else
	{
		printf("No Scheduler is Running\n");
	}
}

Server *server_create(void)
{
	Server *server = malloc(sizeof(Server));
	if (!server)
	{
		printf("Error: server cannot be created\n");
		exit(1);
	}

	server->scheduler = scheduler_create();

	server->npuos_num = &server->scheduler->npuos_num;
	for (int i=0; i<MAX_NOS_NUM; i++)
	{
		server->nos[i] = server->scheduler->nos[i];
	}

	server->open = server_open;
	server->handler = server_handler;
	server->close = server_close;
	server->set_scheduler = server_set_scheduler;

	return server;
}
