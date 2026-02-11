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

MCU_BSP_APP_SAMPLE_PDM_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_TEST_APP_PDM=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_PDM_TEST_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_PDM_TEST_PATH)

# Sources
SRCS += pdm_test.c
SRCS += pdm_ctrl.c

