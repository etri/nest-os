#ifndef NN_OBJECT_LIST_H
#define NN_OBJECT_LIST_H
#include "nn_object.h"
#include "support_model.h"

#define NN_OBJECT_NUM 	2

extern NnObject nn_object_2;
extern NnObject nn_object_24;

static NnObjectList obj_list[NN_OBJECT_NUM] = 
{
	{RESNET18_NPU, &nn_object_2},
	{RESNET18_NPU_PART, &nn_object_24},
};
#endif
