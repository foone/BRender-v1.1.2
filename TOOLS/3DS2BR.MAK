# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: 3ds2br.mak 1.2 1995/05/25 13:38:41 sam Exp $
# $Locker:  $
#
# Makefile for Q3D test harness build with WATCOM C32
#
.IMPORT: BASE_DIR MAKE_EXT EXTENDER

# Force floating point, stack based
#
TARGET_TYPE=EXE
BASE_TYPE=FLOAT
LIB_SUFFIX=s

TARGET=3ds2br.exe
BLD_BASE:=3ds

INCLUDES:=-I$(BASE_DIR)\inc -I$(3DSFTK_DIR)\inc

OBJS_C=\
	$(BLD_DIR)/3ds2br.obj\
	$(BLD_DIR)/bininstr.obj\
	$(BLD_DIR)/parse3ds.obj\

EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/brst$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfm$(LIB_TYPE).lib\

.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)

