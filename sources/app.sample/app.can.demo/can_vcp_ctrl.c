
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
static void MotorPDM_Init(void);
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
}


/* ------------------------------- Brake Light ------------------------------- */
void BrakeLED_ON(void)  
{
}

void BrakeLED_OFF(void)
{
}

void ControlBrakeLight(boolean bTurnOn)
{
}

/* -------------------------- Turn Signal Light -------------------------- */
void LeftSignalLED_ON(void)   
{ 
}

void LeftSignalLED_OFF(void)  
{ 
}

void RightSignalLED_ON(void)  
{ 
}

void RightSignalLED_OFF(void) 
{ 
}

void ControlSignalLight(boolean bLeft, boolean bTurnOn)
{
}

/* ------------------------------- Head Light ------------------------------- */

void HeadLED_ON(void)  
{
}

void HeadLED_OFF(void)
{
}

void ControlHeadLight(boolean bTurnOn)
{
}

/* ------------------------------- Fuel Level ------------------------------- */

void lcd_send(uint8 mode, uint8 data) 
{
}

void lcd_cmd(uint8 cmd)  
{ 
}

void lcd_data(uint8 dat) 
{ 
}

void lcd_init(void) 
{
}

void lcd_print(const char *str) 
{
}

void ControlFuelLevel(uint8 fuelLevel)
{
}

/* -------------------------- Motor with PWM -------------------------- */

static inline void pin_out(uint32 p) {}
static inline void pin_hi(uint32 p)  {}
static inline void pin_lo(uint32 p)  {}
static int pdm_apply(uint32 ch, PDMModeConfig_t* cfg)
{
}

uint32 duty_pct_to_ns(uint32 pct, uint32 period_ns)
{
}

sint8 duty_from_speed(sint8 speed)
{
}

void MotorPDM_Init(void)
{
}

void MotorA_Set(uint32 duty_pct, uint32 forward)
{
}

void MotorB_Set(uint32 duty_pct, uint32 forward)
{
}

void ConfigureServoPDM(uint32 channel, uint32 port, uint32 angle_deg)
{
}

void ControlBreadBoardSensors(uint32 mId, uint8 nDataLength, sint8* pucData)
{
}
#endif


