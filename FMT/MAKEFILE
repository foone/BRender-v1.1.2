# Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
#
# $Id: makefile 1.4 1995/05/30 16:56:43 sam Exp $
# $Locker:  $
#
# Makefile for BRender foreign file formats 
#
.IMPORT: BASE_DIR MAKE_EXT
TARGET_TYPE:=LIB

LIB_PART:=brfm

INCLUDES:=-I$(BASE_DIR)\inc -I$(BASE_DIR)\fw 

OBJS_C=\
	$(BLD_DIR)/loadnff$(OBJ_EXT)\
	$(BLD_DIR)/loadasc$(OBJ_EXT)\
	$(BLD_DIR)/loadscr$(OBJ_EXT)\
	$(BLD_DIR)/loadgif$(OBJ_EXT)\
	$(BLD_DIR)/loadiff$(OBJ_EXT)\
	$(BLD_DIR)/loadbmp$(OBJ_EXT)\
	$(BLD_DIR)/loadtga$(OBJ_EXT)\
	$(BLD_DIR)/savescr$(OBJ_EXT)\

OBJS_ASM=\

.INCLUDE: $(BASE_DIR)/makedefs$(MAKE_EXT)
