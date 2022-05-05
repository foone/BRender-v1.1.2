/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stderr.c 1.5 1995/02/22 22:02:03 sam Exp $
 * $Locker:  $
 *
 * Default error handler that reports error through stderr
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

static void BR_CALLBACK BrStdioError(char *message)
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
br_errorhandler BrStdioErrorHandler = {
	"Stdio ErrorHandler",
	BrStdioWarning,
	BrStdioError,
};

/*
 * Override default
 */
br_errorhandler *_BrDefaultErrorHandler = &BrStdioErrorHandler;


