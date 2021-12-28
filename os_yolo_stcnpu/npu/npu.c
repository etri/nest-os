#include <stdio.h>
#include <stdlib.h>
#include "hwtype.h"

EtriStc stc0 = {0};
EtriStc stc1 = {1};

#if (STCNPU_INCLUDED==1)
int npu_module(void);
void npu_open(void);
void npu_close(void);

static int npu_num_module;
#endif

void stcnpu_open(void)
{
#if (STCNPU_INCLUDED==1)
        npu_num_module = npu_module();

        if (npu_num_module)
        {
                npu_open(); // global variable stc_index = 0, 1, ...
        }
        else
        {
                printf("NPU is not available\n");
                exit(1);
        }
#endif
}

void stcnpu_close(void)
{
#if (STCNPU_INCLUDED==1)
        if (npu_num_module)
        {
                npu_close(); // global variable stc_index = 0, 1, ...
        }
#endif
}
