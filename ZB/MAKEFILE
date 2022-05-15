# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: makefile 1.9 1995/08/31 16:47:34 sam Exp $
# $Locker:  $
#
# Makefile for Brender Z-Buffer renderer
#
.IMPORT: BASE_DIR MAKE_EXT
.IMPORT .IGNORE: TARGET_TYPE

LIB_PART:=brzb
LIB_INC:=zb
TARGET_TYPE:=LIB

INCLUDES:=-I$(BASE_DIR)\inc -I$(BASE_DIR)\fw

OBJS_C=\
	$(BLD_DIR)/awtmz$(OBJ_EXT)\
	$(BLD_DIR)/bbox$(OBJ_EXT)\
	$(BLD_DIR)/decalz$(OBJ_EXT)\
	$(BLD_DIR)/dither$(OBJ_EXT)\
	$(BLD_DIR)/frcp$(OBJ_EXT)\
	$(BLD_DIR)/l_piz$(OBJ_EXT)\
	$(BLD_DIR)/p_piz$(OBJ_EXT)\
	$(BLD_DIR)/perspz$(OBJ_EXT)\
	$(BLD_DIR)/zbclip$(OBJ_EXT)\
	$(BLD_DIR)/zbmatl$(OBJ_EXT)\
	$(BLD_DIR)/zbmesh$(OBJ_EXT)\
	$(BLD_DIR)/zbmeshe$(OBJ_EXT)\
	$(BLD_DIR)/zbmeshp$(OBJ_EXT)\
	$(BLD_DIR)/zbrendr$(OBJ_EXT)\
	$(BLD_DIR)/zbsetup$(OBJ_EXT)\

OBJS_ASM=\
	$(BLD_DIR)/mesh386$(OBJ_EXT)\
	$(BLD_DIR)/safediv$(OBJ_EXT)\
	$(BLD_DIR)/sar16$(OBJ_EXT)\
	$(BLD_DIR)/t_piza$(OBJ_EXT)\
	$(BLD_DIR)/t_piza2$(OBJ_EXT)\
	$(BLD_DIR)/ti8_piz$(OBJ_EXT)\
	$(BLD_DIR)/ti8_pizp$(OBJ_EXT)\
	$(BLD_DIR)/tt15_piz$(OBJ_EXT)\
	$(BLD_DIR)/tt24_piz$(OBJ_EXT)\

.INCLUDE: $(BASE_DIR)/makedefs$(MAKE_EXT)


