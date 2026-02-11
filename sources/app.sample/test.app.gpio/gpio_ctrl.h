#ifndef GPIO_CTRL_HEADER
#define GPIO_CTRL_HEADER

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

void BrakeLED_ON(void);
void BrakeLED_OFF(void);
void LeftSignalLED_ON(void);
void LeftSignalLED_OFF(void);
void RightSignalLED_ON(void);
void RightSignalLED_OFF(void);
void HeadLightLED_ON(void);
void HeadLightLED_OFF(void);
void ControlBrakeLight(boolean bTurnOn);
void ControlSignalLight(boolean bLeft, boolean bTurnOn);
void ControlHeadLight(boolean bTurnOn);
void BrakeLightTask(void *pvParameters);
void TurnSignalTask(void *pvParameters);
void EmergencySignalTask(void *pvParameters);
void HeadLightTask(void *pvParameters);

#endif

#endif
