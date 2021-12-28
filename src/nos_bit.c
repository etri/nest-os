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
#include "nos_bit.h"

static void nosbit_set_bit(NosBit *nosbit, BitString *bs)
{
	partblock_set_bit(&nosbit->bs, bs);
}

static void nosbit_clr_bit(NosBit *nosbit, BitString *bs)
{
	partblock_clr_bit(&nosbit->bs, bs);
}

static int nosbit_is_partbit_set(NosBit *nosbit, BitString *bs)
{
	return (partblock_is_partbit_set(&nosbit->bs, bs));
}

static int nosbit_is_allbit_set(NosBit *nosbit, BitString *bs)
{
	return (partblock_is_allbit_set(&nosbit->bs, bs));
}

static int nosbit_is_allbit_clr(NosBit *nosbit, BitString *bs)
{
	return (partblock_is_allbit_clr(&nosbit->bs, bs));
}

static int nosbit_is_allbit_zero(NosBit *nosbit)
{
	return (partblock_is_allbit_zero(&nosbit->bs));
}

NosBit *nosbit_create(void)
{
	NosBit *nosbit = malloc(sizeof(NosBit));

	if (!nosbit)
	{
		printf("Error: nosbit cannot be created\n");
		exit(1);
	}

	for (int i=0; i<BIT_STRING_NUM; i++)
		nosbit->bs.bit[i] = 0;

	nosbit->set_bit = nosbit_set_bit;
	nosbit->clr_bit = nosbit_clr_bit;
	nosbit->is_partbit_set = nosbit_is_partbit_set;
	nosbit->is_allbit_set = nosbit_is_allbit_set;
	nosbit->is_allbit_clr = nosbit_is_allbit_clr;
	nosbit->is_allbit_zero = nosbit_is_allbit_zero;

	return nosbit;
}

void nosbit_free(NosBit *nosbit)
{
	free(nosbit);
}

static void nnpos_set_nos_bit(NnPos *nnpos, int osid, int part_num_start, int part_num_delta)
{
	nnpos->os_bit |= (1 << osid);

	NosBit *nosbit = nnpos->nosbit[osid];

	if (!nosbit)
	{
		nosbit = nosbit_create();
		nnpos->nosbit[osid] = nosbit;
	}

	BitString bs = BIT_STRING_INITIALIZER;
	convert_to_bitstring(part_num_start, part_num_start+part_num_delta, &bs);
	nosbit->set_bit(nosbit, &bs);
}

static void nnpos_clr_nos_bit(NnPos *nnpos, int osid, int part_num_start, int part_num_delta)
{
	NosBit *nosbit = nnpos->nosbit[osid];

	if (!nosbit)
	{
		printf("Error: nosbit is empty\n");
		exit(1);
	}

	BitString bs = BIT_STRING_INITIALIZER;
	convert_to_bitstring(part_num_start, part_num_start+part_num_delta, &bs);
	nosbit->clr_bit(nosbit, &bs);

	if (nosbit->is_allbit_zero(nosbit))
	{
		nnpos->os_bit &= ~(1 << osid);
		nosbit_free(nnpos->nosbit[osid]);
		nnpos->nosbit[osid] = NULL;
	}
}

static int nnpos_is_nos_bit(NnPos *nnpos, int osid, int part_num_start, int part_num_delta)
{
	if (nnpos->os_bit & (1 << osid))
	{
		if (nnpos->nosbit[osid])
		{
			NosBit *nosbit = nnpos->nosbit[osid];

			BitString bs = BIT_STRING_INITIALIZER;
			convert_to_bitstring(part_num_start, part_num_start+part_num_delta, &bs);

			int r = nosbit->is_partbit_set(nosbit, &bs);

			return r;
		}	
	}

	return 0;
}

static int nnpos_is_nos_bit_match(NnPos *nnpos, int osid, int part_num_start, int part_num_delta)
{
	if (nnpos->os_bit & (1 << osid))
	{
		if (nnpos->nosbit[osid])
		{
			NosBit *nosbit = nnpos->nosbit[osid];

			BitString bs = BIT_STRING_INITIALIZER;
			convert_to_bitstring(part_num_start, part_num_start+part_num_delta, &bs);

			int r = nosbit->is_allbit_set(nosbit, &bs);

			return r;
		}	
	}

	return 0;
}

static unsigned int nnpos_mask_nos_bit_match(NnPos *nnpos, int part_num_start, int part_num_delta)
{
	unsigned int os_bit_r = nnpos->os_bit;

	BitString bs = BIT_STRING_INITIALIZER;
	convert_to_bitstring(part_num_start, part_num_start+part_num_delta, &bs);

	for (int osid=0; osid<MAX_NOS_SUPPORT; osid++)
	{
		if (nnpos->os_bit & (1 << osid))
		{
			if (nnpos->nosbit[osid])
			{
				NosBit *nosbit = nnpos->nosbit[osid];

				int r = nosbit->is_allbit_set(nosbit, &bs);

				if (!r)
				{
					os_bit_r &= ~(1 << osid); // remove the os bit
				}
			}	
		}
	}

	return os_bit_r;
}

NnPos *nnpos_create(void)
{
	NnPos *nnpos = malloc(sizeof(NnPos));
	if (!nnpos)
	{
		printf("Error: nnpos cannot be created\n");
		exit(1);
	}	

	nnpos->os_bit = 0;
	
	for (int i=0; i<MAX_NOS_SUPPORT; i++)
	{
		nnpos->nosbit[i] = NULL;
	}

	nnpos->set_nos_bit = nnpos_set_nos_bit;
	nnpos->clr_nos_bit = nnpos_clr_nos_bit;
	nnpos->is_nos_bit = nnpos_is_nos_bit;
	nnpos->is_nos_bit_match = nnpos_is_nos_bit_match;
	nnpos->mask_nos_bit_match = nnpos_mask_nos_bit_match;

	return (nnpos);
}

void nnpos_free(NnPos *nnpos)
{
	for (int i=0; i<MAX_NOS_SUPPORT; i++)
	{
		if (nnpos->nosbit[i])
		{
			nosbit_free(nnpos->nosbit[i]);
			nnpos->nosbit[i] = NULL;
		}
	}

	free(nnpos);
	nnpos = NULL;
}
