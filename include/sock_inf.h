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
#ifndef SOCK_INF_H
#define SOCK_INF_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "share_comm.h"

typedef struct sock_inf
{
	int sockfd;

	struct sockaddr_in server_addr;
	socklen_t server_addr_len;
	struct sockaddr_in os_addr;
	socklen_t os_addr_len;

	void (*send_msg)(int sockfd, void *msg, size_t msg_len, const struct sockaddr *dest_addr);
	void (*recv_msg)(int sockfd, SockMessage *msg, struct sockaddr *src_addr, socklen_t *src_len);

	void (*send_data)(int sockfd, void *data, size_t data_len);
	void (*recv_data)(int sockfd, void *data, size_t data_len);

	void (*send_file)(int sockfd, int filefd, int file_len);
	void (*recv_file)(int sockfd, int filefd, int file_len);
	void (*send_filename)(int sockfd, char *file_name);
	void (*recv_filename)(int sockfd, char *filename);
	int (*server_init)(int *server_fd, int port);
	int (*server_accept)(int *server_fd);
	int (*client_connect)(char *dest_addr, int port);
} SockInf;

SockInf *sock_inf_create(void);
void sock_inf_destroy(SockInf *sock_inf);

#endif
