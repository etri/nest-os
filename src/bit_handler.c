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
#include "bit_handler.h"

// convert (start, end) <integer pair string> into <bit string> representation
void convert_to_bitstring(int start, int end, BitString *bs)
{
	for (int i=start; i<=end; i++)
	{
		int p, q;

		p = i/BIT_UNIT;
		q = i%BIT_UNIT;

		bs->bit[p] |= (1 << q);
	}
}

// set a block of *bs to *bs_t block 
void partblock_set_bit(BitString *bs_t, BitString *bs)
{
	for (int i=0; i<BIT_STRING_NUM; i++)
	{
		bs_t->bit[i] |= bs->bit[i];
	}
}

// clear a block of *bs to *bs_t block 
void partblock_clr_bit(BitString *bs_t, BitString *bs)
{
	for (int i=0; i<BIT_STRING_NUM; i++)
	{
		bs_t->bit[i] &= ~bs->bit[i];
	}
}

// check if some of bits in *bs_t block are set
int partblock_is_partbit_set(BitString *bs_t, BitString *bs)
{
	BitString bs_r;

	int count = 0;
	for (int i=0; i<BIT_STRING_NUM; i++)
	{
		bs_r.bit[i] = bs_t->bit[i] & bs->bit[i];
		count += bs_r.bit[i] > 0;
	}
	return (count > 0);
}

// check if a block of bits in *bs_t is all set
int partblock_is_allbit_set(BitString *bs_t, BitString *bs)
{
	BitString bs_r = BIT_STRING_INITIALIZER;

	int count = 0;
	for (int i=0; i<BIT_STRING_NUM; i++)
	{
		bs_r.bit[i] = bs_t->bit[i] & bs->bit[i];
		count += (bs_r.bit[i] == bs->bit[i]);
	}
	return (count == BIT_STRING_NUM);
}

// check if a block of bits in *bs_t is all cleared
int partblock_is_allbit_clr(BitString *bs_t, BitString *bs)
{
	BitString bs_r = BIT_STRING_INITIALIZER;

	int count = 0;
	for (int i=0; i<BIT_STRING_NUM; i++)
	{
		bs_r.bit[i] = ~bs_t->bit[i] & bs->bit[i]; // bs_r is the partition block of bs_t for bs partition block
		count += (bs_r.bit[i] == bs->bit[i]);
	}
	return (count == BIT_STRING_NUM);
}

// check if all bits in *bs_t are cleared
int partblock_is_allbit_zero(BitString *bs_t)
{
	int count = 0;
	for (int i=0; i<BIT_STRING_NUM; i++)
	{
		count += (bs_t->bit[i] == 0);
	}
	return (count == BIT_STRING_NUM);
}
