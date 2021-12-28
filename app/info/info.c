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
#include "info.h"

char *baseos_name[3] =
{
        "non_os",
        "linux",
        "rtos"
};

int main(int argc, char *argv[])
{
	Info *info = info_create();

	printf("num npuos = %d\n", info->get_nest_info(info));
	printf("nos id = %d queryid = %d NOS version = %.1f\n", 0, NOS_VERSION, (float)info->get_nos_info(info, 0, NOS_VERSION)/10);
	printf("nos id = %d queryid = %d NOS base os = %s\n", 0, NOS_BASEOS, baseos_name[info->get_nos_info(info, 0, NOS_BASEOS)]);
}
