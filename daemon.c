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
#include <signal.h>
#include "share_comm.h"
#include "server_comm.h"

static Server *server;

static void sig_handler(int signo)
{
        fprintf(stderr, "[Server]: I received SIGINT(%d)\n", SIGINT);

	server->close(server); // includes npu_close();

        fprintf(stderr, "[Server]: The program exited abnormally (exit 1)\n");
        exit(1); // enforce exit the program
}

int main(int argc, char *argv[])
{
	/* Signal Handler Registration to address CTRL+C, abort, and segmentation fault */
	signal(SIGINT, (void *)sig_handler);
	signal(SIGABRT, (void *)sig_handler);
	signal(SIGSEGV, (void *)sig_handler);

	server = server_create();

	server->set_scheduler(server, server->scheduler->mwct);
	//server->set_scheduler(server, server->scheduler->mnt);
		
	server->open(server);

	/* Server Listen */
	server->handler(server);

	/* Server releases all resources to exit */
	server->close(server);
}
