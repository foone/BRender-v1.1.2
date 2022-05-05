/*
 *    Copyright Voxar Limited 1995
 *
 *    bininstr.h : provides a means of reading binary files.
 *
 *    $Id: bininstr.h 1.2 1995/05/02 15:51:31 sam Exp $
 *
 */

#ifndef _BININSTR_H_
#define _BININSTR_H_


typedef struct BinInStream_t* BinInStream_tp;


extern
    BinInStream_tp
AllocateBinInStream(
);


extern
    Bool_t
OpenBinInStream(
    BinInStream_tp stream,
    char          *pathname
);


extern
    Bool_t
ReadChar(
    BinInStream_tp stream,
    char           *result
);


extern
    Bool_t
ReadUInt8(
    BinInStream_tp stream,
    br_uint_8      *result
);


extern
    Bool_t
ReadUInt16(
    BinInStream_tp stream,
    br_uint_16     *result
);


extern
    Bool_t
ReadUInt32(
    BinInStream_tp stream,
    br_uint_32     *result
);


extern
    Bool_t
ReadInt8(
    BinInStream_tp stream,
    br_int_8       *result
);


extern
    Bool_t
ReadInt16(
    BinInStream_tp stream,
    br_int_16      *result
);


extern
    Bool_t
ReadInt32(
    BinInStream_tp stream,
    br_int_32      *result
);


extern
    Bool_t
ReadFloat(
    BinInStream_tp stream,
    Float_t        *result
);


extern
    Bool_t
SkipBytes(
    BinInStream_tp stream,
    Int_t          n_bytes
);


extern
    void
CloseBinInStream(
    BinInStream_tp stream
);


extern
    void
DeallocateBinInStream(
    BinInStream_tp stream
);


#endif

