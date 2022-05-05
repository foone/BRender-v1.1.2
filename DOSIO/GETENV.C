/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: getenv.c 1.2 1995/02/22 22:07:03 sam Exp $
 * $Locker:  $
 *
 * Custom environment support
 */
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

#include "fw.h"
#include "syshost.h"

#if HAS_ENV_H
#include <env.h>
#endif


char * BR_ASM_CALL DOSGetEnv(char *name)
{
	return getenv(name);
}

