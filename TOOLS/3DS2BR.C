/*
 *    Copyright Voxar Limited 1995
 *
 *    3ds2br.c : the main() function which sets up the options, and 
 *               calls Parse3dsBin() from source file parse3ds.c.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <brender.h>

#include "basetype.h"
#include "parse3ds.h"

#define MAX_FILE_LINE_LEN (255) /* Fixed in the next release, as they */
			        /* say. */

#define SHOW_TOKENS       (FALSE)


typedef enum CommandFlag_t {
    cf_none,
    cf_help,
    cf_verbose,
    cf_mod,
    cf_mat,
    cf_scr,
    cf_act,
    cf_flat,
    cf_no_lights,
    cf_no_cameras,
    cf_log,
    cf_hither,
    cf_yon,
    cf_pc
} CommandFlag_t;

typedef struct FlagParseState_t {
    char *command;
    CommandFlag_t last_flag;
    Parse3dsBinOptions_t *options;
} FlagParseState_t;

int ParseAtFile(FlagParseState_t *fp_state, char *filename);

static
    void
DisplayUsage(
    char *command
) {
    fprintf(stdout,
	    "Usage: %s <input file name>\n"
	    "       [ -h ]"
	    "    Display this message.\n"
	    "       [ -v ]"
	    "    Turns verbose mode on.\n"
	    "       [ -mod <model file name> ]"
	    "    Save models to file.\n" 
	    "       [ -mat <material file name> ]"
	    "    Save materials to file.\n" 
	    "       [ -scr <material script file name> ]"
	    "    Save materials to script file.\n" 
	    "       [ -act <actor file name> ]"
	    "    Save actor hierarchy to file.\n" 
	    "       [ -log <log file name> ]"
	    "    Save log to file.\n"
        "       [ -flat ]"
        "    Ignore keyframer hierarchy and build a flat one instead.\n"
        "       [ -nopivot ]"
        "    Ignore keyframer pivot point\n"
        "       [ -nomatrix ]"
        "    Ignore mesh matrix\n"
        "       [ -noaxis ]"
        "    Do not remap axes to corespond to 3DS user interface\n"
	    "       [ -nl ]"
	    "    Replace lights with dummy actors.\n"
	    "       [ -nc ]"
	    "    Replace cameras with dummy actors.\n"
	    "       [ -hither <distance > ]"
	    "    Sets all camera hither distances to this value.\n"
	    "       [ -yon <distance > ]"
	    "    Sets all camera yon distances to this value.\n"
            "       [ -pc ]"
            "    Set perspective correction for all textured materials.\n",
            command); 
}

    Bool_t
ConditionalSetString(
    char **pointer, 
    char *arg, 
    char *err_msg
) {
    if (*pointer != NULL) {
        printf(err_msg, arg);
        return FALSE;
    } else {
        *pointer = strdup(arg);
        if (*pointer == NULL) {
            printf("Out of memory!\n");
            return FALSE;
        }
    }
    return TRUE;
}

    Bool_t
NextToken(
    FlagParseState_t *fp_state,
    char *tok_string
) {
    Bool_t is_ok = TRUE;

    switch (fp_state->last_flag) {
    case cf_none:
        switch (tok_string[0]) {
        case '\0':
            is_ok = FALSE;
            break;
        case '@':
            is_ok = ParseAtFile(fp_state, tok_string + 1);
            fp_state->last_flag = cf_none;
            break;
        case '-':
	    if (!strcmp(tok_string, "-h")) {
#if SHOW_TOKENS
		printf("Help flag found.\n");
#endif
		DisplayUsage(fp_state->command);
		fp_state->last_flag = cf_none;
	    } else if (!strcmp(tok_string, "-v")) {
#if SHOW_TOKENS
		printf("Verbose flag found.\n");
#endif
		fp_state->options->verbose = TRUE;
		fp_state->last_flag = cf_none;
	    } else if (!strcmp(tok_string, "-mod")) {
		fp_state->last_flag = cf_mod;
	    } else if (!strcmp(tok_string, "-mat")) {
		fp_state->last_flag = cf_mat;
	    } else if (!strcmp(tok_string, "-scr")) {
		fp_state->last_flag = cf_scr;
	    } else if (!strcmp(tok_string, "-act")) {
		fp_state->last_flag = cf_act;
	    } else if (!strcmp(tok_string, "-log")) {
		fp_state->last_flag = cf_log;
	    } else if (!strcmp(tok_string, "-flat")) {
#if SHOW_TOKENS
                printf("Flat hierarchy flag: \"%s\"\n", tok_string);
#endif
		fp_state->options->flat_hierarchy = TRUE;
		fp_state->last_flag = cf_none;
	    } else if (!strcmp(tok_string, "-nopivot")) {
		fp_state->options->apply_pivot = FALSE;
		fp_state->last_flag = cf_none;
	    } else if (!strcmp(tok_string, "-nomatrix")) {
		fp_state->options->apply_meshmat = FALSE;
		fp_state->last_flag = cf_none;
	    } else if (!strcmp(tok_string, "-noaxis")) {
		fp_state->options->correct_axis = FALSE;
		fp_state->last_flag = cf_none;
	    } else if (!strcmp(tok_string, "-nl")) {
#if SHOW_TOKENS
                printf("No lights flag: \"%s\"\n", tok_string);
#endif
		fp_state->options->no_lights = TRUE;
		fp_state->last_flag = cf_none;
	    } else if (!strcmp(tok_string, "-nc")) {
#if SHOW_TOKENS
                printf("No cameras flag: \"%s\"\n", tok_string);
#endif
	    } else if (!strcmp(tok_string, "-pc")) {
#if SHOW_TOKENS
                printf("Perspective correction flag found.\n");
#endif
		fp_state->options->perspective_tex = TRUE;
		fp_state->last_flag = cf_none;
	    } else if (!strcmp(tok_string, "-hither")) {
		fp_state->last_flag = cf_hither;
	    } else if (!strcmp(tok_string, "-yon")) {
		fp_state->last_flag = cf_yon;
	    } else {
                printf("Unrecognised flag \"%s\"\n", tok_string);
		is_ok = FALSE;
	    }
            break;
        default:
#if SHOW_TOKENS
            printf("Unannounced argument: \"%s\"\n", tok_string);
#endif
	    is_ok = ConditionalSetString(
	        &(fp_state->options->input_filename),
		tok_string, "Extra input filename %s\n");
	    fp_state->last_flag = cf_none;
	    break;
	}
	break;
    case cf_mod:
#if SHOW_TOKENS
        printf("Mod flag: \"%s\"\n", tok_string);
#endif
	is_ok = ConditionalSetString(
	    &(fp_state->options->mod_filename),
            tok_string, "Extra mod file name %s\n");
	fp_state->last_flag = cf_none;
	break;
    case cf_mat:
#if SHOW_TOKENS
        printf("Mat flag: \"%s\"\n", tok_string);
#endif
	is_ok = ConditionalSetString(
	    &(fp_state->options->mat_filename),
            tok_string, "Extra mat file name %s\n");
	fp_state->last_flag = cf_none;
	break;
    case cf_scr:
#if SHOW_TOKENS
        printf("Scr flag: \"%s\"\n", tok_string);
#endif
	is_ok = ConditionalSetString(
	    &(fp_state->options->scr_filename),
            tok_string, "Extra scr file name %s\n");
	fp_state->last_flag = cf_none;
	break;
    case cf_act:
#if SHOW_TOKENS
        printf("act flag: \"%s\"\n", tok_string);
#endif
	is_ok = ConditionalSetString(
	    &(fp_state->options->act_filename),
            tok_string, "Extra act file name %s\n");
	fp_state->last_flag = cf_none;
	break;
    case cf_log:
#if SHOW_TOKENS
        printf("log flag: \"%s\"\n", tok_string);
#endif
	is_ok = ConditionalSetString(
	    &(fp_state->options->log_filename),
            tok_string, "Extra log filename option %s\n");
	fp_state->last_flag = cf_none;
	break;
    case cf_hither:
#if SHOW_TOKENS
        printf("hither flag: \"%s\"\n", tok_string);
#endif
	is_ok = ConditionalSetString(
	    &(fp_state->options->hither_string),
            tok_string, "Extra hither option %s\n");
	fp_state->last_flag = cf_none;
	break;
    case cf_yon:
#if SHOW_TOKENS
        printf("yon flag: \"%s\"\n", tok_string);
#endif
	is_ok = ConditionalSetString(
	    &(fp_state->options->yon_string),
            tok_string, "Extra yon option %s\n");
	fp_state->last_flag = cf_none;
	break;
    default:
	printf("Internal error: unexpected parser state.\n");
	is_ok = FALSE;
	break;
    }
    return is_ok;
}

    Bool_t
ParseCmdString(
    FlagParseState_t *fp_state,
    char *cmd_string /* reserves the right to temporarily modify it! */
) {
    Bool_t is_ok = TRUE;
    char *p, *arg_begin, tmp_c;
    
    p = cmd_string;
    while (is_ok && *p != '\0') {
        while (isspace(*p)) {
           p++;
        }
        arg_begin = p;
	while (*p != '\0' && !isspace(*p)) {
           p++;
        }
        if (*arg_begin != '\0') {
            tmp_c = *p;
            *p = '\0';
            is_ok = NextToken(fp_state, arg_begin);
            *p = tmp_c;
        }
    }
    return is_ok;
}

    Bool_t
ParseAtFile(
    FlagParseState_t *fp_state,
    char *filename
) {
    Bool_t is_ok = TRUE;
    FILE *fp;
    char *line_buffer;
    int line_buffer_length;

    line_buffer_length = MAX_FILE_LINE_LEN;
    line_buffer = malloc(line_buffer_length);
    if (line_buffer == NULL) {
        is_ok = FALSE;
    }
    if (is_ok) {
	fp = fopen(filename, "r");
	if (fp == NULL) {
	    printf("File opening failed.\n");
	    is_ok = FALSE;
	}
    }
    while (is_ok && !ferror(fp) && !feof(fp)) {
	if (fgets(line_buffer, line_buffer_length, fp) != NULL){
            /* Insert checks for buffer overrun here */
            ParseCmdString(fp_state, line_buffer);
        }
    }
    if (line_buffer != NULL) {
        free(line_buffer);
    }
    return is_ok;
}

    Bool_t
ParseArgv(
    FlagParseState_t *fp_state,
    char *argv[]
) {
    Bool_t is_ok = TRUE;

    while (is_ok && *argv != NULL) { /* This is ANSI standard now. */
        is_ok = NextToken(fp_state, *argv);
        argv++;
    }
    return is_ok;
}

    int
main(
    int  argc,
    char **argv
) {
    Parse3dsBinOptions_t options;
    FlagParseState_t fp_state;
    Bool_t is_ok = TRUE;

    fprintf(stdout,"3ds2br v1.1. Copyright Voxar Limited 1995.\n");

    options.input_filename = NULL;

    options.verbose = FALSE;

    options.mod_filename = NULL;
    options.mat_filename = NULL;
    options.scr_filename = NULL;
    options.act_filename = NULL;
    options.log_filename = NULL;

    options.log = NULL;

    options.flat_hierarchy = FALSE;

    options.perspective_tex = FALSE;

    options.apply_pivot = TRUE;
    options.apply_meshmat = TRUE;
    options.correct_axis = TRUE;

    options.no_lights  = FALSE;
    options.no_cameras = FALSE;

    options.hither = 1.0;
    options.hither_string = NULL;

    options.yon = 2000.0;
    options.yon_string = NULL;

    fp_state.command = argv[0];
    fp_state.options = &options;
    fp_state.last_flag = cf_none;
    is_ok = ParseArgv(&fp_state, argv +1);
    if (is_ok && options.input_filename == NULL) {
	DisplayUsage(argv[0]);
        printf("\nNo input filename specified.\n");
	is_ok = FALSE;
    }
    if (is_ok && options.hither_string != NULL) {
        options.hither = atof(options.hither_string);
        /* FIXME: atof has no error checking.  We could do with some. */
    }
    if (is_ok && options.yon_string != NULL) {
        options.yon = atof(options.yon_string);
        /* FIXME: atof has no error checking.  We could do with some. */
    }
    if (is_ok) {
        if (options.log_filename != NULL) {
            options.log = fopen(options.log_filename, "w");
	    if (options.log == NULL) {
	        printf("Couldn't open log file %s\n",options.log_filename);
                is_ok = FALSE;
            }
        } else if (options.verbose) {
            options.log = stdout;
        }
    }

    if (is_ok) {
	BrBegin();
        is_ok = Parse3dsBin(&options);
	BrEnd();
    }
    
    return is_ok?0:-1;
}
