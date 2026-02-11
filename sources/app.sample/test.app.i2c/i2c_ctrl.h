#ifndef MCU_BSP_CAN_VCP_CTRL_HEADER
#define MCU_BSP_CAN_VCP_CTRL_HEADER

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

void lcd_send(uint8 mode, uint8 data);
void lcd_cmd(uint8 cmd);
void lcd_data(uint8 data);
void lcd_init(void);
void lcd_print(const char *str);
void ControlFuelLevel(uint8 fuelLevel);
boolean LCDSensor_Init(void);

void FuelLevelTask(void *pvParameters);

#endif

#endif
