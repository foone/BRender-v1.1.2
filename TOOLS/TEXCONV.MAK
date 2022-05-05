# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: texconv.mak 1.5 1995/05/25 13:39:04 sam Exp $
# $Locker:  $
#
# Makefile for Q3D test harness build with WATCOM C32
#
.IMPORT: BASE_DIR MAKE_EXT EXTENDER

TARGET=texconv.exe
TARGET_TYPE=EXE
BLD_BASE:=tex

INCLUDES:=-I$(BASE_DIR)\inc -I$(BASE_DIR)\dosio -I$(BASE_DIR)\fw

OBJS_C=\
	$(BLD_DIR)/texconv$(OBJ_EXT)\

EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE)$(LIB_EXT)\
	$(BASE_DIR)/lib/brfm$(LIB_TYPE)$(LIB_EXT)\
	$(BASE_DIR)/lib/brst$(LIB_TYPE)$(LIB_EXT)\
	$(BASE_DIR)/lib/dio$(LIB_SUFFIX_EXTENDER)$(LIB_SUFFIX_C)$(LIB_SUFFIX_B)$(LIB_SUFFIX)$(LIB_EXT)\

.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)
