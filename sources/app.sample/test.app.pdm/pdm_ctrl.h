#ifndef PDM_CTRL_HEADER
#define PDM_CTRL_HEADER

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

void pin_out(uint32 p);
void pin_hi (uint32 p);
void pin_lo (uint32 p);

void MotorPWM_Init(void);
void MotorA_Set(uint32 duty_pct, uint32 forward);
void MotorB_Set(uint32 duty_pct, uint32 forward);
uint32 duty_pct_to_ns(uint32 pct, uint32 period_ns);
sint8 duty_from_speed(sint8 speed);
void ConfigureServoPWM(uint32 channel, uint32 port, uint32 angle_deg);

void MotorSpeedTask(void *pvParameters);
void MotorWheelTask(void *pvParameters);

#endif

#endif
