# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: ifftopix.mak 1.1 1995/03/16 12:04:42 sam Exp $
# $Locker:  $
#
# Makefile for Q3D test harness build with WATCOM C32
#
.IMPORT: BASE_DIR EXTENDER MAKE_EXT

TARGET=ifftopix.exe
TARGET_TYPE=EXE
BLD_BASE:=i2p

INCLUDES:=-I$(BASE_DIR)\inc

OBJS_C=\
	$(BLD_DIR)/ifftopix.obj\
	$(BLD_DIR)/loadiff.obj\
	$(BLD_DIR)/bitmap.obj\

EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/brst$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE).lib\
	p:\tech\lib\wargstdr.lib

.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)

