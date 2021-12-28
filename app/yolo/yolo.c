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
#include "util_time.h"
#include "demo/image.h"
#include "config.h" // model name

static char *image_path;

//static void input_handler(Ntask *nt, char *image_path)
static void input_handler(Ntask *nt)
{
	// 1. prepare image and prediction
	Image im = load_image_color(image_path, 0, 0);

	// 2. image copy into shared memory region
	Image *im_in = nt->get_memory_pointer(nt);
	*im_in = im;
	im_in->data = (float *)(nt->get_memory_pointer(nt)+32);
	memcpy(im_in->data, im.data, im.h*im.w*im.c*sizeof(float));

	// 3. specify input and output data region in memory
	nt->set_data_input(nt, 0, 32+im.h*im.w*im.c*sizeof(float));
}

Image *im_out;
static void output_handler(Ntask *nt)
{
	int output_offset, output_size;

	nt->get_data_output(nt, &output_offset, &output_size);
	//printf("output data : offset = %d size = %d\n", output_offset, output_size);

	// get the output data, and adjust the image data pointer if necessary
        im_out = (Image *)(nt->get_memory_pointer(nt)+output_offset);
	im_out->data = (float *)(nt->get_memory_pointer(nt)+output_offset+32);
}

int main(int argc, char *argv[])
{
	//char *image_path = argc==2? argv[1]:"data/dog.jpg";
	image_path = argc==2? argv[1]:"data/dog.jpg";
	//printf("argc = %d argv[1]=%s\n", argc, argv[1]);

	// ntask create and do setting if necessary (affinity or priority)
	Ntask *nt = ntask_create("nt", MODEL_NAME, input_handler, output_handler); 
	//nt->set_affinity(nt, MASK(0));
	//nt->set_priority(nt, 10);

///////// for UNIST, use get_nos_mask() API to get mask that contains installed NOSes for the model
	//unsigned int mask = nt->get_nos_mask(nt);
	//printf("mask = %x\n", mask);
	//nt->set_affinity(nt, mask);
	//nt->set_affinity(nt, MASK(1));
	
	
	printf("> prediction starts\n");
	double time=what_time_is_it_now();

	//input_handler(nt, image_path);
	// run input_handler+prediction is made
	nt->run(nt);
	//output_handler(nt);

	printf("> prediction ends -> took %f seconds\n", what_time_is_it_now() - time);
	// save image or show image on window
#ifdef OPENCV
        make_window("predictions", 512, 512, 0);
	//Image *im_out = (Image *)(nt->get_memory_pointer(nt)+output_offset);
        show_image(*im_out, "predictions", 0);
#else
        save_image(*im_out, "predictions"); // save on file
#endif

	// ntask destroy
	nt->exit(nt);
}
