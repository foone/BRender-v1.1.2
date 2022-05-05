#ifndef _PARSE3DS_H_
#define _PARSE3DS_H_
/****************************************************************************
  parse3ds.h

  Copyright Voxar Limited 1995

  $Id: parse3ds.h 1.5 1995/08/31 16:39:29 sam Exp $

  This is the header file for the parse3ds module. This module
provides a facility for converting files in 3D Studio binary (.3ds)
format to BRender format.

  This file defines the structure Parse3dsBinOptions_t. The main
parsing function, Parse3dsBin expects a pointer to one of these as
argument. 

  The various fields of the structure convey information
that is supplied by the user, as opposed to the 3DS file, such as
file names and what to do in certain odd cases.

****************************************************************************/


typedef struct Parse3dsBinOptions_t {
    char    *input_filename;  /* Guess! */

    Bool_t  verbose;          /* Maybe we need a number for this */

    char    *mat_filename;    /* Where to save the materials */
    char    *mod_filename;    /* Where to save the models */
    char    *act_filename;    /* Where to save the actor hierarchy */
    char    *scr_filename;    /* where to save the material script */
    char    *log_filename;    /* where to save the logged info */
    FILE    *log;             /* if !=NULL, log material and mesh info */

    Bool_t  flat_hierarchy;   /* if true, ignore keyframer hierarchy and */
                              /* build a flat one instead */

    Bool_t  perspective_tex;  /* If true, set perspective correct */
						      /* texture flag on materials */

    Bool_t  correct_axis;	  /* If true, remap axes to corespond to UI */

    Bool_t  apply_meshmat;	  /* If true, apply mesh matrix to model vertices */

    Bool_t  apply_pivot;	  /* If true, apply pivot to model vertices */

    Bool_t  no_lights;        /* If true, ignore lights in input file */
    Bool_t  no_cameras;       /* If true, ignore cameras in input file */

    char    *hither_string;   /* if !=NULL, all hithers become this */
    Float_t hither;           /* defaults to 1.0 */
    char    *yon_string;
    Float_t yon;              /* defaults to 2000.0 */

} Parse3dsBinOptions_t;


extern
    Bool_t
Parse3dsBin(
    Parse3dsBinOptions_t *options
);

/*
 * Flags that are added to BRender face flags to indicate texture wrapping
 */
#define FACEF_UWRAP	0x40
#define FACEF_VWRAP	0x80

#endif /* _PARSE3DS_H_ */

