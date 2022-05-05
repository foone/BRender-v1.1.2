# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: mkshades.mak 1.2 1995/03/16 12:01:30 sam Exp $
# $Locker:  $
#
.IMPORT: BASE_DIR MAKE_EXT EXTENDER

TARGET=mkshades.exe
TARGET_TYPE=EXE
BLD_BASE:=mks

INCLUDES:=-I$(BASE_DIR)\inc -I$(BASE_DIR)\sys -I$(BASE_DIR)\fw

OBJS_C=\
	$(BLD_DIR)/mkshades$(OBJ_EXT)\

EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE)$(LIB_EXT)\
	$(BASE_DIR)/lib/brst$(LIB_TYPE)$(LIB_EXT)\

.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)

