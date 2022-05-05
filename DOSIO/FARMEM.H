/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: farmem.h 1.1 1995/06/30 15:43:35 sam Exp $
 * $Locker:  $
 */
#ifndef _FARMEM_H_
#define _FARMEM_H_

#ifdef __cplusplus
extern "C" {
#endif

void BR_ASM_CALL FarBlockWrite(unsigned int offset, unsigned short seg, unsigned char *block, int count);
void BR_ASM_CALL FarBlockRead(unsigned int offset, unsigned short seg, unsigned char *block, int count);

void BR_ASM_CALL FarBlockFill(unsigned int offset, unsigned short seg, unsigned char value, int count);

void BR_ASM_CALL FarByteWrite(unsigned int offset, unsigned short seg, unsigned char value);
void BR_ASM_CALL FarWordWrite(unsigned int offset, unsigned short seg, unsigned short value);
void BR_ASM_CALL FarDWordWrite(unsigned int offset, unsigned short seg, unsigned long value);

unsigned char BR_ASM_CALL FarByteRead(unsigned int offset, unsigned short seg);
unsigned short BR_ASM_CALL FarWordRead(unsigned int offset, unsigned short seg);
unsigned long BR_ASM_CALL FarDWordRead(unsigned int offset, unsigned short seg);

#ifdef __cplusplus
};
#endif
#endif
