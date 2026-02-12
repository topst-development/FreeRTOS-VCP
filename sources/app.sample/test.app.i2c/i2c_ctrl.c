#include "i2c.h"
#include "stdio.h"
#include "i2c_ctrl.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <vcp_types.h>

extern QueueHandle_t xQ_Fuel;

#define VCP_DATA_READY_BIT (1 << 0)

#define I2C_CH     0
#define I2C_PORT   0
#define LCD_ADDR   0x27        
#define I2C_SPEED  100          // kHz
#define LCD_CMD    0
#define LCD_DATA   1


boolean LCDSensor_Init(void)
{
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

/* ------------------------------- Task ------------------------------- */

void FuelLevelTask(void *pvParameters) 
{
    uint8 recvBuf[2];
    uint8 fuelLevel = 0;

    (void)pvParameters;

    for (;;) 
    {
        if (xQueueReceive(xQ_Fuel, recvBuf, portMAX_DELAY) == pdPASS) 
        {
            fuelLevel = (recvBuf[0] > 100) ? 100 : recvBuf[0];
			ControlFuelLevel(fuelLevel);
        }
    }
}
