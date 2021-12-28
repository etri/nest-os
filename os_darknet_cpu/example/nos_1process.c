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

int main(int argc, char *argv[])
{
	NpuOS *nos;

	nos = npuos_create(NULL);

	/* OS, HW setting */
        //nos->set_id(nos, 0); // os id = 0
        //nos->set_hwtype(nos, ETRI_STC);  // ETRI_STC, LG_DQ1, XILINX_PYNQ_Z1
        //nos->set_inf_type(nos, INF_SOCK); // INF_IPC or INF_SOCK
        nos->set_npu(nos, NULL);  // if NULL, npu is not attached, that is cpu
        //nos->set_npu(nos, &stc0);  // if NULL, npu is not attached, that is cpu

	/* Start OS */
	nos->start(nos); // start the os (including init)

	return 0;
}
