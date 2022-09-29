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
#include <sys/msg.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "share_comm.h"
#include "error.h"
#include "msgq_inf.h"
#include "sock_inf.h"

#define NEST_DBG	0

static void msgq_send_msg(int mqid, void *msg)
{
        int status;

        status = msgsnd(mqid, msg, sizeof(Message)-sizeof(long), 0);

        if (status == -1)
        {
                perror("Send fail");
                exit(1);
        }
        else
        {
                //printf("Msg Sent (msgq)\n");
        }
}

static void msgq_recv_msg(int mqid, Message *msg)
{
        int status;

        status =  msgrcv(mqid, msg, sizeof(Message), 0, 0);
        if (status == -1)
        {
                perror("Receive fail");
                exit(1);
        }
        else
        {
                //printf("Msg Received (msgq)\n"); 
        }
}

MsgQInf *msgq_inf_create(void)
{
	MsgQInf *msgq_inf;

	msgq_inf = (MsgQInf *)malloc(sizeof(MsgQInf));
	if (!msgq_inf)
	{
		printf("Error: msgq_inf cannot be created\n");
		exit(1);
	}

	msgq_inf->msgq_client = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
	msgq_inf->msgq_server = msgget((key_t) SERVER_MSGQ_KEY, IPC_CREAT | 0666);
	msgq_inf->send_msg = msgq_send_msg;
	msgq_inf->recv_msg = msgq_recv_msg;

	return msgq_inf;
}

void msgq_inf_destroy(MsgQInf *msgq_inf)
{
	free(msgq_inf);
}

#if 0
static void sock_send_udp_msg(int sockfd, void *msg, size_t msg_len, const struct sockaddr *dest_addr)
{
	int n;

#if (NEST_DBG==1)
        printf("[NEST_DBG]: sock send msg starts\n");
#endif
	n = sendto(sockfd, (const void *) msg, msg_len, MSG_CONFIRM, dest_addr, sizeof(*dest_addr));

	if (n == -1)
	{
                perror("Send fail");
		exit(1);
	}
	else
	{
#if (NEST_DBG==1)
        printf("[NEST_DBG]: sock recv msg finished\n");
#endif
                //printf("Msg Sent (sock)\n");
	}
}

static void sock_recv_udp_msg(int sockfd, SockMessage *msg, struct sockaddr *src_addr, socklen_t *src_len)
{
	int n;

#if (NEST_DBG==1)
        printf("[NEST_DBG]: sock recv msg starts\n");
#endif
	n = recvfrom(sockfd, msg, MESSAGE_SIZE, MSG_WAITALL, (struct sockaddr *) src_addr, src_len);	
	if (n == -1)
	{
                perror("Receive fail");
		exit(1);
	}
	else
	{
#if (NEST_DBG==1)
        printf("[NEST_DBG]: sock recv msg finished\n");
#endif
                //printf("Msg Received (sock)\n");
	}
}
#endif

#define MAX_UDP_PAYLOAD 1024

static void sock_send_data(int sockfd, void *data, size_t data_len)
{
	int repeat = data_len / MAX_UDP_PAYLOAD;
	size_t remain_len = data_len - repeat * MAX_UDP_PAYLOAD;
	int n;
	void *p = data;
	int ndata = 0;

	for (int i=0; i<repeat; i++)
	{
		n = write(sockfd, (const void *) p, (size_t) MAX_UDP_PAYLOAD);
		if (n == -1)
		{
			perror("Error : write data");
			exit(1);
		}
		ndata += n;

		p += MAX_UDP_PAYLOAD;
	}
	n = write(sockfd, (const void *) p, remain_len);
	if (n == -1)
	{
		perror("Error : write data last");
		exit(1);
	}
	ndata += n;

#if (NEST_DBG==1)
	printf("[NEST_DBG]: sock send data (%d bytes) finished\n", ndata);
#endif
} 

static void sock_recv_data(int sockfd, void *data, size_t data_len)
{
	void *p = data;
	char buf[MAX_UDP_PAYLOAD] = {0, };
	size_t ndata = data_len;

	while (ndata > 0)
	{
		int payload_size = ndata;
		if (ndata >= MAX_UDP_PAYLOAD) 
			payload_size = MAX_UDP_PAYLOAD; 

		ssize_t n = read(sockfd, buf, payload_size);
		if (n == -1)
		{
			perror("Error : read data\n");
			exit(1);
		}
		ndata -= n;

		memcpy(p, (const void *)buf, n);
		p += n; 
	}

#if (NEST_DBG==1)
	printf("[NEST_DBG]: sock recv data (%zu bytes) finished\n", data_len);
#endif
}

static void sock_send_filename(int sockfd, char *file_name)
{
	// send filename before sending filedata
	int n = send(sockfd, file_name, 100, 0); // the max length of file name is 100
	if (n==-1)
	{
                perror("Send filename fail");
		exit(1);
	}
	else
	{
		printf("Sent Filename %s\n", file_name);
	}
}

static void sock_send_file(int sockfd, int filefd, int file_size)
{
	// send filename before sending filedata
	ssize_t rv = sendfile(sockfd, filefd, NULL, file_size);
	if (rv == -1)
	{
                perror("Send file fail");
		exit(1);
	}
	else
	{
		printf("Sent File (%d bytes)\n", file_size);
	}
} 

static void sock_recv_filename(int sockfd, char *name_buf)
{
	int n = recv(sockfd, name_buf, 100, 0);
	if (n==-1)
	{
                perror("Receive filename fail");
		exit(1);
	}
	else
	{
		printf("Received Filename %s\n", name_buf);
	}
}

static void sock_recv_file(int sockfd, int filefd, int file_size)
{
	char buf[MAX_UDP_PAYLOAD] = {0, };

        ssize_t ndata = file_size;
        while (ndata > 0)
	{
		int c = 0;
		int payload_size = ndata;
		if (ndata >= MAX_UDP_PAYLOAD) 
			payload_size = MAX_UDP_PAYLOAD; 
        	ssize_t n = recv(sockfd, buf, payload_size, 0);
		if (n == -1)
		{
			perror("receive error");
			exit(1);
		}
		ndata -= n;
                //printf("recvd bytes = %zu\n", n);

                // writes n bytes in buf to the file
                c = write(filefd, buf, n);
		if (c == -1)
		{
			perror("write error");
			exit(1);
		}
                //printf("wrote bytes = %d\n", c);
	}

	//printf("Received File (%zu bytes)\n", file_size);
#if (NEST_DBG==1)
	printf("[NEST_DBG]: Received File (%d bytes)\n", file_size);
#endif
}

static int sock_server_init(int *server_fd, int port)
{
	// create TCP server
	if ((*server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("TCP Server : can't open stream socket\n");
		exit(1);
	}

	//printf("server_fd : %d\n", *server_fd);

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//server_addr.sin_port = htons(DATA_SERVER_PORT);
	server_addr.sin_port = htons(port);

#if (NEST_DBG==1)
	printf("[NEST_DBG]: TCP Server: Port %d\n", port);
#endif
	int optval = 1;
	setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("TCP Server : Can't bind local address.\n");
		exit(1);
	}


	if (listen(*server_fd, 5) < 0)
	{
		perror("TCP Server : Can't listen for connection\n");
		exit(1);
	}

	return *server_fd;
}

static int sock_server_accept(int *server_fd, char *addr_str)
{
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	//char temp[20];
	int client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd < 0)
        {
         	perror("TCP Server: accept failed.\n");
            	exit(1);
        }

        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, addr_str, SOCKADDR_LEN);
#if (NEST_DBG==1)
        printf("[NEST]: TCP Server: %s client connected.\n", temp);
#endif
    
	return client_fd;
}

static int sock_client_connect(char *dest_addr, int port)
{
	int sock_fd;
	struct sockaddr_in server_addr;
	char temp[20];

	if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{ 
		perror("TCP Client : socket error"); 
		exit(1);
	}
	
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr(dest_addr);
	server_addr.sin_port = htons(port);

        inet_ntop(AF_INET, &server_addr.sin_addr.s_addr, temp, sizeof(temp));
#if (NEST_DBG==1)
        printf("[NEST_DBG]: TCP Client: %s is the server address Port: %d\n", temp, port);
#endif

	if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("TCP Client : connect error"); 
		exit(1);
	}

	return sock_fd;
}

SockInf *sock_inf_create(void)
{
	SockInf *sock_inf;

	sock_inf = (SockInf *)malloc(sizeof(SockInf));
	if (!sock_inf)
	{
		printf("Error: sock_inf cannot be created\n");
		exit(1);
	}

	memset(&sock_inf->server_addr, 0, sizeof(sock_inf->server_addr));
	memset(&sock_inf->os_addr, 0, sizeof(sock_inf->os_addr));

	sock_inf->server_addr_len = sizeof(sock_inf->server_addr);
	sock_inf->os_addr_len = sizeof(sock_inf->os_addr);

	// UDP messages
	sock_inf->send_msg = sock_send_data;
	sock_inf->recv_msg = sock_recv_data;

	// TCP messages
	sock_inf->send_data = sock_send_data;
	sock_inf->recv_data = sock_recv_data;
	sock_inf->send_filename = sock_send_filename;
	sock_inf->send_file = sock_send_file;
	sock_inf->recv_filename = sock_recv_filename;
	sock_inf->recv_file = sock_recv_file;
	sock_inf->server_init = sock_server_init;
	sock_inf->server_accept = sock_server_accept;
	sock_inf->client_connect = sock_client_connect;
	

	return sock_inf;
}

void sock_inf_destroy(SockInf *sock_inf)
{
	free(sock_inf);
}
