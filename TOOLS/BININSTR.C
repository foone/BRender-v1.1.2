/*
 *    Copyright Voxar Limited 1995
 *
 *    bininstr.c : A module for reading numeric types from binary files
 *
 *    $Id: bininstr.c 1.2 1995/05/02 15:51:30 sam Exp $
 *
 *  
 * This module contains functions for reading characters, integers of
 * various sizes and 32-bit IEEE floats from a file. The file is
 * expected to be in little-endian order but the code will run on big
 * or little endian machines. 
 *
 * All this happens to be very useful if you want to read 3D Studio
 * binary files.
 *
 * The purpose of this module is to abstract away file I/O into something
 * small and easily portable.
 *
 */

/*
 *
 *    WARNING:
 * 
 * This code makes certain assumptions about the types available on
 * the target system. If it is your job to port or maintain this code,
 * look at the function ReadFloat.
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <brender.h>

#include "basetype.h"
#include "bininstr.h"



typedef struct BinInStream_t {
    FILE* file;
} BinInStream_t;



    BinInStream_tp
AllocateBinInStream(
) {
    BinInStream_t* stream;

    stream = (BinInStream_t*) malloc (sizeof (BinInStream_t));
    if (stream != NULL) {
	stream->file = NULL;
    }
    return stream;
}


    Bool_t
OpenBinInStream(
    BinInStream_tp stream,
    char           *pathname
) {
    if (stream == NULL) return FALSE;
    if (stream->file != NULL) return FALSE;
    stream->file = fopen(pathname,"rb");
    if (stream->file == NULL) return FALSE;
    return TRUE;
}


    void
CloseBinInStream(
    BinInStream_tp stream
) {
    if (stream == NULL) return;
    if (stream->file == NULL) return;
    fclose(stream->file);
    stream->file = NULL;
}        


    void
DeallocateBinInStream(
    BinInStream_tp stream
) {
    if (stream == NULL) return;
    CloseBinInStream(stream);
    free(stream);
}


    Bool_t
ReadChar(
    BinInStream_tp stream,
    char           *result
) {
    int ch;

    if (stream == NULL) return FALSE; 
    if (stream->file == NULL) return FALSE; 
    ch = fgetc(stream->file);
    if (ch == EOF) return FALSE;
    *result = (char) ch;
    return TRUE;
}


    Bool_t
ReadUInt8(
    BinInStream_tp stream,
    br_uint_8      *result
) {
    Bool_t ok;
    char ch;

    ok = ReadChar(stream,&ch);
    if (!ok) return FALSE;
    *result = (br_uint_8) ch;
    return TRUE;
}


    Bool_t
ReadUInt16(
    BinInStream_tp stream,
    br_uint_16     *result
) {
    Bool_t ok;
    br_uint_8 msb;
    br_uint_8 lsb;

    ok = ReadUInt8(stream,&lsb);
    ok = ok && ReadUInt8(stream,&msb);
    if (!ok) return FALSE;
    *result = (br_uint_16) ((msb << 8) | lsb);
    return TRUE;
}


    Bool_t
ReadUInt32(
    BinInStream_tp stream,
    br_uint_32     *result
) {
    Bool_t ok;
    br_uint_16 msw;
    br_uint_16 lsw;

    ok = ReadUInt16(stream,&lsw);
    ok = ok && ReadUInt16(stream,&msw);
    if (!ok) return FALSE;
    *result = (br_uint_32) ((msw << 16) | lsw);
    return TRUE;
}


    Bool_t
ReadInt8(
    BinInStream_tp stream,
    br_int_8       *result
) {
    Bool_t ok;
    char ch;

    ok = ReadChar(stream,&ch);
    if (!ok) return FALSE;
    *result = (br_int_8) ch;
    return TRUE;
}


    Bool_t
ReadInt16(
    BinInStream_tp stream,
    br_int_16      *result
) {
    Bool_t ok;
    br_uint_16 us;

    ok = ReadUInt16(stream,&us);
    if (!ok) return FALSE;
    *result = (br_int_16) us;
    return TRUE;
}


    Bool_t
ReadInt32(
    BinInStream_tp stream,
    br_int_32      *result
) {
    Bool_t ok;
    br_uint_32 us;

    ok = ReadUInt32(stream,&us);
    if (!ok) return FALSE;
    *result = (br_int_32) us;
    return TRUE;
}


    Bool_t
ReadFloat(
    BinInStream_tp stream,
    Float_t        *result
) {
    /*
     * WARNING!
     *
     * Architecture dependent code:
     *   1: The type "float" must be 32 bits
     *   2: The type "br_uint_32" must be 32 bits
     *   3: Your machine must store floats in IEEE format
     *
     * Having made these assumptions, this function will work:
     *   1: Whether your machine is big or little endian
     *   2: Whatever size you have defined Float_t to be
     */

    Bool_t ok;
    float f;
    float* fp;
    br_uint_32 ui;

    ok = ReadUInt32 (stream, &ui);
    if (!ok) return FALSE;
    fp = (float*) (&ui);
    f = *fp;
    *result = (Float_t) f; 
    return TRUE;
}


    Bool_t
SkipBytes(
    BinInStream_tp stream,
    Int_t          n_bytes
) {
    int ch;
    Int_t i;

    if (stream == NULL) return FALSE;
    if (stream->file == NULL) return FALSE;
    for (i=0; i<n_bytes; i++) {
	ch = fgetc(stream->file);
	if (ch == EOF) return FALSE;
    }
    return TRUE;
}        

#undef BUFFER_SIZE
