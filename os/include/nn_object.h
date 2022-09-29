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
#ifndef NN_OBJECT_H
#define NN_OBJECT_H

#include "bit_handler.h"

#define INPUT_DATA_BIT  (0x01)
#define INPUT_FILE_BIT  (0x02)

typedef struct nn_object_net_info
{
	int part_num_start;
	int part_num_delta;
} NetInfo;

typedef struct nn_object_input
{
	int type;
	int data_offset;
	int data_size;	
	char *filename;
} Input;

typedef struct nn_object_output
{
	int data_offset_client; // source offset at wsmem in client
	int data_offset; // source offset at wsmem in nos
	int data_size;	
} Output;

typedef struct nn_object
{
	int nnid; // neural net id, YOLOv2, YOLOv2_tiny, ...
	void *wsmem; // memory pointer of workspace memory
	int loaded; // nn_object loaded(1) or not(0)?
	void *npu; // attached npu pointer by npuos
	char *load_filename_buf;
	//unsigned int part_bit[4];
	BitString bs;

	void (*load)(struct nn_object *object, NetInfo *info);
	void (*unload)(struct nn_object *object, NetInfo *info);
	void (*predict)(struct nn_object *object, NetInfo *info);
	void (*set_input)(struct nn_object *object, NetInfo *info, Input *input);
	void (*get_output)(struct nn_object *object, NetInfo *info, Output *output);
} NnObject;

typedef struct nn_object_list
{
        int nnid;
        NnObject *object;
} NnObjectList;

NnObject *nn_object_create(void);
void nn_object_free(NnObject *nn_object);

#define NN_OBJECT_INITIALIZER(NNID) {NNID, 0, 0, 0, 0, BIT_STRING_INITIALIZER, load, unload, predict, set_input, get_output}

#endif
