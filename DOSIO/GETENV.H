/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: getenv.h 1.2 1995/02/22 22:07:03 sam Exp $
 * $Locker:  $
 *
 * Custom environment support
 */
#ifndef _GETENV_H_
#define _GETENV_H_

#ifdef __cplusplus
extern "C" {
#endif

char * BR_ASM_CALL DOSGetEnv(char *name);

#ifdef __cplusplus
};
#endif
#endif
