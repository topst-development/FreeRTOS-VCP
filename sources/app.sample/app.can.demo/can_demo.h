// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_demo.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_CAN_DEMO_HEADER
#define MCU_BSP_CAN_DEMO_HEADER

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <can_config.h>
#include <can.h>


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

#define CAN_DEMO_TASK_STK_SIZE          (2048)
#define CAN_MAX_TEST_MSG_NUM            (9UL)

//#define CAN_DEMO_RESPONSE_TEST          //for CAN response test

typedef struct CANDemoTestInfo
{
    uint8                               tiRecv;
    uint8                               tiSendRecv;
} CANDemoTestInfo_t;

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

sint32 CAN_DemoInitialize
(
    void
);

void CAN_DemoTest
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

void CAN_DemoCreateApp
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

#endif  // MCU_BSP_CAN_DEMO_HEADER

