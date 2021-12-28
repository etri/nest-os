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
#ifndef NOS_BIT_H
#define NOS_BIT_H
#include "bit_handler.h"

typedef struct nos_bit
{
	BitString bs;

	void (*set_bit)(struct nos_bit *nosbit, BitString *bs);
	void (*clr_bit)(struct nos_bit *nosbit, BitString *bs);
	int  (*is_partbit_set)(struct nos_bit *nosbit, BitString *bs);
	int  (*is_allbit_set)(struct nos_bit *nosbit, BitString *bs);
	int  (*is_allbit_clr)(struct nos_bit *nosbit, BitString *bs);
	int  (*is_allbit_zero)(struct nos_bit *nosbit);
} NosBit;

typedef struct nn_pos
{
	unsigned int os_bit; // os_bit, allows up to 32 noses
	NosBit *nosbit[MAX_NOS_SUPPORT]; // 32 noses 

	void (*set_nos_bit)(struct nn_pos *nnpos, int osid, int part_num_start, int part_num_delta);
	void (*clr_nos_bit)(struct nn_pos *nnpos, int osid, int part_num_start, int part_num_delta);
	int (*is_nos_bit)(struct nn_pos *nnpos, int osid, int part_num_start, int part_num_delta);
	int (*is_nos_bit_match)(struct nn_pos *nnpos, int osid, int part_num_start, int part_num_delta);
	unsigned int (*mask_nos_bit_match)(struct nn_pos *nnpos, int part_num_start, int part_num_delta);
} NnPos;

NnPos *nnpos_create(void);
void nnpos_free(NnPos *nnpos);
#endif
