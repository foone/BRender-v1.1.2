# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: brview.mak 1.1 1995/03/22 18:06:13 sam Exp $
# $Locker:  $
#
# Makefile for Q3D test harness build with WATCOM C32
#
.IMPORT: BASE_DIR MAKE_EXT EXTENDER

# Force floating point, stack based
#
TARGET=brview.exe
TARGET_TYPE=EXE
BLD_BASE:=brv

INCLUDES:=-I$(BASE_DIR)\inc -I$(BASE_DIR)\dosio

.IF $(EXTENDER) == DOS4GW
EXTRA_LDFLAGS=option stub=$(BASE_DIR:s,\,\\,)\\bin\\brwstub.exe
.ENDIF

OBJS_C=\
	$(BLD_DIR)/brview.obj\

EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/dio$(LIB_SUFFIX_EXTENDER)$(LIB_SUFFIX_C)$(LIB_SUFFIX_B)$(LIB_SUFFIX)$(LIB_EXT)\
	$(BASE_DIR)/lib/brst$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brfm$(LIB_TYPE).lib\
	$(BASE_DIR)/lib/brzb$(LIB_TYPE).lib\

.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)

