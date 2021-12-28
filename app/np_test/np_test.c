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
#include "nprocess.h"
#include "config.h"

int main(int argc, char *argv[])
{
	Nprocess *np = nprocess_create("np");

	Ntask *nt1 = ntask_create("nt1", 1, NULL, NULL);
	Ntask *nt2 = ntask_create("nt2", 1, NULL, NULL);
	Ntask *nt3 = ntask_create("nt3", 1, NULL, NULL);
	Ntask *nt4 = ntask_create("nt4", 1, NULL, NULL);
	Ntask *nt5 = ntask_create("nt5", 1, NULL, NULL);

	// draw graph
	nt1->next_nt(nt1, nt3);
	nt2->next_nt(nt2, nt3);
	nt2->next_nt(nt2, nt4);
	nt3->next_nt(nt3, nt5);
	nt4->next_nt(nt4, nt5);

	// contain node
	np->contain(np, 5, nt1, nt2, nt3, nt4, nt5);

	np->run(np);
}
