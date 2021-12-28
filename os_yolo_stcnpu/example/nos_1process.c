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
#include <sys/shm.h>
#include "npuos.h"
#include "npu.h"

int main(int argc, char *argv[])
{
#if (STCNPU_INCLUDED==1)
	printf("stcnpu opened!\n");
	stcnpu_open(); // for STC NPU
#endif

	NpuOS *nos;

	nos = npuos_create(NULL); // use default memory as wsmem

        //nos->set_id(nos, os_id); // os id = 0
        nos->set_hwtype(nos, ETRI_STC);  // ETRI_STC, LG_DQ1, XILINX_PYNQ_Z1
#if (STCNPU_INCLUDED==1)
        //nos->set_npu(nos, &stc0);  
        nos->set_npu(nos, &stc1); 
#else
        nos->set_npu(nos, NULL);  // if NULL, npu is not attached, that is cpu
#endif
	//nos->set_serverip(nos, "129.254.74.241");
	//nos->set_serverip(nos, "129.254.74.87");
	//nos->set_serverip(nos, "192.168.0.4");

	/* Start OS */
	nos->start(nos); // start the os (including init)

#if (STCNPU_INCLUDED==1)
	printf("stcnpu closed!\n");
	stcnpu_close(); // for STC NPU
#endif

	return 0;
}
