
#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <app_cfg.h>

#include "bsp.h"
#include "gic.h"
#include "gpio.h"
#include "debug.h"
#include "pdm.h"
#include "i2c.h"
#include "stdio.h"

#include "can_vcp_ctrl.h"
#define MIN_DUTY      (20) 

/**************************************************************************************************
*                                           LOCAL FUNCTIONS
**************************************************************************************************/
static PDMModeConfig_t cfgA, cfgB;

static void BrakeLED_ON(void);
static void BrakeLED_OFF(void);
static void LeftSignalLED_ON(void);
static void LeftSignalLED_OFF(void);
static void RightSignalLED_ON(void);
static void RightSignalLED_OFF(void);
static void HeadLED_ON(void);
static void HeadLED_OFF(void);

static void lcd_send(uint8 mode, uint8 data);
static void lcd_cmd(uint8 cmd);
static void lcd_data(uint8 data);
static void lcd_init(void);
static void lcd_print(const char *str);

static void pin_out(uint32 p);
static void pin_hi (uint32 p);
static void pin_lo (uint32 p);

static int pdm_apply(uint32 ch, PDMModeConfig_t* cfg);
static void MotorPWM_Init(void);
static void MotorA_Set(uint32 duty_pct, uint32 forward);
static void MotorB_Set(uint32 duty_pct, uint32 forward);
static uint32 duty_pct_to_ns(uint32 pct, uint32 period_ns);
static sint8 duty_from_speed(sint8 speed);

static void ControlBrakeLight(boolean bTurnOn);
static void ControlSignalLight(boolean bLeft, boolean bTurnOn);
static void ControlHeadLight(boolean bTurnOn);
static void ControlFuelLevel(uint8 fuelLevel);

void InitSensorControlls(void);

/**************************************************************************************************
*                                           LOCAL IMPLEMENTATIONS
**************************************************************************************************/
boolean InitSensorControls(void)
{
	MotorPWM_Init();
	I2C_Init();
    if (I2C_Open(I2C_CH, I2C_PORT, I2C_SPEED, NULL_PTR, NULL_PTR) != SAL_RET_SUCCESS) 
	{
        mcu_printf("[LCD] I2C_Open failed (ch=%d, port=%d, speed=%d)\r\n",
                   (int)I2C_CH, (int)I2C_PORT, (int)I2C_SPEED);
        return FALSE;
    }
    I2C_ScanSlave(I2C_CH);

	lcd_init();

	return TRUE;
}


/* ------------------------------- Brake Light ------------------------------- */
void BrakeLED_ON(void)  
{
	GPIO_Set(BrakeLEDPIN, 1);
}

void BrakeLED_OFF(void)
{
	GPIO_Set(BrakeLEDPIN, 0);
}

void ControlBrakeLight(boolean bTurnOn)
{
    static boolean binit = FALSE;

	mcu_printf("[BRAKE] Controlling Brake Light\r\n");
    if (binit == FALSE) {
        GPIO_Config(BrakeLEDPIN, GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
        BrakeLED_OFF();
        binit = TRUE;
    }
 
    if (bTurnOn) {
        BrakeLED_ON();
        mcu_printf("[BRAKE] ON\r\n");
    } else {
        BrakeLED_OFF();
        mcu_printf("[BRAKE] OFF\r\n");
    }
}

/* -------------------------- Turn Signal Light -------------------------- */
void LeftSignalLED_ON(void)   
{ 
	GPIO_Set(LeftSignalLEDPIN, 1); 
}

void LeftSignalLED_OFF(void)  
{ 
	GPIO_Set(LeftSignalLEDPIN, 0); 
}

void RightSignalLED_ON(void)  
{ 
	GPIO_Set(RightSignalLEDPIN, 1); 
}

void RightSignalLED_OFF(void) 
{ 
	GPIO_Set(RightSignalLEDPIN, 0); 
}

void ControlSignalLight(boolean bLeft, boolean bTurnOn)
{
	static boolean binit = FALSE;

	mcu_printf("[SIGNAL] Controlling Signal Light\r\n");
	if(binit == FALSE) {
		GPIO_Config(LeftSignalLEDPIN,  GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
		GPIO_Config(RightSignalLEDPIN,  GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
		LeftSignalLED_OFF();
		RightSignalLED_OFF();
		binit = TRUE;
	}
	
	if (bLeft) {
		if (bTurnOn) {
			LeftSignalLED_ON();
			mcu_printf("[LeftSignal] ON\r\n");
		}
		else {
			LeftSignalLED_OFF();
			mcu_printf("[LeftSignal] OFF\r\n");
		}	
	} else {
		if (bTurnOn) {
			RightSignalLED_ON();
			mcu_printf("[RightSignal] ON\r\n");
		}
		else {
			RightSignalLED_OFF();
			mcu_printf("[RightSignal] OFF\r\n");
		}		
	}
}

/* ------------------------------- Head Light ------------------------------- */

void HeadLED_ON(void)  
{
	GPIO_Set(HeadLEDPIN, 1);
}

void HeadLED_OFF(void)
{
	GPIO_Set(HeadLEDPIN, 0);
}

void ControlHeadLight(boolean bTurnOn)
{
	static boolean binit = FALSE;

	mcu_printf("[LIGHT] Controlling Head Light\r\n");
	if (binit == FALSE) {
        GPIO_Config(HeadLEDPIN, GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
        HeadLED_OFF();
        binit = TRUE;
    }
 
    if (bTurnOn) {
        HeadLED_ON();
        mcu_printf("[Head Light] ON\r\n");
    } else {
        HeadLED_OFF();
        mcu_printf("[Head Light] OFF\r\n");
    }
}

/* ------------------------------- Fuel Level ------------------------------- */

void lcd_send(uint8 mode, uint8 data) 
{
    uint8 high = data & 0xF0;
    uint8 low  = (data << 4) & 0xF0;
    uint8 buf[6];

    // BL=0x08, EN=0x04, RS=0x01 (RW=0)
    buf[0] = high | 0x08 | (mode ? 0x01 : 0x00);
    buf[1] = high | 0x0C | (mode ? 0x01 : 0x00);
    buf[2] = high | 0x08 | (mode ? 0x01 : 0x00);
    buf[3] = low  | 0x08 | (mode ? 0x01 : 0x00);
    buf[4] = low  | 0x0C | (mode ? 0x01 : 0x00);
    buf[5] = low  | 0x08 | (mode ? 0x01 : 0x00);

    I2CXfer_t xfer = {
        .xCmdLen = 0,
        .xOutLen = 6,
        .xOutBuf = buf,
        .xInLen  = 0,
        .xInBuf  = NULL,
        .xCmdBuf = NULL,
        .xOpt    = 0
    };

    (void)I2C_Xfer(I2C_CH, (uint8)(LCD_ADDR << 1), xfer, 0); // sync
    SAL_TaskSleep(2);
}

void lcd_cmd(uint8 cmd)  
{ 
	lcd_send(LCD_CMD, cmd); 
}

void lcd_data(uint8 dat) 
{ 
	lcd_send(LCD_DATA, dat); 
}

void lcd_init(void) 
{
    SAL_TaskSleep(50);
    lcd_cmd(0x33);
    lcd_cmd(0x32);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
    SAL_TaskSleep(5);
}

void lcd_print(const char *str) 
{
    while (*str) lcd_data((uint8)*str++);
}

void ControlFuelLevel(uint8 fuelLevel)
{
    static uint8 prev       = 0xFF;

	uint8 pct = (fuelLevel > 100) ? 100 : fuelLevel;

    if (pct == prev) 
	{
        return;
    }
    prev = pct;
	
    lcd_cmd((uint8)(0x80 | 0x00));        
    lcd_print("Fuel Level      ");       

    char line[17];
    (void)snprintf(line, sizeof(line), "Fuel: %3u%%     ", (unsigned)pct); 
    lcd_cmd((uint8)(0x80 | 0x40));     
    lcd_print(line);

	mcu_printf("[FUEL] Controlling Fuel Level\r\n");
}

/* -------------------------- Motor with PWM -------------------------- */

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
    if (speed > 80) 
	{
		speed = 80;
	}
    if (speed < 0)
    {
        return 0;
    }

    return 40 + ((speed - 1) * 80) / 79;
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

void ControlBreadBoardSensors(uint32 mId, uint8 nDataLength, sint8* pucData)
{
	sint8 ucMsgData[2] = {0x00, 0x00};

	if(nDataLength == 0)
	{
		// Somethong wrong. does nothing
		return;
	}
	
	switch(mId)
	{
		case VCP_IO_BREAK_LIGHT:
			ucMsgData[0] = pucData[0];
			if(ucMsgData[0] == VCP_IO_ACTION_ON)
			{
				// turn on the break light
				ControlBrakeLight(TRUE);
			}
			else
			{
				// turn off the break light
				ControlBrakeLight(FALSE);
			}
				
			break;

		case VCP_IO_TURN_SIGNAL:
			ucMsgData[0] = pucData[0];
			ucMsgData[1] = pucData[1];

			if (ucMsgData[0] == VCP_IO_SUB_LEFT)
            {  
                if(ucMsgData[1] == VCP_IO_ACTION_ON)
                {
                    // turn on the left signal
                    ControlSignalLight(TRUE, TRUE);
                }
                else
                {
                    // turn off the left light
                    ControlSignalLight(TRUE, FALSE);
                }
            }

            if (ucMsgData[0] == VCP_IO_SUB_RIGHT)
            {              
                if(ucMsgData[1] == VCP_IO_ACTION_ON)
                {
                    // turn on the right signal
                    ControlSignalLight(FALSE, TRUE);
                }
                else
                {
                    // turn off the right light
                    ControlSignalLight(FALSE, FALSE);
                }
            }
           
            break;

		case VCP_IO_EMER_SIGNAL:
			ucMsgData[0] = pucData[0];
			if(ucMsgData[0] == VCP_IO_ACTION_ON)
			{
				// turn on the emergency signal lights
				ControlSignalLight(TRUE, TRUE);
				ControlSignalLight(FALSE, TRUE);
			}
			else
			{
				// turn off the emergency signal lights
				ControlSignalLight(TRUE, FALSE);
				ControlSignalLight(FALSE, FALSE);
			}

			break;

		case VCP_IO_HEAD_LIGHT:
			ucMsgData[0] = pucData[0];
			if(ucMsgData[0] == VCP_IO_ACTION_ON)
			{
				// turn on the head light
				ControlHeadLight(TRUE);
			}
			else
			{
				// turn off the head light
				ControlHeadLight(FALSE);
			}

			break;

		case VCP_IO_FUEL_LEVEL:
			ucMsgData[0] = (pucData[0] > 100)?100:pucData[0];
			// set the fuel level
			ControlFuelLevel(ucMsgData[0]);
			//~
			break;

		case VCP_IO_MOTOR_SPEED:
		{
			static sint8 duty = 0;
			sint8 speed = pucData[0]; // 0~80

			if(speed==0)
			{
				MotorA_Set(0, 1);
				MotorB_Set(0, 1);
			}
			else if(speed > 0)
			{
                duty = duty_from_speed(speed);
				MotorA_Set(duty, 1);
				MotorB_Set(duty, 1);
            }
            else 
            {
                duty = duty_from_speed(speed);
				MotorA_Set(duty, 0);
				MotorB_Set(duty, 0);
            }

			mcu_printf("[MOTOR] speed=%d => duty=%d%%\r\n", (sint8)speed, (sint8)duty);
			
			break;
		}
		case VCP_IO_MOTOR_WHEEL: // Servo motor control
		{
			int8_t input = (int8_t)pucData[0];   // 0~256
			if (input < 0) input = 0;
			if (input > 127) input = 127;

			// 중앙 기준으로 -45~+45도로 매핑
			int angle = 90 + ((input - 65) * 45) / 65;

			ConfigureServoPWM(5, GPIO_PERICH_CH3, angle);

			mcu_printf("[SERVO] input=%d -> angle=%d deg\r\n", input, angle);
			break;
		}
			
		default:
			// undefined message
			mcu_printf("[%s][%d] undefined can id !!!\r\n", __FUNCTION__, __LINE__);
			break;
	}

}
#endif


