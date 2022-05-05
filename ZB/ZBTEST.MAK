# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: zbtest.mak 1.12 1995/08/31 16:48:09 sam Exp $
# $Locker:  $
#
# Makefile for brender test harness
#
.IMPORT: BASE_DIR BUILD MAKE_EXT EXTENDER
.IMPORT .IGNORE: TARGET_TYPE

TARGET:=zt.exe

.IF $(TARGET_TYPE)X == X
#TARGET_TYPE:=EXE_DOS4GW
#TARGET_TYPE:=EXE_DOS4GWPRO
#TARGET_TYPE:=EXE_PHARLAP
#TARGET_TYPE:=EXE_X32
TARGET_TYPE:=EXE_GO32
.ENDIF

.IF $(MAKE_EXT) == .msc
EXTRA_CFLAGS=-D__PHARLAP386__=1
EXTRA_ASFLAGS=-D__PHARLAP386__=1
.ENDIF

.IF $(MAKE_EXT) == .djg
EXTRA_CFLAGS=-D__GO32__=1
EXTRA_ASFLAGS=-D__GO32__=1
EXTRA_LDFLAGS=-lpc -lm
.ENDIF

BLD_BASE:=zt

INCLUDES:=-I$(BASE_DIR)\inc -I$(BASE_DIR)\dosio -I$(BASE_DIR)\fw -I$(BASE_DIR)\zb

OBJS_C=\
	$(BLD_DIR)/zbtest$(OBJ_EXT)\


#	$(BLD_DIR)/trackmem$(OBJ_EXT)\
#	$(BLD_DIR)/calltrak$(OBJ_EXT)\
#	$(BLD_DIR)/stdmem$(OBJ_EXT)\
#	$(BLD_DIR)/resdbg$(OBJ_EXT)\

#	$(BLD_DIR)/testmats$(OBJ_EXT)\
#	$(BLD_DIR)/trigen$(OBJ_EXT)\

#	$(BLD_DIR)/tritab$(OBJ_EXT)\

OBJS_ASM=\
	$(BLD_DIR)/fnhooks$(OBJ_EXT)\

#	$(BLD_DIR)/ti8_pfz$(OBJ_EXT)\


EXTRA_LDLIBS=\
	$(BASE_DIR)/lib/dio$(LIB_SUFFIX_EXTENDER)$(LIB_SUFFIX_C)$(LIB_SUFFIX_B)$(LIB_SUFFIX)$(LIB_EXT)\
	$(BASE_DIR)/lib/brzb$(LIB_TYPE)$(LIB_EXT)\
	$(BASE_DIR)/lib/brfw$(LIB_TYPE)$(LIB_EXT)\
	$(BASE_DIR)/lib/brst$(LIB_TYPE)$(LIB_EXT)\
	$(BASE_DIR)/lib/brfm$(LIB_TYPE)$(LIB_EXT)\


.INCLUDE: $(BASE_DIR)\makedefs$(MAKE_EXT)

