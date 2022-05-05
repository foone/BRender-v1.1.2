/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stddiag.c 1.1 1995/07/28 19:03:47 sam Exp $
 * $Locker:  $
 *
 * Default diagnostic handler that reports through stderr
 */
#include <stdio.h>
#include <stdlib.h>

#include "brender.h"

static void BR_CALLBACK BrStdioWarning(char *message)
{
	fflush(stdout);
	fputs(message,stderr);
	fputc('\n',stderr);
	fflush(stderr);
}

static void BR_CALLBACK BrStdioFailure(char *message)
{
	fflush(stdout);
	fputs(message,stderr);
	fputc('\n',stderr);
	fflush(stderr);
	exit(10);
}

/*
 * ErrorHandler structure
 */
br_errorhandler BrStdioDiagHandler = {
	"Stdio DiagHandler",
	BrStdioWarning,
	BrStdioFailure,
};

/*
 * Override default
 */
br_diaghandler *_BrDefaultDiagHandler = &BrStdioDiagHandler;


