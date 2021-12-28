#ifndef NN_OBJECT_LIST_H
#define NN_OBJECT_LIST_H
#include "nn_object.h"
#include "support_model.h"

extern NnObject nn_object_23;

static NnObjectList obj_list[] =
{
        {RESNET18_NPU_DYNAMIC, &nn_object_23}
};
#endif

