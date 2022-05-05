/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: segregs.h 1.3 1995/06/30 15:42:22 sam Exp $
 * $Locker:  $
 */
#ifndef _SEGREGS_H_
#define _SEGREGS_H_

#ifdef __cplusplus
extern "C" {
#endif

unsigned short BR_ASM_CALL GetDS(void);
unsigned short BR_ASM_CALL GetCS(void);
unsigned short BR_ASM_CALL GetSS(void);
unsigned short BR_ASM_CALL GetES(void);

#ifdef __cplusplus
};
#endif
#endif

