# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : ruls.mk
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

MCU_BSP_APP_SAMPLE_CAN_DEMO_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_CAN_DEMO=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_CAN_DEMO_PATH)

# Include
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_CAN_DEMO_PATH)

# Sources
SRCS += can_demo.c
SRCS += can_vcp_ctrl.c
