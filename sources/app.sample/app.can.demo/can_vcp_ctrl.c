#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/
#include <app_cfg.h>

#include "bsp.h"
#include "gic.h"
#include "gpio.h"
#include "debug.h"
#include "i2c.h"
#include "stdio.h"
#include "pdm.h"
#include "can_vcp_ctrl.h"
#define MIN_DUTY      (0) 
static PDMModeConfig_t cfgA, cfgB;
/**************************************************************************************************
*                                           LOCAL FUNCTIONS
**************************************************************************************************/

/* LED (Class) */
static void LedGpioInitOnce(void);
static uint32 LedPinFromClassId(uint32 mId);
static void ControlClassLedByCanId(uint32 mId, uint8 action);
#define SERVO_STEP_SIZE  5   // 서보 모터 변화 단계 (도)
#define SERVO_STEP_DELAY 10
static uint32 gCurrentAngle = 90;
static inline void pin_out(uint32 p) { GPIO_Config(p, GPIO_OUTPUT | GPIO_FUNC(0) | GPIO_DS(3)); }
static inline void pin_hi(uint32 p)  { GPIO_Set(p, 1); }
static inline void pin_lo(uint32 p)  { GPIO_Set(p, 0); }
static int pdm_apply(uint32 ch, PDMModeConfig_t* cfg)
{
    (void)PDM_Disable(ch, PMM_OFF);
    uint32 wait = 0;

    while (PDM_GetChannelStatus(ch) && wait < 100) {
        SAL_TaskSleep(1);
        wait++;
    }

    if (PDM_SetConfig(ch, cfg) != SAL_RET_SUCCESS) return -1;
    if (PDM_Enable(ch, PMM_OFF) != SAL_RET_SUCCESS) return -2;
    return 0;
}

uint32 duty_pct_to_ns(uint32 pct, uint32 period_ns)
{
    if (pct == 0) return 0;
    if (pct > 100) pct = 100;
    if (pct < MIN_DUTY && pct > 0) pct = MIN_DUTY;
    uint64 num = (uint64)period_ns * (uint64)pct + 50ULL;
    return (uint32)(num / 100ULL);
}

sint8 duty_from_speed(sint8 speed)
{
    if (speed == 0) 
	{
		return 0;
	}
    if (speed > 100) 
	{
		speed = 100;
	}
    if (speed < 0)
    {
        speed = speed * -1;
    }

     return MIN_DUTY + ((speed - 1) * (100 - MIN_DUTY)) / 100;
}

void MotorPWM_Init(void)
{
    static boolean inited = FALSE;
    if (inited) return;

    /* GPIO 초기화 */
    pin_out(IN1); pin_out(IN2);
    pin_out(IN3); pin_out(IN4);
    pin_lo(IN1); pin_lo(IN2);
    pin_lo(IN3); pin_lo(IN4);

    /* PDM 초기화 */
    PDM_Init();
    PDM_CfgSetWrPw();
    PDM_CfgSetWrLock(0);

    SAL_MemSet(&cfgA, 0, sizeof(cfgA));
    cfgA.mcPortNumber      = ENA_PORT;
    cfgA.mcOperationMode   = PDM_OUTPUT_MODE_PHASE_1;
    cfgA.mcPeriodNanoSec1  = PWM_PERIOD_NS;
    cfgA.mcDutyNanoSec1    = 0;
    (void)pdm_apply(ENA_SEL, &cfgA);

    SAL_MemSet(&cfgB, 0, sizeof(cfgB));
    cfgB.mcPortNumber      = ENB_PORT;
    cfgB.mcOperationMode   = PDM_OUTPUT_MODE_PHASE_1;
    cfgB.mcPeriodNanoSec1  = PWM_PERIOD_NS;
    cfgB.mcDutyNanoSec1    = 0;
    (void)pdm_apply(ENB_SEL, &cfgB);

    inited = TRUE;
}

void MotorA_Set(uint32 duty_pct, uint32 forward)
{
    if (duty_pct == 0) {
        pin_lo(IN1); pin_lo(IN2);
    } else if (forward) {
        pin_lo(IN1); pin_hi(IN2);   // 정방향
    } else {
        pin_hi(IN1); pin_lo(IN2);   // 역방향
    }

    cfgA.mcDutyNanoSec1 = duty_pct_to_ns(duty_pct, cfgA.mcPeriodNanoSec1);
    (void)pdm_apply(ENA_SEL, &cfgA);
    
}

void MotorB_Set(uint32 duty_pct, uint32 forward)
{
    if (duty_pct == 0) {
        pin_lo(IN3); pin_lo(IN4);
    } else if (forward) {
        pin_hi(IN3); pin_lo(IN4);   // 정방향
    } else {
        pin_lo(IN3); pin_hi(IN4);   // 역방향
    }

    cfgB.mcDutyNanoSec1 = duty_pct_to_ns(duty_pct, cfgB.mcPeriodNanoSec1);
    (void)pdm_apply(ENB_SEL, &cfgB);
    
}

/* ------------------------------- CLASS LED ------------------------------- */

void SystemPwmInitOnce(void)
{
    static boolean inited = FALSE;
    if (inited) return;

    PDM_Init();
    PDM_CfgSetWrPw();
    PDM_CfgSetWrLock(0);

    MotorPWM_Init();   // 여기서만 PDM 관련 init
    inited = TRUE;
}

static void LedGpioInitOnce(void)
{
    static boolean inited = FALSE;
    if (inited) return;

    GPIO_Config(CLASS_00_LED, GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
    GPIO_Config(CLASS_01_LED, GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
    GPIO_Config(CLASS_02_LED, GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
    GPIO_Config(CLASS_03_LED, GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);

    GPIO_Set(CLASS_00_LED, 0);
    GPIO_Set(CLASS_01_LED, 0);
    GPIO_Set(CLASS_02_LED, 0);
    GPIO_Set(CLASS_03_LED, 0);

    inited = TRUE;
}



static uint32 LedPinFromClassId(uint32 mId)
{
    switch (mId)
    {
        case CLASS_00: return CLASS_00_LED;
        case CLASS_01: return CLASS_01_LED;
        case CLASS_02: return CLASS_02_LED;
        case CLASS_03: return CLASS_03_LED;
        default:       return 0;
    }
}
void Servo_Set_Smooth(uint32 target_angle)
{
    while (gCurrentAngle != target_angle)
    {
        if (gCurrentAngle < target_angle) gCurrentAngle += SERVO_STEP_SIZE;
        else gCurrentAngle -= SERVO_STEP_SIZE;

        ConfigureServoPWM(5, GPIO_PERICH_CH3, gCurrentAngle);
        SAL_TaskSleep(SERVO_STEP_DELAY);
    }
}
/* one-hot 표시: 특정 CLASS ON이면 나머지는 OFF (데모에 가장 직관적) */
static void ControlClassLedByCanId(uint32 mId, uint8 action)
{
    static boolean pdm_inited = FALSE;
    static uint8 toggle = 0;  // 0: left, 1: right
    static sint8 duty = 0;
    uint32 pin = LedPinFromClassId(mId);
    if (pin == 0)
    {
        mcu_printf("[LED] unknown class id:0x%x\r\n", (unsigned)mId);
        return;
    }
    LedGpioInitOnce();

    /* one-hot: 모두 끄고, 선택만 켬 */
    GPIO_Set(CLASS_00_LED, 0);
    GPIO_Set(CLASS_01_LED, 0);
    GPIO_Set(CLASS_02_LED, 0);
    GPIO_Set(CLASS_03_LED, 0);

    if (action == VCP_IO_ACTION_ON)
    {
        GPIO_Set(pin, 1);
        mcu_printf("[LED] id=0x%x -> ON\r\n", (unsigned)mId);
        uint32 angle = (toggle == 0) ? 40: 150;
        toggle ^= 1;
        Servo_Set_Smooth(angle);
        mcu_printf("[SERVO] angle=%d deg\r\n", angle);
        duty = duty_from_speed(50);
        MotorA_Set(duty, 1);
		MotorB_Set(duty, 1);
    }
    else
    {
        /* OFF면 전체 OFF 유지 */
        mcu_printf("[LED] id=0x%x -> OFF\r\n", (unsigned)mId);
        duty = duty_from_speed(60);
        Servo_Set_Smooth(100);
        MotorA_Set(duty, 1);
		MotorB_Set(duty, 1);
    }
}


/* ------------------------------- CAN Dispatcher ------------------------------- */

void ControlBreadBoardSensors(uint32 mId, uint8 nDataLength, sint8* pucData)
{
    if (nDataLength == 0 || pucData == NULL_PTR)
        return;

    /* 공통: payload[0] 사용 */
    uint8 d0 = (uint8)pucData[0];

    switch (mId)
    {
        case CLASS_00:
        case CLASS_01:
        case CLASS_02:
        case CLASS_03:
            /* payload[0] = VCP_IO_ACTION_ON/OFF */
            ControlClassLedByCanId(mId, d0);
            break;

        default:
            mcu_printf("[%s][%d] undefined can id:0x%x\r\n",
                       __FUNCTION__, __LINE__, (unsigned)mId);
            break;
    }
}


void ConfigureServoPWM(uint32 channel, uint32 port, uint32 angle_deg)
{
    PDMModeConfig_t pwm_cfg;
    uint32 duty_ns = 500000 + (angle_deg * (2000000 / 180)); // 0~180도 → 0.5~2.5ms
    uint32 wait_cnt = 0;

    pwm_cfg.mcPortNumber      = port;
    pwm_cfg.mcOperationMode   = PDM_OUTPUT_MODE_PHASE_1;
    pwm_cfg.mcInversedSignal  = 0;
    pwm_cfg.mcOutSignalInIdle = 0;
    pwm_cfg.mcLoopCount       = 0;
    pwm_cfg.mcOutputCtrl      = 0;

    pwm_cfg.mcPeriodNanoSec1  = 20000000; // 20ms (50Hz)
    pwm_cfg.mcDutyNanoSec1    = duty_ns;
    pwm_cfg.mcPeriodNanoSec2  = 0;
    pwm_cfg.mcDutyNanoSec2    = 0;

    PDM_Disable(channel, PMM_ON);
    while (PDM_GetChannelStatus(channel))
    {
        SAL_TaskSleep(1);
        if (++wait_cnt > 100)
        {
            mcu_printf("Timeout on channel %d\n", channel);
            return;
        }
    }

    if (PDM_SetConfig(channel, &pwm_cfg) != SAL_RET_SUCCESS)
    {
        mcu_printf("SetConfig fail (CH:%d)\n", channel);
        return;
    }

    if (PDM_Enable(channel, PMM_ON) != SAL_RET_SUCCESS)
    {
        mcu_printf("Enable fail (CH:%d)\n", channel);
        return;
    }

    mcu_printf("CH%d angle: %3d° → duty: %d ns\n", channel, angle_deg, duty_ns);
}

#endif /* MCU_BSP_SUPPORT_CAN_DEMO == 1 */
