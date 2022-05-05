#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <ctype.h>

#include "brender.h"
#include "brassert.h"
#include "syshost.h"
#include "dosio.h"

#include "dosgfxcm.h"

/*
 * Magic cookies to catch brender diagnostics and switch back to text mode
 */
static void BR_CALLBACK _DOSGfxFailure(char *message)
{
	if(pc_gfx_set) {
		DOSGfxEnd();
	}

	fflush(stdout);
	fputs(message,stderr);
	fputc('\n',stderr);
	fflush(stderr);
	exit(10);
}

static struct br_diaghandler brender_diaghandler = {
	"DOS GFX ErrorHandler",
	_DOSGfxFailure,
	_DOSGfxFailure,
};

struct br_diaghandler *_BrDefaultDiagHandler = &brender_diaghandler;

