# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: rawtopix.mak 1.1 1995/03/16 12:04:49 sam Exp $
# $Locker:  $
#
# Makefile for Q3D test harness build with WATCOM C32
#
.IMPORT: BASE_DIR EXTENDER MAKE_EXT

TARGET=rawtopix.exe
TARGET_TYPE=EXE
BLD_BASE:=r2p

INCLUDES:=-I$(BASE_DIR)\inc -I$(3DSFTK_DIR)\inc

OBJS_C=\
	$(BLD_DIR)/rawtopix.obj\

EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/brst$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfm$(LIB_TYPE).lib

.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)

