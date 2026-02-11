// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : main.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_BASE == 1 )

#include <main.h>

#include <sal_api.h>
#include <app_cfg.h>
#include <debug.h>
#include <bsp.h>

#if (APLT_LINUX_SUPPORT_SPI_DEMO == 1)
    #include <spi_eccp.h>
#endif
#if (APLT_LINUX_SUPPORT_POWER_CTRL == 1)
    #include <power_app.h>
#endif
#if ( MCU_BSP_SUPPORT_APP_KEY == 1)
    #include <key.h>
#endif  // ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#if ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )
    #include <console.h>
#endif  // ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
	#include <FreeRTOS.h>
	#include <event_groups.h>
	#include <queue.h>
	#include <task.h>
    #include <can_demo.h>
	#include <gpio_ctrl.h>
	#include <pdm_ctrl.h>
	#include <i2c_ctrl.h>
#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

#if ( MCU_BSP_SUPPORT_APP_IDLE == 1 )
    #include <idle.h>
#endif  // ( MCU_BSP_SUPPORT_APP_IDLE == 1 )

#if ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )
    #include <spi_led.h>
#endif  // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
    #include "fwupdate.h"
#elif ( MCU_BSP_SUPPORT_APP_FW_UPDATE_ECCP == 1 )
    #include "fwupdate.h"
#endif

/*
***************************************************************************************************
*                                         GLOBAL VARIABLES
***************************************************************************************************
*/
uint32                                  gALiveMsgOnOff;
static uint32                           gALiveCount;

/* VCP Multi-Tasking Resources */
EventGroupHandle_t xVcpEventGroup = NULL;
QueueHandle_t      xCanQueue      = NULL;
VcpMessage_t       g_vcp_shared_buf;

#define VCP_DATA_READY_BIT (1 << 0)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void VcpDispatcherTask
(
	void *pvParameters
);

static void VCP_CreateApp
(
	void
);

static void Main_StartTask
(
    void *                              pArg
);

static void AppTaskCreate
(
    void
);

static void DisplayAliveLog
(
    void
);

static void DisplayOTPInfo
(
    void
);


/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          cmain
*
* This is the standard entry point for C code.
*
* Notes
*   It is assumed that your code will call main() once you have performed all necessary
*   initialization.
*
***************************************************************************************************
*/
void cmain (void)
{
    static uint32           AppTaskStartID = 0;
    static uint32           AppTaskStartStk[ACFG_TASK_MEDIUM_STK_SIZE];
    SALRetCode_t            err;
    SALMcuVersionInfo_t     versionInfo = {0,0,0,0};

    (void)SAL_Init();

    BSP_PreInit(); /* Initialize basic BSP functions */

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
    (void)CAN_DemoInitialize();
#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

    BSP_Init(); /* Initialize BSP functions */

    (void)SAL_GetVersion(&versionInfo);
    mcu_printf("\n===============================\n");
    mcu_printf("    MCU BSP Version: V%d.%d.%d\n",
           versionInfo.viMajorVersion,
           versionInfo.viMinorVersion,
           versionInfo.viPatchVersion);
    mcu_printf("-------------------------------\n");
    DisplayOTPInfo();
    mcu_printf("===============================\n\n");

    // create the first app task...
    err = (SALRetCode_t)SAL_TaskCreate(&AppTaskStartID,
                         (const uint8 *)"App Task Start",
                         (SALTaskFunc) &Main_StartTask,
                         &AppTaskStartStk[0],
                         ACFG_TASK_MEDIUM_STK_SIZE,
                         SAL_PRIO_APP_CFG,
                         NULL);

    if (err == SAL_RET_SUCCESS)
    {
        // start woring os.... never return from this function
        (void)SAL_OsStart();
    }
}

/*
***************************************************************************************************
*                                          Main_StartTask
*
* This is an example of a startup task.
*
* Notes
*   As mentioned in the book's text, you MUST initialize the ticker only once multitasking has
*   started.
*
*   1) The first line of code is used to prevent a compiler warning because 'pArg' is not used.
*      The compiler should not generate any code for this statement.
*
***************************************************************************************************
*/
static void Main_StartTask(void * pArg)
{
    (void)pArg;
    (void)SAL_OsInitFuncs();

    /* Service Init*/
	MotorPWM_Init();
	LCDSensor_Init();

    /* Create application tasks */
    AppTaskCreate();

	//while (1)
    //{  /* Task body, always written as an infinite loop.       */
    //    DisplayAliveLog();
    //    //mcu_printf("\n MCU Idle !!!");
    //    (void)SAL_TaskSleep(5000);
    //}
}

static void AppTaskCreate(void)
{
#if (APLT_LINUX_SUPPORT_SPI_DEMO == 1)
    ECCP_InitSPIManager();
#endif  
#if (APLT_LINUX_SUPPORT_POWER_CTRL == 1)
    POWER_APP_StartDemo();
#endif

  
#if ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )
    CreateConsoleTask();
#endif  // ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )

#if ( MCU_BSP_SUPPORT_APP_KEY == 1 )
    KEY_AppCreate();
#endif  // ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
    CAN_DemoCreateApp();
	VCP_CreateApp();
#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
    CreateFWUDTask();
#elif ( MCU_BSP_SUPPORT_APP_FW_UPDATE_ECCP == 1 )
    CreateFWUDTask();
#endif

#if ( MCU_BSP_SUPPORT_APP_IDLE == 1 )
    IDLE_CreateTask();
#endif  // ( MCU_BSP_SUPPORT_APP_IDLE == 1 )

#if ( MCU_BSP_SUPPORT_APP_SPI_LED == 1)
    SPILED_CreateAppTask();
#endif  // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

}

static void DisplayAliveLog(void)
{
    if (gALiveMsgOnOff != 0U)
    {
        mcu_printf("\n %d", gALiveCount);

        gALiveCount++;

        if(gALiveCount >= MAIN_UINT_MAX_NUM)
        {
            gALiveCount = 0;
        }
    }
    else
    {
        gALiveCount = 0;
    }
}

#define LDT1_AREA_ADDR  0xA1011800U
#define PMU_REG_ADDR    0xA0F28000U

static void DisplayOTPInfo(void)
{
    volatile uint32 *ldt1Addr;
    volatile uint32 *chipNameAddr;
    volatile uint32 *remapAddr;
    volatile uint32 *hsmStatusAddr;
    uint32          chipName = 0;
    uint32          dualBankVal = 0;
    uint32          dual_bank = 0;
    uint32          expandFlashVal = 0;
    uint32          expand_flash = 0;
    uint32          remap_mode = 0;
    uint32          hsm_ready = 0;

    //----------------------------------------------------------------
    // OTP LDT1 Read
    // [11:0]Dual_Bank_Selection, [59:48]EXPAND_FLASH
    // Dual_Bank_Sel: [0xC0][11: 0] & [0xD0][11: 0] & [0xE0][11: 0] & [0xF0][11: 0]
    // EXPAND_FLASH : [0xC4][27:16] & [0xD4][27:16] & [0xE4][27:16] & [0xF4][27:16]
    // HwMC_PRG_FLS_LDT1: 0xA1011800

    ldt1Addr = (volatile uint32 *)(LDT1_AREA_ADDR + 0x00C0);
    chipNameAddr = (volatile uint32 *)(LDT1_AREA_ADDR + 0x0300);
    remapAddr = (volatile uint32 *)(PMU_REG_ADDR);
    hsmStatusAddr = (volatile uint32 *)(PMU_REG_ADDR + 0x0020);

    chipName = *chipNameAddr;
    chipName &= 0x000FFFFF;

    dualBankVal = ldt1Addr[ 0];
    expandFlashVal = ldt1Addr[ 1];

    dualBankVal &= ldt1Addr[ 4];
    expandFlashVal &= ldt1Addr[ 5];

    dualBankVal &= ldt1Addr[ 8];
    expandFlashVal &= ldt1Addr[ 9];

    dualBankVal &= ldt1Addr[12];
    expandFlashVal &= ldt1Addr[13];

    dualBankVal = (dualBankVal >> 0) & 0x0FFF;
    expandFlashVal  = (expandFlashVal >> 16) & 0x0FFF;

    dual_bank = (dualBankVal == 0x0FFF) ? 0 : 1;            // (single_bank : dual_bank)
    expand_flash  = (expandFlashVal  == 0x0000) ? 0 : 1;    // (only_eFlash : use_extSNOR)

    remap_mode = remapAddr[ 0];

    mcu_printf("    CHIP   NAME  : %x\n",    chipName);
    mcu_printf("    DUAL   BANK  : %d\n",    dual_bank);
    mcu_printf("    EXPAND FLASH : %d\n",    expand_flash);
    mcu_printf("    REMAP  MODE  : %d\n",    (remap_mode >> 16));

    hsm_ready = hsmStatusAddr[ 0];
    hsm_ready = (hsm_ready >> 2) & 0x0001;
#if 0
    if(hsm_ready)
    {
        mcu_printf("    HSM    READY : %d\n",    hsm_ready);
    }
    else
    {
        while(hsm_ready != 1)
        {
            mcu_printf("    HSM    READY : %d\n",    hsm_ready);
            mcu_printf("    wait...\n");
            hsm_ready = (hsm_ready >> 2) & 0x0001;
        }
    }
#else
    mcu_printf("    HSM    READY : %d\n",    hsm_ready);
#endif
}

/* VCP Task Stack Size Defines */
#define VCP_DISP_STK_SIZE   (256)
#define VCP_CTRL_STK_SIZE   (256)
#define VCP_LCD_STK_SIZE    (512) // LCD/snprintf 사용으로 넉넉하게

void VcpDispatcherTask(void *pvParameters) 
{
    VcpMessage_t rxMsg;
    (void)pvParameters;

    for (;;) 
    {
        /* 1. 큐에서 데이터가 들어올 때까지 무한 대기 (Blocked) */
        if (xQueueReceive(xCanQueue, &rxMsg, portMAX_DELAY) == pdPASS) 
        {
            /* 2. 데이터를 공유 버퍼(Global)에 복사 */
            g_vcp_shared_buf = rxMsg;

            /* 3. 모든 Task 깨우기 (Event Bit 설정) */
            xEventGroupSetBits(xVcpEventGroup, VCP_DATA_READY_BIT);

            /* 4. 소비자 Task들이 데이터를 읽을 시간을 줌 */
            vTaskDelay(pdMS_TO_TICKS(5));

            /* 5. 다음 메시지를 위해 비트 초기화 (Clear) */
            xEventGroupClearBits(xVcpEventGroup, VCP_DATA_READY_BIT);
        }
    }
}

void VCP_CreateApp(void)
{
    /* 1. RTOS 자원(Queue, EventGroup) 생성 */
    if (xVcpEventGroup == NULL) {
        xVcpEventGroup = xEventGroupCreate();
    }
    if (xCanQueue == NULL) {
        xCanQueue = xQueueCreate(20, sizeof(VcpMessage_t));
    }

    /* 2. Task ID 및 Stack 메모리 정적 선언 */
    // (1) Dispatcher Task
    static uint32 uiDispID;
    static uint32 uiDispStk[VCP_DISP_STK_SIZE];

    // (2) Brake Task
    static uint32 uiBrakeID;
    static uint32 uiBrakeStk[VCP_CTRL_STK_SIZE];

    // (3) Motor Speed Task
    static uint32 uiSpeedID;
    static uint32 uiSpeedStk[VCP_CTRL_STK_SIZE];

    // (4) Motor Wheel Task
    static uint32 uiWheelID;
    static uint32 uiWheelStk[VCP_CTRL_STK_SIZE];

    // (5) Emergency Task
    static uint32 uiEmerID;
    static uint32 uiEmerStk[VCP_CTRL_STK_SIZE];

    // (6) Turn Signal Task
    static uint32 uiTurnID;
    static uint32 uiTurnStk[VCP_CTRL_STK_SIZE];

    // (7) Head Light Task
    static uint32 uiHeadID;
    static uint32 uiHeadStk[VCP_CTRL_STK_SIZE];

    // (8) Fuel Level Task
    static uint32 uiFuelID;
    static uint32 uiFuelStk[VCP_LCD_STK_SIZE];


    /* 3. SAL_TaskCreate를 이용한 Task 생성 */

    // [Dispatcher]
    (void)SAL_TaskCreate(&uiDispID, (const uint8 *)"VCP Disp", (SALTaskFunc)&VcpDispatcherTask,
                         &uiDispStk[0], VCP_DISP_STK_SIZE, SAL_PRIO_APP_CFG, NULL);

    // [Brake]
    (void)SAL_TaskCreate(&uiBrakeID, (const uint8 *)"VCP Brake", (SALTaskFunc)&BrakeLightTask,
                         &uiBrakeStk[0], VCP_CTRL_STK_SIZE, SAL_PRIO_APP_CFG, NULL);

    // [Motor Speed]
    (void)SAL_TaskCreate(&uiSpeedID, (const uint8 *)"VCP Speed", (SALTaskFunc)&MotorSpeedTask,
                         &uiSpeedStk[0], VCP_CTRL_STK_SIZE, SAL_PRIO_APP_CFG, NULL);

    // [Motor Wheel]
    (void)SAL_TaskCreate(&uiWheelID, (const uint8 *)"VCP Wheel", (SALTaskFunc)&MotorWheelTask,
                         &uiWheelStk[0], VCP_CTRL_STK_SIZE, SAL_PRIO_APP_CFG, NULL);

    // [Emergency]
    (void)SAL_TaskCreate(&uiEmerID, (const uint8 *)"VCP Emer", (SALTaskFunc)&EmergencySignalTask,
                         &uiEmerStk[0], VCP_CTRL_STK_SIZE, SAL_PRIO_APP_CFG, NULL);

    // [Turn Signal]
    (void)SAL_TaskCreate(&uiTurnID, (const uint8 *)"VCP Turn", (SALTaskFunc)&TurnSignalTask,
                         &uiTurnStk[0], VCP_CTRL_STK_SIZE, SAL_PRIO_APP_CFG, NULL);

    // [Head Light]
    (void)SAL_TaskCreate(&uiHeadID, (const uint8 *)"VCP Head", (SALTaskFunc)&HeadLightTask,
                         &uiHeadStk[0], VCP_CTRL_STK_SIZE, SAL_PRIO_APP_CFG, NULL);

    // [Fuel Level]
    (void)SAL_TaskCreate(&uiFuelID, (const uint8 *)"VCP Fuel", (SALTaskFunc)&FuelLevelTask,
                         &uiFuelStk[0], VCP_LCD_STK_SIZE, SAL_PRIO_APP_CFG, NULL);
}

#endif  // ( MCU_BSP_SUPPORT_APP_BASE == 1 )

