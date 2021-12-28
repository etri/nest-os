#ifndef HWTYPE_H
#define HWTYPE_H

#define ETRI_STC	1
#define LG_DQ1		2
#define XILINX_PYNQ_Z1	3
#define XILINX_ULTRA96	4
#define XILINX_ZCU102	5

// more information can be added in the data structure 
typedef struct etri_stc
{
	int npuid;
} EtriStc;

typedef struct lg_dq1
{
	int npuid;
} LgDQ1;

typedef struct xilinx_pynq_z1
{
	int npuid;
} XilinxPynqZ1;

typedef struct xilinx_ultra96
{
        int npuid;
} XilinxUltra96;

typedef struct xilinx_zcu102
{
        int npuid;
} XilinxZcu102;

#endif
