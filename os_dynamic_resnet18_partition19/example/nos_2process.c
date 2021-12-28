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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "npuos.h"
#include "npu.h"

void *hw[2] = {&stc0, &stc1};

int main(int argc, char *argv[])
{
	int pid;

	if ((pid = fork()) < 0)
	{
		printf("fork error\n");
		exit(0);
	}
	else if (pid == 0) // child
	{
		NpuOS *nos;

		nos = npuos_create();

        	nos->set_id(nos, 0); // id = 0
        	nos->set_hwtype(nos, ETRI_STC);
        	nos->set_inf_type(nos, INF_SOCK);
		nos->set_npu(nos, hw[0]);

		nos->start(nos); // start the os (including init)
	}
	else // parent
	{
		NpuOS *nos;

		nos = npuos_create();

        	nos->set_id(nos, 1); // id = 1
        	nos->set_hwtype(nos, ETRI_STC);
        	nos->set_inf_type(nos, INF_SOCK);
		nos->set_npu(nos, hw[1]);

		nos->start(nos); // start the os (including init)
	}

	return 0;
}
