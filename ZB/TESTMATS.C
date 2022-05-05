/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: testmats.c 1.18 1995/02/22 21:53:42 sam Exp $
 * $Locker:  $
 *
 * A temporary table of materials until real material handling is implemented
 */

#include "brender.h"

static char rscid[] = "$Id: testmats.c 1.18 1995/02/22 21:53:42 sam Exp $";

br_material test_materials[] = {
	{
	"grey",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.00),			/* ks				*/
	BR_SCALAR(50),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"grey_flat",
	BR_COLOUR_RGB(255,10,10),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.00),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT,				/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"red",
	BR_COLOUR_RGB(255,10,10),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.40),			/* ks				*/
	BR_SCALAR(30),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	160,31,						/* index base/range	*/
	},
	{
	"red_flat",
	BR_COLOUR_RGB(255,10,10),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT,				/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	160,31,						/* index base/range	*/
	},
	{
	"green",
	BR_COLOUR_RGB(10,255,10),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	96,31,						/* index base/range	*/
	},
	{
	"green_flat",
	BR_COLOUR_RGB(10,255,10),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),							/* power			*/
	BR_MATF_LIGHT,					/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	96,31,						/* index base/range	*/
	},
	{
	"blue",
	BR_COLOUR_RGB(10,10,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.01),			/* kd				*/
	BR_UFRACTION(0.90),			/* ks				*/
	BR_SCALAR(70),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	64,31,						/* index base/range	*/
	},
	{
	"blue_flat",
	BR_COLOUR_RGB(10,10,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),							/* power			*/
	BR_MATF_LIGHT,					/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	64,31,						/* index base/range	*/
	},
	{
	"cyan",
	BR_COLOUR_RGB(255,255,10),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(70),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	128,31,						/* index base/range	*/
	},
	{
	"cyan_flat",
	BR_COLOUR_RGB(255,255,10),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),							/* power			*/
	BR_MATF_LIGHT,					/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	128,31,						/* index base/range	*/
	},
	{
	"magenta",
	BR_COLOUR_RGB(255,10,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(70),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	192,31,						/* index base/range	*/
	},
	{
	"magenta_flat",
	BR_COLOUR_RGB(255,10,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),							/* power			*/
	BR_MATF_LIGHT,					/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	192,31,						/* index base/range	*/
	},
	{
	"yellow",
	BR_COLOUR_RGB(10,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(70),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	224,31,						/* index base/range	*/
	},
	{
	"yellow_flat",
	BR_COLOUR_RGB(10,255,255),	/* c* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),							/* power			*/
	BR_MATF_LIGHT,					/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	224,31,						/* index base/range	*/
	},
	{
	"mandrill",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.20),			/* ka				*/
	BR_UFRACTION(0.45),			/* kd				*/
	BR_UFRACTION(0.90),			/* ks				*/
	BR_SCALAR(15),				/* power			*/
	BR_MATF_LIGHT|BR_MATF_SMOOTH|BR_MATF_PERSPECTIVE,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"jupiter",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.05),			/* ka				*/
	BR_UFRACTION(0.75),			/* kd				*/
	BR_UFRACTION(0.00),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT|BR_MATF_SMOOTH, /* flags			*/
	0,63,						/* index base/range	*/
	},
	{
	"rosewood",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.05),			/* ka				*/
	BR_UFRACTION(0.45),			/* kd				*/
	BR_UFRACTION(0.40),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH,	/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"rock",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.70),			/* kd				*/
	BR_UFRACTION(0.10),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT | BR_MATF_SMOOTH, /* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"brick",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.70),			/* kd				*/
	BR_UFRACTION(0.10),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT|BR_MATF_SMOOTH, /* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"earth",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.05),			/* ka				*/
	BR_UFRACTION(0.30),			/* kd				*/
	BR_UFRACTION(0.45),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT|BR_MATF_SMOOTH, /* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"test_texture",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.30),			/* kd				*/
	BR_UFRACTION(0.90),			/* ks				*/
	BR_SCALAR(20),				/* power			*/

#if 0
	BR_MATF_LIGHT|BR_MATF_SMOOTH,
#else
	0,
#endif

	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"terrain",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.30),			/* kd				*/
	BR_UFRACTION(0.90),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_PERSPECTIVE, 		/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"test_environment",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.20),			/* ka				*/
	BR_UFRACTION(0.50),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT|BR_MATF_SMOOTH|BR_MATF_ENVIRONMENT_I, /* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"test_environment_1",
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.20),			/* ka				*/
	BR_UFRACTION(0.50),			/* kd				*/
	BR_UFRACTION(0.60),			/* ks				*/
	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT|BR_MATF_SMOOTH|BR_MATF_ENVIRONMENT_L, /* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	0,63,						/* index base/range	*/
	},
	{
	"solid_red",
	BR_COLOUR_RGB(255,0,0),		/* colour			*/
	255,						/* opacity			*/
	BR_UFRACTION(0.10),			/* ka				*/
	BR_UFRACTION(0.60),			/* kd				*/
	BR_UFRACTION(0.40),			/* ks				*/
	BR_SCALAR(30),				/* power			*/
	BR_MATF_FORCE_Z_0,			/* flags			*/
	{{ BR_VECTOR2(1,0), BR_VECTOR2(0,1), BR_VECTOR2(0,0), }}, /* Map transform */
	184,0,						/* index base/range	*/
	},
};

/*
 * Size of the above table
 */
int test_materials_count = sizeof(test_materials)/sizeof(test_materials[0]);

