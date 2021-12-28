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
#ifndef NN_OBJECT_LIST_H
#define NN_OBJECT_LIST_H
#include "nn_object.h"
#include "support_model.h"

#define NN_OBJECT_NUM 	5

extern NnObject nn_object_1;
//extern NnObject nn_object_10;
//extern NnObject nn_object_11;
extern NnObject nn_object_129;
extern NnObject nn_object_130;
extern NnObject nn_object_131;
extern NnObject nn_object_132;

typedef struct nn_object_list
{
	int nnid;
	NnObject *object;
} NnObjectList;

static NnObjectList obj_list[NN_OBJECT_NUM] = 
{
	{YOLOV2_NPU, &nn_object_1},
       // {DARKNET_YOLOV3_TINY, &nn_object_10},
       // {DARKNET_YOLOV3, &nn_object_11},
	{YOLOV2_CPU, &nn_object_129},
	{YOLOV2_TINY_CPU, &nn_object_130},
	{YOLOV2_TINY_CPU1, &nn_object_131},
	{YOLOV2_TINY_CPU2, &nn_object_132}
};
#endif
