# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: makefile 1.14 1995/08/31 16:29:31 sam Exp $
# $Locker:  $
#
# Makefile for BRender Framework built with Watcom C32
#
.IMPORT: BASE_DIR MAKE_EXT

LIB_PART:=brfw
LIB_INC:=fw
TARGET_TYPE:=LIB

INCLUDES:=-I$(BASE_DIR)\inc

## Base library object files
##
OBJS_C=\
	$(BLD_DIR)/actsupt$(OBJ_EXT)\
	$(BLD_DIR)/angles$(OBJ_EXT)\
	$(BLD_DIR)/brlists$(OBJ_EXT)\
	$(BLD_DIR)/brqsort$(OBJ_EXT)\
	$(BLD_DIR)/bswap$(OBJ_EXT)\
	$(BLD_DIR)/custsupt$(OBJ_EXT)\
	$(BLD_DIR)/datafile$(OBJ_EXT)\
	$(BLD_DIR)/def_mdl$(OBJ_EXT)\
	$(BLD_DIR)/def_mat$(OBJ_EXT)\
	$(BLD_DIR)/diag$(OBJ_EXT)\
	$(BLD_DIR)/envmap$(OBJ_EXT)\
	$(BLD_DIR)/error$(OBJ_EXT)\
	$(BLD_DIR)/ffhooks$(OBJ_EXT)\
	$(BLD_DIR)/file$(OBJ_EXT)\
	$(BLD_DIR)/fileops$(OBJ_EXT)\
	$(BLD_DIR)/fixed$(OBJ_EXT)\
	$(BLD_DIR)/fontptrs$(OBJ_EXT)\
	$(BLD_DIR)/fwsetup$(OBJ_EXT)\
	$(BLD_DIR)/light8$(OBJ_EXT)\
	$(BLD_DIR)/light8o$(OBJ_EXT)\
	$(BLD_DIR)/light24$(OBJ_EXT)\
	$(BLD_DIR)/light24o$(OBJ_EXT)\
	$(BLD_DIR)/logprint$(OBJ_EXT)\
	$(BLD_DIR)/matrix23$(OBJ_EXT)\
	$(BLD_DIR)/matrix34$(OBJ_EXT)\
	$(BLD_DIR)/matrix4$(OBJ_EXT)\
	$(BLD_DIR)/mem$(OBJ_EXT)\
	$(BLD_DIR)/onscreen$(OBJ_EXT)\
	$(BLD_DIR)/pick$(OBJ_EXT)\
	$(BLD_DIR)/pixelmap$(OBJ_EXT)\
	$(BLD_DIR)/pmdsptch$(OBJ_EXT)\
	$(BLD_DIR)/pmgenops$(OBJ_EXT)\
	$(BLD_DIR)/pmmemops$(OBJ_EXT)\
	$(BLD_DIR)/pool$(OBJ_EXT)\
	$(BLD_DIR)/prelight$(OBJ_EXT)\
	$(BLD_DIR)/prepmatl$(OBJ_EXT)\
	$(BLD_DIR)/prepmesh$(OBJ_EXT)\
	$(BLD_DIR)/prepmap$(OBJ_EXT)\
	$(BLD_DIR)/preptab$(OBJ_EXT)\
	$(BLD_DIR)/quat$(OBJ_EXT)\
	$(BLD_DIR)/quantize$(OBJ_EXT)\
	$(BLD_DIR)/register$(OBJ_EXT)\
	$(BLD_DIR)/regsupt$(OBJ_EXT)\
	$(BLD_DIR)/resource$(OBJ_EXT)\
	$(BLD_DIR)/scalar$(OBJ_EXT)\
	$(BLD_DIR)/scale$(OBJ_EXT)\
	$(BLD_DIR)/scratch$(OBJ_EXT)\
	$(BLD_DIR)/scrstr$(OBJ_EXT)\
	$(BLD_DIR)/surface$(OBJ_EXT)\
	$(BLD_DIR)/transfrm$(OBJ_EXT)\
	$(BLD_DIR)/vector$(OBJ_EXT)\

#	$(BLD_DIR)/brexcept$(OBJ_EXT)\
#	$(BLD_DIR)/loader$(OBJ_EXT)\

#	$(BLD_DIR)/defdiag$(OBJ_EXT)
#	$(BLD_DIR)/deffile$(OBJ_EXT)
#	$(BLD_DIR)/defmem$(OBJ_EXT)

OBJS_ASM=\
	$(BLD_DIR)/fixed386$(OBJ_EXT)\
	$(BLD_DIR)/fxadc386$(OBJ_EXT)\
	$(BLD_DIR)/blockops$(OBJ_EXT)\
	$(BLD_DIR)/magicsym$(OBJ_EXT)\
	$(BLD_DIR)/fnt3x5$(OBJ_EXT)\
	$(BLD_DIR)/fntp4x6$(OBJ_EXT)\
	$(BLD_DIR)/fntp7x9$(OBJ_EXT)\
	$(BLD_DIR)/memloops$(OBJ_EXT)\

.IF $(MAKE_EXT) == .icc
# CSet++ version included a DLL import library
#
OBJS_EXTRA=\
	$(BASE_DIR)/os2lock/brsuppt.lib
.ENDIF

.INCLUDE: $(BASE_DIR)/makedefs$(MAKE_EXT)

