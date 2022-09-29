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
#include "ntask.h"

int main(int argc, char *argv[])
{
	unsigned int nos_mask;

	if (argc == 1)
	{
		nos_mask = ~0;
	}
	else if (argc == 2)
	{
		nos_mask = (1 << atoi(argv[1]));
	}
	else if (argc == 3)
	{
		if (!strcmp(argv[1], "-mask"))
		{
			nos_mask = atoi(argv[2]);
		}
		else
		{
			printf("Usage:\n");
			printf("dmkill : kill all NOSes\n");
			printf("dmkill <nos_id> : kill a specific NOS with <nos_id>\n");
			printf("dmkill -mask <nos_mask> : kill a set of NOSes corresponding <nos_mask>\n");
			exit(1);
		}
		
	}
	else
	{
			printf("Usage:\n");
			printf("dmkill : kill all NOSes\n");
			printf("dmkill <nos_id> : kill a specific NOS with <nos_id>\n");
			printf("dmkill -mask <nos_mask> : kill a set of NOSes corresponding <nos_mask>\n");
			exit(1);
	}

	daemon_kill(nos_mask);
}
