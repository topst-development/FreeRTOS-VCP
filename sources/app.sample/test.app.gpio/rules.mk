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

MCU_BSP_APP_SAMPLE_GPIO_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_TEST_APP_GPIO=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_GPIO_TEST_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_GPIO_TEST_PATH)

# Sources
SRCS += gpio_test.c
SRCS += gpio_ctrl.c

