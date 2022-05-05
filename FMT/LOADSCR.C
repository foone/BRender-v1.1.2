/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: loadscr.c 1.7 1995/08/31 16:28:18 sam Exp $
 * $Locker:  $
 * $BC<"make -f zbtest.mak %s.obj;">
 *
 * Load a Material Script file
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "brender.h"
#include "fmt.h"

#define LEX_DEBUG 0

static char rscid[] = "$Id: loadscr.c 1.7 1995/08/31 16:28:18 sam Exp $";

/*
 * Parsing macros
 */
#if LEX_DEBUG
#define LexAdvance LexAdvanceDebug
#else
#define LexAdvance LexAdvanceReal
#endif

#define ADVANCE() LexAdvance()
#define EXPECT(t) {if(CURRENT != (t)) LexTokenError(t); else LexAdvance();}
#define CHECK(t)  {if(CURRENT != (t)) LexTokenError(t);}
#define ERROR(s) LexError(s)

#define CURRENT			(Current.id)
#define CURRENT_SCALAR	(Current.scalar)
#define CURRENT_STRING	(Current.string)


/*
 * Default material fields
 */
br_material _DefaultScriptMaterial = {
	NULL,
	BR_COLOUR_RGB(255,255,255),	/* colour			*/
	255,						/* opacity			*/

	BR_UFRACTION(0.10),			/* Indexed ka		*/
	BR_UFRACTION(0.70),			/*         kd		*/
	BR_UFRACTION(0.0),			/*         ks		*/

	BR_SCALAR(20),				/* power			*/
	BR_MATF_LIGHT,				/* flags			*/
	{{
		BR_VECTOR2(1,0),		/* map transform	*/
		BR_VECTOR2(0,1),
		BR_VECTOR2(0,0),
	}},
	0,63,						/* index base/range	*/
};

/*
 * Token IDs 
 */
typedef enum {
	T_EOF,

	/*
	 * Constants
	 */
	T_SCALAR,
	T_STRING,
	T_IDENT,

	/*
	 * Single characters
	 */
	T_LSQUARE 	= '[',
	T_RSQUARE 	= ']',
	T_SEMI		= ';',
	T_EQUALS	= '=',
	T_COMMA		= ',',

	/*
	 * Keywords - see below
	 */
	T_MATERIAL = 128,

	/*
	 * Members of material
	 */
	T_IDENTIFIER,
	T_NAME,
	T_FLAGS,
	T_COLOUR,
	T_OPACITY,
	T_AMBIENT,
	T_KA,
	T_DIFFUSE,
	T_KD,
	T_SPECULAR,
	T_KS,
	T_POWER,
	T_MAP_TRANSFORM,
	T_INDEX_BASE,
	T_INDEX_RANGE,
	T_COLOUR_MAP,
	T_SCREEN_DOOR,
	T_INDEX_SHADE,
	T_INDEX_BLEND,

	/*
	 * Material flags
	 */
	T_LIGHT,
	T_PRELIT,
	T_SMOOTH,
	T_GOURAUD,
	T_ENVIRONMENT,
	T_ENVIRONMENT_I,
	T_ENVIRONMENT_LOCAL,
	T_ENVIRONMENT_L,
	T_PERSPECTIVE,
	T_DECAL,
	T_ALWAYS_VISIBLE,
	T_TWO_SIDED,
	T_FORCE_Z_0,
	T_DITHER

} scr_token_id;

STATIC char * token_names[] = {
	"end of file",
	"scalar",
	"string",
	"identfier",
};

STATIC char * token_keywords[] = {
	"material",
	"identifier",
	"name",
	"flags",
	"colour",
	"opacity",
	"ambient",
	"ka",
	"diffuse",
	"kd",
	"specular",
	"ks",
	"power",
	"map_transform",
	"index_base",
	"index_range",
	"colour_map",
	"screen_door",
	"index_shade",
	"index_blend",
	"light",
	"prelit",
	"smooth",
	"gouraud",
	"environment",
	"environment_i",
	"environment_local",
	"environment_l",
	"perspective",
	"decal",
	"always_visible",
	"two_sided",
	"force_z_0",
	"dither",
};

/*
 * A token - id + value
 */
struct scr_token {
	/*
	 * Token ID
	 */
	scr_token_id	id;
	/*
	 * Numerical value
	 */
	br_scalar		scalar;
	/*
	 * String value
	 */
	char *			string;
};

/*
 * The current token
 */
STATIC struct scr_token Current;

/*
 * The current file
 */
struct lex_file {
	int line;
	char *name;
	void *handle;
	int next_char;
} CurrentFile;

/*
 * Start processing a file
 */
STATIC int LexAddFile(char *filename)
{
	int mode = BR_FS_MODE_TEXT;

#if LEX_DEBUG
	printf("LexAddFile(%s)\n",filename);
#endif

	CurrentFile.line = 1;
	CurrentFile.name = filename;
	CurrentFile.handle = BrFileOpenRead(filename, 0, NULL, &mode);

	if(CurrentFile.handle == NULL) {
#if LEX_DEBUG
		printf("LexAddFile - could not open file\n");
#endif
		return 0;
	}

	/*
	 * File is always one character ahead of the current token
	 */
	CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);

	return 1;
}

/*
 * Close file
 */
STATIC void LexCloseFile(void)
{
#if LEX_DEBUG
	printf("LexCloseFile\n");
#endif
	BrFileClose(CurrentFile.handle);
}

/*
 * Report an error, including file name and line number
 */
STATIC int LexError(char *string)
{
	LexCloseFile();

	BR_ERROR3("%s in %s, line %d",string,CurrentFile.name,CurrentFile.line);

	return 0;
}

/*
 * Report an expected token
 */
STATIC int LexTokenError(scr_token_id token)
{
	LexCloseFile();

	if(token >= 128) {
		/*
		 * Token is >= 128, it is a keyword
		 */
		BR_ERROR3("Expected '%s' in %s, line %d",token_keywords[token-128],CurrentFile.name,CurrentFile.line);

	} else if(token <= ' ') {
		/*
		 * Token is <= ' ' - it is a constant or some other named ID
		 */
		BR_ERROR3("Expected %s in %s, line %d",token_names[token],CurrentFile.name,CurrentFile.line);

	} else {
		/*
		 * Otherwise it is a printable character
		 */
		BR_ERROR3("Expected '%c' in %s, line %d",token,CurrentFile.name,CurrentFile.line);
	}

	return 0;
}

/*
 * Collect next token from script file
 */

STATIC void LexAdvanceReal(void)
{
	int n;
	static char lex_buffer[256];

	for(;;) {

		if(CurrentFile.next_char == BR_EOF) {
			Current.id = T_EOF;
			return;
		} else if(CurrentFile.next_char == '#') {
			/*
			 * Comment - skip until EOL or EOF
			 */
			for(;;) {
				CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);
				if(CurrentFile.next_char == '\n' ||
				   CurrentFile.next_char == BR_EOF)
					break;
			}
			continue;

		} else if(CurrentFile.next_char == '\n') {
			/*
			 * Newline - track line number
			 */
			CurrentFile.line++;
			CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);
			continue;

		} else if(CurrentFile.next_char == '"') {
			/*
			 * Read a string - read characters until '"', newline, or EOF
			 */
			n = 0;
			for(n=0; n < BR_ASIZE(lex_buffer)-1; ) {
				CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);
				if(CurrentFile.next_char == '"' ||
				   CurrentFile.next_char == '\n' ||
				   CurrentFile.next_char == BR_EOF)
					break;
				lex_buffer[n++] = CurrentFile.next_char;
			}

			if(CurrentFile.next_char != '"')
				LexError("Unterminated string");

			lex_buffer[n] = '\0';
			Current.string = lex_buffer;
			Current.id = T_STRING;
			CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);
			return;

		} else if(isspace(CurrentFile.next_char) || CurrentFile.next_char == '\032') {
			/*
			 * white space - ignore
			 */
			CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);
			continue;

		} else if(isalpha(CurrentFile.next_char) || CurrentFile.next_char == '_') {
			/*
			 * Idenitifier - read characters until not alpha, _ or 0-9
			 */
			lex_buffer[0] = CurrentFile.next_char;

			for(n=1; n < BR_ASIZE(lex_buffer)-1; ) {
				CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);
				if(!(isalpha(CurrentFile.next_char) ||
				     isdigit(CurrentFile.next_char) ||
				     CurrentFile.next_char == '_'))
					break;
				lex_buffer[n++] = CurrentFile.next_char;
			}
			lex_buffer[n] = '\0';
			Current.string = lex_buffer;

			/*
			 * Try to look up a keyword
			 */
			for(n=0; n < BR_ASIZE(token_keywords); n++) {
				if(token_keywords[n][0] == lex_buffer[0] &&
					!stricmp(token_keywords[n],lex_buffer)) {
					/*
					 * Found a keyword - which are based at 128
					 */
					Current.id = n + 128;
					return;
				}
			}

			Current.id = T_IDENT;
			return;

		} else if(isdigit(CurrentFile.next_char) || CurrentFile.next_char == '-') {
			/*
			 * Read a number
			 */
			lex_buffer[0] = CurrentFile.next_char;

			for(n=1; n < BR_ASIZE(lex_buffer)-1; ) {
				CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);
				if(!(isdigit(CurrentFile.next_char) ||
				     CurrentFile.next_char == '.'))
					break;
				lex_buffer[n++] = CurrentFile.next_char;
			}
			lex_buffer[n] = '\0';

			Current.scalar = BrFloatToScalar(atof(lex_buffer));
			Current.string = lex_buffer;
			Current.id = T_SCALAR;
			return;

		} else if(isprint(CurrentFile.next_char)) {
			/*
			 * All other printable characters turn into tokens
			 */
			Current.id = CurrentFile.next_char;
			CurrentFile.next_char = BrFileGetChar(CurrentFile.handle);
			return;

		} else
			LexError("Unexpected character");
	}
}

#if LEX_DEBUG
STATIC void LexAdvanceDebug(void)
{
	int i;

	static struct {
		char *name;
		scr_token_id id;
	} token_names[] = {
		"T_EOF",				T_EOF,
		"T_SCALAR",				T_SCALAR,
		"T_STRING",				T_STRING,
		"T_IDENT",				T_IDENT,
		"T_LSQUARE",			T_LSQUARE,
		"T_RSQUARE",			T_RSQUARE,
		"T_SEMI",				T_SEMI,
		"T_EQUALS",				T_EQUALS,
		"T_COMMA",				T_COMMA,
		"T_MATERIAL",			T_MATERIAL,
		"T_IDENTIFIER",			T_IDENTIFIER,
		"T_NAME",				T_NAME,
		"T_FLAGS",				T_FLAGS,
		"T_COLOUR",				T_COLOUR,
		"T_OPACITY",			T_OPACITY,
		"T_AMBIENT",			T_AMBIENT,
		"T_KA",					T_KA,
		"T_DIFFUSE",			T_DIFFUSE,
		"T_KD",					T_KD,
		"T_SPECULAR",			T_SPECULAR,
		"T_KS",					T_KS,
		"T_POWER",				T_POWER,
		"T_MAP_TRANSFORM",		T_MAP_TRANSFORM,
		"T_INDEX_BASE",			T_INDEX_BASE,
		"T_INDEX_RANGE",		T_INDEX_RANGE,
		"T_COLOUR_MAP",			T_COLOUR_MAP,
		"T_SCREEN_DOOR",		T_SCREEN_DOOR,
		"T_INDEX_SHADE",		T_INDEX_SHADE,
		"T_INDEX_BLEND",		T_INDEX_BLEND,
		"T_LIGHT",				T_LIGHT,
		"T_PRELIT",				T_PRELIT,
		"T_SMOOTH",				T_SMOOTH,
		"T_GOURAUD",			T_GOURAUD,
		"T_ENVIRONMENT",		T_ENVIRONMENT,
		"T_ENVIRONMENT_I",		T_ENVIRONMENT_I,
		"T_ENVIRONMENT_LOCAL",	T_ENVIRONMENT_LOCAL,
		"T_ENVIRONMENT_L",		T_ENVIRONMENT_L,
		"T_PERSPECTIVE",		T_PERSPECTIVE,
		"T_DECAL",				T_DECAL,
		"T_ALWAYS_VISIBLE",		T_ALWAYS_VISIBLE,
		"T_TWO_SIDED",			T_TWO_SIDED,
		"T_FORCE_Z_0",			T_FORCE_Z_0,
		"T_DITHER",				T_DITHER,
	};

	LexAdvanceReal();

	for(i=0; i < BR_ASIZE(token_names); i++) {
		if(Current.id == token_names[i].id)
			break;
	}

	if(i < BR_ASIZE(token_names))
		printf("<%s>",token_names[i].name);
	else
		printf("<%03X>",token_names[i].id);

	if(Current.id == T_SCALAR)
		printf(" = %f (%s)",BrScalarToFloat(Current.scalar),Current.string);

	if(Current.id == T_STRING || Current.id == T_IDENT)
		printf(" = \"%s\"",Current.string);

	if(isgraph(CurrentFile.next_char))
		printf(" next_char = '%c'\n",CurrentFile.next_char);
	else
		printf(" next_char = %d\n",CurrentFile.next_char);
}
#endif

br_uint_32 ParseMatFlags(void)
{
	br_uint_32 f = 0;

	EXPECT(T_LSQUARE);

	if(CURRENT == T_RSQUARE) {
		ADVANCE();
		return 0;
	}

	/*
	 * Read flag keywords until ]
	 */
	for(;;) {
		switch(CURRENT) {
		case T_LIGHT:				f |= BR_MATF_LIGHT;				break;
		case T_PRELIT:				f |= BR_MATF_PRELIT;			break;
		case T_SMOOTH:
		case T_GOURAUD:				f |= BR_MATF_SMOOTH;			break;
		case T_ENVIRONMENT_I:
		case T_ENVIRONMENT:			f |= BR_MATF_ENVIRONMENT_I;		break;
		case T_ENVIRONMENT_L:
		case T_ENVIRONMENT_LOCAL:	f |= BR_MATF_ENVIRONMENT_L;		break;
		case T_PERSPECTIVE:			f |= BR_MATF_PERSPECTIVE;		break;
		case T_DECAL:				f |= BR_MATF_DECAL;				break;
		case T_ALWAYS_VISIBLE:		f |= BR_MATF_ALWAYS_VISIBLE;	break;
		case T_TWO_SIDED:			f |= BR_MATF_TWO_SIDED;			break;
		case T_FORCE_Z_0:			f |= BR_MATF_FORCE_Z_0;			break;
		case T_DITHER:				f |= BR_MATF_DITHER;			break;
		default:
			ERROR("Unknown material flag");
		}
		ADVANCE();

		if(CURRENT == T_RSQUARE)
			break;

		EXPECT(T_COMMA);
	}

	EXPECT(T_RSQUARE);

	return f;
}

int ParseVector(br_scalar *v, int max)
{
	int n;

	EXPECT(T_LSQUARE);

	for(n=0;n < max; n++) {
		CHECK(T_SCALAR);
		*v++ = CURRENT_SCALAR;
		ADVANCE();

		if(CURRENT == T_RSQUARE)
			break;
		EXPECT(T_COMMA);
	}

	EXPECT(T_RSQUARE);
	return n+1;
}

int ParseMatrix(br_scalar *m, int width, int max_h)
{
	int n;

	EXPECT(T_LSQUARE);

	for(n=0; n < max_h; n++, m+= width) {
		ParseVector(m,width);

		if(CURRENT == T_RSQUARE)
			break;

		EXPECT(T_COMMA);
	}

	EXPECT(T_RSQUARE);

	return n;
}


br_material *ParseMaterial(void)
{
	br_material *mat;
	br_vector3 v3;

	mat = BrMaterialAllocate(NULL);
	*mat = _DefaultScriptMaterial;

	/*
	 * material = [
	 */
	EXPECT(T_MATERIAL);
	EXPECT(T_EQUALS);
	EXPECT(T_LSQUARE);

	while(CURRENT != T_RSQUARE) {
		switch(CURRENT) {
		case T_IDENTIFIER:
		case T_NAME:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_STRING);
			mat->identifier = BrResStrDup(mat,CURRENT_STRING);
			ADVANCE();
			break;

		case T_FLAGS:
			ADVANCE(); EXPECT(T_EQUALS);
			mat->flags = ParseMatFlags();
			break;

		case T_COLOUR:
			ADVANCE(); EXPECT(T_EQUALS);

			if(ParseVector(&v3.v[0],3) != 3)
				ERROR("Colour has too few entries");

			mat->colour = BR_COLOUR_RGB(
				BrScalarToInt(v3.v[0]),
				BrScalarToInt(v3.v[1]),
				BrScalarToInt(v3.v[2]));
			break;

		case T_OPACITY:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_SCALAR);
			mat->opacity = CURRENT_SCALAR;
			ADVANCE();
			break;

		case T_KA:
		case T_AMBIENT:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_SCALAR);
			mat->ka = CURRENT_SCALAR;
			ADVANCE();
			break;

		case T_KD:
		case T_DIFFUSE:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_SCALAR);
			mat->kd = CURRENT_SCALAR;
			ADVANCE();
			break;

		case T_KS:
		case T_SPECULAR:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_SCALAR);
			mat->ks = CURRENT_SCALAR;
			ADVANCE();
			break;

		case T_POWER:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_SCALAR);
			mat->power = CURRENT_SCALAR;
			ADVANCE();
			break;

		case T_MAP_TRANSFORM:
			ADVANCE(); EXPECT(T_EQUALS);
			ParseMatrix(&mat->map_transform.m[0][0],2,3);
			break;

		case T_INDEX_BASE:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_SCALAR);
			mat->index_base = BrScalarToInt(CURRENT_SCALAR);
			ADVANCE();
			break;

		case T_INDEX_RANGE:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_SCALAR);
			mat->index_range = BrScalarToInt(CURRENT_SCALAR);
			ADVANCE();
			break;

		case T_COLOUR_MAP:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_STRING);
			mat->colour_map = BrMapFind(CURRENT_STRING);
			ADVANCE();
			break;

		case T_SCREEN_DOOR:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_STRING);
			mat->screendoor = BrTableFind(CURRENT_STRING);
			ADVANCE();
			break;

		case T_INDEX_SHADE:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_STRING);
			mat->index_shade = BrTableFind(CURRENT_STRING);
			ADVANCE();
			break;

		case T_INDEX_BLEND:
			ADVANCE(); EXPECT(T_EQUALS); CHECK(T_STRING);
			mat->index_blend = BrTableFind(CURRENT_STRING);
			ADVANCE();
			break;

		default:
			ERROR("Incorrect material member name");
		}
		EXPECT(T_SEMI);
	}

	/*
	 * ];
	 */
	ADVANCE();
	EXPECT(T_SEMI);

	return mat;
};

br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialLoadMany(char *filename,br_material **materials,br_uint_16 num)
{
	int count;

	if(LexAddFile(filename) == 0)
		return 0;

	ADVANCE();

	for (count=0; CURRENT != T_EOF && count<num; count++) {
		materials[count] = ParseMaterial();
	}

	LexCloseFile();

	return count;
}

br_material * BR_PUBLIC_ENTRY BrFmtScriptMaterialLoad(char *filename)
{
	br_material *ptr;

	return (BrFmtScriptMaterialLoadMany(filename,&ptr,1) != 1)?NULL:ptr;
}


