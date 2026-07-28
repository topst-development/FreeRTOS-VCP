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
#include <gic.h>
#include <gpio.h>
#include <pdm.h>
#include <uart.h>

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
    #include <can_demo.h>
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
#define APP_DEMO_TB6612                 (1)
#define APP_DEMO_HCSR04                 (0)

#if (APP_DEMO_TB6612 == 1)
/*
 * VCP-G Arduino header pin mapping:
 *   D4  / GPIO_B27 / PDM channel 7 -> TB6612 PWMA
 *   D2  / GPIO_B28 / PDM channel 8 -> TB6612 PWMB
 *   D7  / GPIO_B1                -> TB6612 AIN1
 *   D8  / GPIO_B0                -> TB6612 AIN2
 *   D12 / GPIO_C15               -> TB6612 BIN1
 *   D13 / GPIO_C12               -> TB6612 BIN2
 *   D6  / GPIO_A13               -> TB6612 STBY
 *   D10 / GPIO_C13               <- Motor A encoder phase A
 *   D11 / GPIO_C14               <- Motor A encoder phase B
 */
#define TB6612_PWMA_CHANNEL              ((uint32)PDM_CHANNEL_7)
#define TB6612_PWMB_CHANNEL              ((uint32)PDM_CHANNEL_8)
#define TB6612_PWM_PORT                  (GPIO_PERICH_CH1)

#define TB6612_AIN1_PIN                  (GPIO_GPB(1UL))
#define TB6612_AIN2_PIN                  (GPIO_GPB(0UL))
#define TB6612_BIN1_PIN                  (GPIO_GPC(15UL))
#define TB6612_BIN2_PIN                  (GPIO_GPC(12UL))
#define TB6612_STBY_PIN                  (GPIO_GPA(13UL))

#define TB6612_PWM_FREQUENCY_HZ          (20000UL)
#define TB6612_PWM_PERIOD_NS             (50000UL)
#define TB6612_PDM_STOP_TIMEOUT_MS       (100UL)
#define TB6612_DIRECTION_CHANGE_DELAY_MS (100UL)

#define TB6612_DIR_COAST                 (0UL)
#define TB6612_DIR_FORWARD               (1UL)
#define TB6612_DIR_REVERSE               (2UL)
#define TB6612_DIR_BRAKE                 (3UL)

#define ENCODER_A_PIN                    (GPIO_GPC(13UL))
#define ENCODER_B_PIN                    (GPIO_GPC(14UL))
#define ENCODER_A_IRQ                    ((uint32)GIC_EXT4)
#define ENCODER_SAMPLE_PERIOD_MS         (500UL)
/* Set this to phase-A rising edges per output-shaft revolution. */
#define ENCODER_PULSES_PER_REV           (960UL)

static PDMModeConfig_t                   gTB6612PwmAConfig;
static PDMModeConfig_t                   gTB6612PwmBConfig;
static uint32                            gTB6612DutyPercent;
static uint32                            gTB6612Direction;
static volatile sint32                   gEncoderCount;
static sint32                            gEncoderLastCount;
static sint32                            gEncoderLastDelta;
static sint32                            gEncoderRpm;
static uint32                            gEncoderLastSampleTick;
#endif

#if (APP_DEMO_HCSR04 == 1)
#define HCSR04_TRIG_PIN                 (GPIO_GPB(27UL))
#define HCSR04_ECHO_PIN                 (GPIO_GPB(28UL))
#define HCSR04_ECHO_TIMEOUT_US          (30000UL)
#define HCSR04_DELAY_LOOPS_PER_US       (28UL)
#define HCSR04_INVALID_PULSE_US         (0xFFFFFFFFUL)
#endif

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

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

#if (APP_DEMO_HCSR04 == 1)
static void HCSR04_DelayUs
(
    uint32                              uiUs
);

static uint32 HCSR04_MeasurePulseUs
(
    void
);
#endif
#if (APP_DEMO_TB6612 == 1)
static SALRetCode_t TB6612_Init
(
    void
);

static SALRetCode_t TB6612_SetDuty
(
    uint32                              uiDutyPercent
);

static SALRetCode_t TB6612_SetDirection
(
    uint32                              uiDirection
);

static void TB6612_PrintDebug
(
    void
);

static void TB6612_PrintHelp
(
    void
);

static void TB6612_ProcessUartCommand
(
    sint32                              iCommand
);
static void Encoder_AIsr(void * pArg);
static SALRetCode_t Encoder_Init(void);
static void Encoder_Service(void);
static void Encoder_Print(void);
#endif


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
#if (APP_DEMO_TB6612 == 1)
    sint8        cUartError;
    sint32       iUartChar;
    SALRetCode_t ret;
    SALRetCode_t encoderRet;

    (void)pArg;
    (void)SAL_OsInitFuncs();

    ret = TB6612_Init();
    encoderRet = Encoder_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        TB6612_PrintHelp();
        TB6612_PrintDebug();
    }
    else
    {
        mcu_printf("[TB6612][ERROR] initialization failed; STBY remains LOW\n");
    }

    if (encoderRet != SAL_RET_SUCCESS)
    {
        mcu_printf("[ENCODER][ERROR] initialization failed\n");
    }

    while (1)
    {
        cUartError = 0;
        iUartChar = UART_GetChar((uint8)UART_DEBUG_CH, 0, &cUartError);

        if ((cUartError == 0) && (iUartChar >= 0))
        {
            TB6612_ProcessUartCommand(iUartChar);
        }

        if (encoderRet == SAL_RET_SUCCESS)
        {
            Encoder_Service();
        }

        (void)SAL_TaskSleep(10UL);
    }
#else
    uint32 pulseUs;
    uint32 distanceMm;

    (void)pArg;
    (void)SAL_OsInitFuncs();

    (void)GPIO_Config(HCSR04_TRIG_PIN,
                      (uint32)(GPIO_FUNC(0UL) | GPIO_OUTPUT | GPIO_NOPULL));
    (void)GPIO_Config(HCSR04_ECHO_PIN,
                      (uint32)(GPIO_FUNC(0UL) | GPIO_INPUT |
                               GPIO_INPUTBUF_EN | GPIO_PULLDN));
    (void)GPIO_Set(HCSR04_TRIG_PIN, 0UL);
    (void)SAL_TaskSleep(100UL);

    mcu_printf("[HC-SR04] TRIG=D4(GPIO_B27), ECHO=D2(GPIO_B28)\r\n");

    while (1)
    {
        pulseUs = HCSR04_MeasurePulseUs();

        if (pulseUs == HCSR04_INVALID_PULSE_US)
        {
            mcu_printf("[HC-SR04] timeout - check wiring\r\n");
        }
        else
        {
            distanceMm = ((pulseUs * 10UL) + 29UL) / 58UL;
            mcu_printf("[HC-SR04] echo=%d us, distance=%d.%d cm\r\n",
                       pulseUs,
                       distanceMm / 10UL,
                       distanceMm % 10UL);
        }

        (void)SAL_TaskSleep(500UL);
    }
#endif
}

#if (APP_DEMO_HCSR04 == 1)
static void HCSR04_DelayUs(uint32 uiUs)
{
    uint32 count;
    uint32 i;

    count = uiUs * HCSR04_DELAY_LOOPS_PER_US;

    for (i = 0UL; i < count; i++)
    {
        BSP_NOP_DELAY();
    }
}

static uint32 HCSR04_MeasurePulseUs(void)
{
    uint32 timeoutUs;
    uint32 pulseUs;

    (void)GPIO_Set(HCSR04_TRIG_PIN, 0UL);
    HCSR04_DelayUs(2UL);
    (void)GPIO_Set(HCSR04_TRIG_PIN, 1UL);
    HCSR04_DelayUs(20UL);
    (void)GPIO_Set(HCSR04_TRIG_PIN, 0UL);

    timeoutUs = HCSR04_ECHO_TIMEOUT_US;
    while ((GPIO_Get(HCSR04_ECHO_PIN) == 0U) && (timeoutUs > 0UL))
    {
        HCSR04_DelayUs(1UL);
        timeoutUs--;
    }

    if (timeoutUs == 0UL)
    {
        return HCSR04_INVALID_PULSE_US;
    }

    pulseUs = 0UL;
    while ((GPIO_Get(HCSR04_ECHO_PIN) != 0U) &&
           (pulseUs < HCSR04_ECHO_TIMEOUT_US))
    {
        HCSR04_DelayUs(1UL);
        pulseUs++;
    }

    if (pulseUs >= HCSR04_ECHO_TIMEOUT_US)
    {
        return HCSR04_INVALID_PULSE_US;
    }

    return pulseUs;
}
#endif

#if (APP_DEMO_TB6612 == 1)
static SALRetCode_t TB6612_ApplyDuty
(
    uint32                              uiChannel,
    PDMModeConfig_t *                   psConfig,
    uint32                              uiDutyPercent
)
{
    uint32       uiTimeout;
    SALRetCode_t ret;

    (void)PDM_Disable(uiChannel, PMM_ON);

    uiTimeout = 0UL;
    while (PDM_GetChannelStatus(uiChannel) != 0UL)
    {
        (void)SAL_TaskSleep(1UL);
        uiTimeout++;

        if (uiTimeout >= TB6612_PDM_STOP_TIMEOUT_MS)
        {
            mcu_printf("[TB6612][ERROR] PDM%d disable timeout\n", uiChannel);
            return SAL_RET_FAILED;
        }
    }

    psConfig->mcDutyNanoSec1 =
        (TB6612_PWM_PERIOD_NS * uiDutyPercent) / 100UL;
    ret = PDM_SetConfig(uiChannel, psConfig);

    if (ret == SAL_RET_SUCCESS)
    {
        ret = PDM_Enable(uiChannel, PMM_ON);
    }

    return ret;
}

static SALRetCode_t TB6612_Init(void)
{
    uint32       uiGpioOutput;
    SALRetCode_t retA;
    SALRetCode_t retB;

    uiGpioOutput = (uint32)(GPIO_FUNC(0UL) | GPIO_OUTPUT | GPIO_NOPULL);

    (void)GPIO_Config(TB6612_STBY_PIN, uiGpioOutput);
    (void)GPIO_Config(TB6612_AIN1_PIN, uiGpioOutput);
    (void)GPIO_Config(TB6612_AIN2_PIN, uiGpioOutput);
    (void)GPIO_Config(TB6612_BIN1_PIN, uiGpioOutput);
    (void)GPIO_Config(TB6612_BIN2_PIN, uiGpioOutput);

    /* Safe startup: disable driver and select coast before starting PDM. */
    (void)GPIO_Set(TB6612_STBY_PIN, 0UL);
    (void)GPIO_Set(TB6612_AIN1_PIN, 0UL);
    (void)GPIO_Set(TB6612_AIN2_PIN, 0UL);
    (void)GPIO_Set(TB6612_BIN1_PIN, 0UL);
    (void)GPIO_Set(TB6612_BIN2_PIN, 0UL);

    (void)PDM_Init();

    gTB6612PwmAConfig.mcOperationMode  = PDM_OUTPUT_MODE_PHASE_1;
    gTB6612PwmAConfig.mcPortNumber     = TB6612_PWM_PORT;
    gTB6612PwmAConfig.mcPeriodNanoSec1 = TB6612_PWM_PERIOD_NS;

    gTB6612PwmBConfig.mcOperationMode  = PDM_OUTPUT_MODE_PHASE_1;
    gTB6612PwmBConfig.mcPortNumber     = TB6612_PWM_PORT;
    gTB6612PwmBConfig.mcPeriodNanoSec1 = TB6612_PWM_PERIOD_NS;

    retA = TB6612_ApplyDuty(
        TB6612_PWMA_CHANNEL, &gTB6612PwmAConfig, 0UL);
    retB = TB6612_ApplyDuty(
        TB6612_PWMB_CHANNEL, &gTB6612PwmBConfig, 0UL);

    gTB6612DutyPercent = 0UL;
    gTB6612Direction   = TB6612_DIR_COAST;

    if ((retA != SAL_RET_SUCCESS) || (retB != SAL_RET_SUCCESS))
    {
        mcu_printf("[TB6612][ERROR] PDM initialization failed\n");
        return SAL_RET_FAILED;
    }

    (void)GPIO_Set(TB6612_STBY_PIN, 1UL);
    mcu_printf("[TB6612] safe start: STBY=1, COAST, duty=0%%\n");

    return SAL_RET_SUCCESS;
}

static SALRetCode_t TB6612_SetDuty(uint32 uiDutyPercent)
{
    SALRetCode_t retA;
    SALRetCode_t retB;

    if (uiDutyPercent > 100UL)
    {
        uiDutyPercent = 100UL;
    }

    retA = TB6612_ApplyDuty(
        TB6612_PWMA_CHANNEL, &gTB6612PwmAConfig, uiDutyPercent);
    retB = TB6612_ApplyDuty(
        TB6612_PWMB_CHANNEL, &gTB6612PwmBConfig, uiDutyPercent);

    if ((retA == SAL_RET_SUCCESS) && (retB == SAL_RET_SUCCESS))
    {
        gTB6612DutyPercent = uiDutyPercent;
        return SAL_RET_SUCCESS;
    }

    (void)GPIO_Set(TB6612_STBY_PIN, 0UL);
    mcu_printf("[TB6612][ERROR] PWM update failed; STBY=0\n");

    return SAL_RET_FAILED;
}

static SALRetCode_t TB6612_SetDirection(uint32 uiDirection)
{
    SALRetCode_t ret;

    ret = TB6612_SetDuty(0UL);
    if (ret != SAL_RET_SUCCESS)
    {
        return ret;
    }

    (void)SAL_TaskSleep(TB6612_DIRECTION_CHANGE_DELAY_MS);

    if (uiDirection == TB6612_DIR_FORWARD)
    {
        (void)GPIO_Set(TB6612_AIN1_PIN, 1UL);
        (void)GPIO_Set(TB6612_AIN2_PIN, 0UL);
        (void)GPIO_Set(TB6612_BIN1_PIN, 1UL);
        (void)GPIO_Set(TB6612_BIN2_PIN, 0UL);
    }
    else if (uiDirection == TB6612_DIR_REVERSE)
    {
        (void)GPIO_Set(TB6612_AIN1_PIN, 0UL);
        (void)GPIO_Set(TB6612_AIN2_PIN, 1UL);
        (void)GPIO_Set(TB6612_BIN1_PIN, 0UL);
        (void)GPIO_Set(TB6612_BIN2_PIN, 1UL);
    }
    else if (uiDirection == TB6612_DIR_BRAKE)
    {
        (void)GPIO_Set(TB6612_AIN1_PIN, 1UL);
        (void)GPIO_Set(TB6612_AIN2_PIN, 1UL);
        (void)GPIO_Set(TB6612_BIN1_PIN, 1UL);
        (void)GPIO_Set(TB6612_BIN2_PIN, 1UL);
    }
    else
    {
        (void)GPIO_Set(TB6612_AIN1_PIN, 0UL);
        (void)GPIO_Set(TB6612_AIN2_PIN, 0UL);
        (void)GPIO_Set(TB6612_BIN1_PIN, 0UL);
        (void)GPIO_Set(TB6612_BIN2_PIN, 0UL);
        uiDirection = TB6612_DIR_COAST;
    }

    gTB6612Direction = uiDirection;
    return SAL_RET_SUCCESS;
}

static void TB6612_PrintDebug(void)
{
    uint32 uiHighNs;
    uint32 uiLowNs;

    uiHighNs = (TB6612_PWM_PERIOD_NS * gTB6612DutyPercent) / 100UL;
    uiLowNs  = TB6612_PWM_PERIOD_NS - uiHighNs;

    mcu_printf("\n[TB6612][DEBUG] freq=%dHz period=%dns duty=%d%%\n",
               TB6612_PWM_FREQUENCY_HZ,
               TB6612_PWM_PERIOD_NS,
               gTB6612DutyPercent);
    mcu_printf("  expected: HIGH=%dns LOW=%dns\n", uiHighNs, uiLowNs);
    mcu_printf("  D4/PWMA/PDM7 status=%d, D2/PWMB/PDM8 status=%d\n",
               PDM_GetChannelStatus(TB6612_PWMA_CHANNEL),
               PDM_GetChannelStatus(TB6612_PWMB_CHANNEL));
    mcu_printf("  STBY=%d AIN=%d/%d BIN=%d/%d direction=%d\n",
               GPIO_Get(TB6612_STBY_PIN),
               GPIO_Get(TB6612_AIN1_PIN),
               GPIO_Get(TB6612_AIN2_PIN),
               GPIO_Get(TB6612_BIN1_PIN),
               GPIO_Get(TB6612_BIN2_PIN),
               gTB6612Direction);
    mcu_printf("  Scope/logic analyzer: probe D4 or D2 relative to GND.\n\n");
}

static void TB6612_PrintHelp(void)
{
    mcu_printf("\n=== TB6612 Tera Term control (115200 baud) ===\n");
    mcu_printf("f=forward, r=reverse, c=coast, b=brake\n");
    mcu_printf("0..9=duty 0..90%%, x=duty 100%%\n");
    mcu_printf("d=debug values, e=encoder values, z=encoder zero, h=help\n");
    mcu_printf("Safe order: f or r, then start with 1 (10%%).\n\n");
}

static void TB6612_ProcessUartCommand(sint32 iCommand)
{
    uint32 uiDuty;

    if ((iCommand >= (sint32)'0') && (iCommand <= (sint32)'9'))
    {
        uiDuty = (uint32)(iCommand - (sint32)'0') * 10UL;
        (void)TB6612_SetDuty(uiDuty);
        TB6612_PrintDebug();
    }
    else if ((iCommand == (sint32)'x') || (iCommand == (sint32)'X'))
    {
        (void)TB6612_SetDuty(100UL);
        TB6612_PrintDebug();
    }
    else if ((iCommand == (sint32)'f') || (iCommand == (sint32)'F'))
    {
        (void)TB6612_SetDirection(TB6612_DIR_FORWARD);
        TB6612_PrintDebug();
    }
    else if ((iCommand == (sint32)'r') || (iCommand == (sint32)'R'))
    {
        (void)TB6612_SetDirection(TB6612_DIR_REVERSE);
        TB6612_PrintDebug();
    }
    else if ((iCommand == (sint32)'c') || (iCommand == (sint32)'C'))
    {
        (void)TB6612_SetDirection(TB6612_DIR_COAST);
        TB6612_PrintDebug();
    }
    else if ((iCommand == (sint32)'b') || (iCommand == (sint32)'B'))
    {
        (void)TB6612_SetDirection(TB6612_DIR_BRAKE);
        TB6612_PrintDebug();
    }
    else if ((iCommand == (sint32)'d') || (iCommand == (sint32)'D'))
    {
        TB6612_PrintDebug();
    }
    else if ((iCommand == (sint32)'e') || (iCommand == (sint32)'E'))
    {
        Encoder_Print();
    }
    else if ((iCommand == (sint32)'z') || (iCommand == (sint32)'Z'))
    {
        gEncoderCount = 0;
        gEncoderLastCount = 0;
        gEncoderLastDelta = 0;
        gEncoderRpm = 0;
        mcu_printf("[ENCODER] count reset to zero\n");
    }
    else if ((iCommand == (sint32)'h') || (iCommand == (sint32)'H'))
    {
        TB6612_PrintHelp();
    }
    else
    {
        /* Ignore CR/LF and unsupported input. */
    }
}

static void Encoder_AIsr(void * pArg)
{
    (void)pArg;
    if (GPIO_Get(ENCODER_B_PIN) == 0UL)
    {
        gEncoderCount++;
    }
    else
    {
        gEncoderCount--;
    }
}

static SALRetCode_t Encoder_Init(void)
{
    uint32 uiInput;
    SALRetCode_t ret;

    uiInput = (uint32)(GPIO_FUNC(0UL) | GPIO_INPUT |
                       GPIO_INPUTBUF_EN | GPIO_PULLUP);
    ret = GPIO_Config(ENCODER_A_PIN, uiInput);
    if (ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }
    ret = GPIO_Config(ENCODER_B_PIN, uiInput);
    if (ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    (void)GIC_IntSrcDis(ENCODER_A_IRQ);
    ret = GPIO_IntExtSet(ENCODER_A_IRQ, ENCODER_A_PIN);
    if (ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }
    ret = GIC_IntVectSet(ENCODER_A_IRQ,
                         GIC_PRIORITY_NO_MEAN,
                         GIC_INT_TYPE_EDGE_RISING,
                         (GICIsrFunc)&Encoder_AIsr,
                         (void *)NULL);
    if (ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }
    ret = GIC_IntSrcEn(ENCODER_A_IRQ);
    if (ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    gEncoderCount = 0;
    gEncoderLastCount = 0;
    gEncoderLastDelta = 0;
    gEncoderRpm = 0;
    (void)SAL_GetTickCount(&gEncoderLastSampleTick);
    mcu_printf("[ENCODER] A=D10(GPIO_C13), B=D11(GPIO_C14), PPR=%d\n",
               ENCODER_PULSES_PER_REV);
    return SAL_RET_SUCCESS;
}

static void Encoder_Service(void)
{
    uint32 uiNow;
    uint32 uiElapsedMs;
    uint32 uiDenominator;
    sint32 iNowCount;
    sint32 iDelta;

    if (SAL_GetTickCount(&uiNow) != SAL_RET_SUCCESS)
    {
        return;
    }
    uiElapsedMs = uiNow - gEncoderLastSampleTick;
    if (uiElapsedMs < ENCODER_SAMPLE_PERIOD_MS)
    {
        return;
    }

    iNowCount = gEncoderCount;
    iDelta = iNowCount - gEncoderLastCount;
    uiDenominator = ENCODER_PULSES_PER_REV * uiElapsedMs;
    if (uiDenominator != 0UL)
    {
        gEncoderRpm = (sint32)((iDelta * (sint32)60000) /
                               (sint32)uiDenominator);
    }
    else
    {
        gEncoderRpm = 0;
    }

    gEncoderLastDelta = iDelta;
    gEncoderLastCount = iNowCount;
    gEncoderLastSampleTick = uiNow;
    Encoder_Print();
}

static void Encoder_Print(void)
{
    mcu_printf("[ENCODER] count=%d delta=%d rpm=%d A=%d B=%d\n",
               gEncoderCount, gEncoderLastDelta, gEncoderRpm,
               GPIO_Get(ENCODER_A_PIN), GPIO_Get(ENCODER_B_PIN));
}
#endif

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

#endif  // ( MCU_BSP_SUPPORT_APP_BASE == 1 )

