# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: mat2txt.mak 1.1 1995/08/31 16:43:26 sam Exp $
# $Locker:  $
#
.IMPORT: BASE_DIR MAKE_EXT EXTENDER

TARGET=mat2txt.exe
TARGET_TYPE=EXE
BLD_BASE:=mtt

INCLUDES:=-I$(BASE_DIR)\inc -I$(BASE_DIR)\fw

OBJS_C=\
	$(BLD_DIR)/mat2txt$(OBJ_EXT)\

EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE)$(LIB_EXT)\
	$(BASE_DIR)/lib/brst$(LIB_TYPE)$(LIB_EXT)\

.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)


