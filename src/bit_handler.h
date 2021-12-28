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
#ifndef BIT_HANDLER_H
#define BIT_HANDLER_H
#include <stdint.h>

#define MAX_NOS_SUPPORT	32

#if 0
#define BIT_UNIT	32 // 32 bit
#define BIT_STRING_NUM	4  // 4*32 = 128 bits
#define BIT_STRING_INITIALIZER {{0, 0, 0, 0}}
typedef uint32_t	BitType;
#else
#define BIT_STRING_INITIALIZER {{0, 0}}
#define BIT_UNIT	64 // 64 bit
#define BIT_STRING_NUM	2  // 2*64 = 128 bits
typedef uint64_t	BitType;
#endif

#define BIT_MAX_NUM	(BIT_UNIT*BIT_STRING_NUM)

typedef struct bit_string
{
	BitType bit[BIT_STRING_NUM]; // 4 bytes(unsigned int) * 4 = 16 bytes (128 bits = 128 partitions)
} BitString;

void convert_to_bitstring(int start, int end, BitString *bs);
void partblock_set_bit(BitString *bs_t, BitString *bs);
void partblock_clr_bit(BitString *bs_t, BitString *bs);
int partblock_is_partbit_set(BitString *bs_t, BitString *bs);
int partblock_is_allbit_set(BitString *bs_t, BitString *bs);
int partblock_is_allbit_clr(BitString *bs_t, BitString *bs);
int partblock_is_allbit_zero(BitString *bs_t);

#endif
