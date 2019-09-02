#===========================================================================;
#
#  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
#  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
#  PURPOSE.
#
#  Copyright (c) 1992 - 1998  Microsoft Corporation.  All Rights Reserved.
#
#---------------------------------------------------------------------------;
#
# DirectShow Project Makefile
#
# Targets are provided by DSHOWSDK.MAK
#
# all     Create executables (default).
# clean   Remove all files created by any goal.
#
#===========================================================================;

# Location relative to root of SDK tree.

!ifndef DXMEDIASDK
DXMEDIASDK = ..\..\..\..\..
!endif

# Location relative to master DSHOWSDK sample makefile.

!ifndef DSHOWSAMPLE
DSHOWSAMPLE = ..
!endif


# Target

TARGET_NAME = MMSwitch
TARGET_TYPE = DYNLINK
TARGET_EXT  = ax

# Source files

SRC_FILES = MMSwitch.cpp MMSwitchprop.cpp texttype.cpp
RC_FILE   = MMSwitchprop.rc
DEF_FILE  = MMSwitchIP.def

# DLL specific defines

DLL_BASE  = 0x1d1c0000

# include DSHOWSDK.MAK

!include "$(DSHOWSAMPLE)\dshowsdk.mak"
