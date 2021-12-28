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
#include "ntask.h"
#include "nprocess.h"
#include "util_time.h"
#include "demo/image.h"
#include "config.h"

static char *image_path;

#define FILE_INPUT	1

/////////////////////////////////////////////////
static void ntask_input_handler(Ntask *nt)
{
#if (FILE_INPUT==1)
	nt->set_file_input(nt, image_path);
#else
#define TRANSFER_DATA_SIZE 346176
	// 1. prepare image and prediction
	Image im = load_image_color(image_path, 0, 0);

        // 2. image copy into shared memory region
	void *memp = nt->get_memory_pointer(nt);

        Image *im_in = (Image *) (memp+TRANSFER_DATA_SIZE);
        *im_in = im;
        im_in->data = (float *)(nt->get_memory_pointer(nt)+TRANSFER_DATA_SIZE+32);
        memcpy(im_in->data, im.data, im.h*im.w*im.c*sizeof(float));

        // 3. specify input and output data region in memory
        nt->set_data_input(nt, TRANSFER_DATA_SIZE, 32+im.h*im.w*im.c*sizeof(float));
#endif
}

static void ntask_output_handler(Ntask *nt)
{
	int output_offset, output_size;
	nt->get_data_output(nt, &output_offset, &output_size);
        //printf("output data : offset = %d size = %d\n", output_offset, output_size);

	// get the output data, and adjust the image data pointer if necessary
        Image *im_out = (Image *)(nt->get_memory_pointer(nt)+output_offset);
	im_out->data = (float *)(nt->get_memory_pointer(nt)+output_offset+32);
}
/////////////////////////////////////////////////

#define PARTITION_0 	0
#define PARTITION_1 	1

int main(int argc, char *argv[])
{
	image_path = argc==2? argv[1]:"data/dog.jpg";
	printf("argc = %d argv[1]=%s\n", argc, argv[1]);

	Nprocess *np = nprocess_create("np");

	// ntask create and do setting if necessary (affinity or priority)
	Ntask *nt1 = ntask_partition_create("nt1", MODEL_NAME, PARTITION_0, ntask_input_handler, OUTPUT_HANDLER_NONE);
	Ntask *nt2 = ntask_partition_create("nt2", MODEL_NAME, PARTITION_1, INPUT_HANDLER_NONE, ntask_output_handler);

	//np->add_ntask(np, nt1); // link: nt1->nt2
	//np->add_ntask(np, nt2); // link: nt1->nt2
	nt1->next_nt(nt1, nt2);

	np->contain(np, 2, nt1, nt2);

	//nt1->set_affinity(nt1, MASK(0));
	//nt2->set_affinity(nt2, MASK(1));
	//nt1->set_partition(nt1, 0, 0);
	//nt2->set_partition(nt2, 1, 1);
	//nt->set_priority(nt, 10);
	
	printf("> prediction starts\n");
	double time=what_time_is_it_now();

	// run input_handler+prediction is made
	np->run(np);

	printf("> prediction ends -> took %f seconds\n", what_time_is_it_now() - time);

	// save image or show image on window
#ifdef OPENCV
        make_window("predictions", 512, 512, 0);

	int output_offset, output_size;
	nt2->get_data_output(nt2, &output_offset, &output_size);

	Image *im_out = (Image *)(nt2->get_memory_pointer(nt2)+output_offset);
        show_image(*im_out, "predictions", 0);
#else
        save_image(*im_out, "predictions"); // save on file
#endif
	// ntask destroy
	np->exit(np);
}
