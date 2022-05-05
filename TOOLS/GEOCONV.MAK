# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: geoconv.mak 1.6 1995/07/28 19:05:15 sam Exp $
# $Locker:  $
#
# Makefile for Q3D test harness build with WATCOM C32
#
.IMPORT: BASE_DIR EXTENDER MAKE_EXT

# Force floating point, stack based
#
TARGET_TYPE=EXE
BASE_TYPE=FLOAT
LIB_SUFFIX=s

TARGET=geoconv.exe
BLD_BASE:=geo
3DSFTK_DIR=t:\3ds_file

INCLUDES:=-I$(BASE_DIR)\inc -I$(3DSFTK_DIR)\inc
EXTRA_CFLAGS=-DSAVE_3DS=1

OBJS_C=\
	$(BLD_DIR)/geoconv.obj\
	$(BLD_DIR)/irit.obj\
	$(BLD_DIR)/save_c.obj\
	$(BLD_DIR)/save3ds.obj\

EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/brst$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfm$(LIB_TYPE).lib\
	$(3DSFTK_DIR)/lib/3dsftkwc.lib\

.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)
