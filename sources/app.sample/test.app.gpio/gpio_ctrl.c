#include "gpio.h"
#include "stdio.h"
#include "bsp.h"
#include "gpio_ctrl.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <vcp_types.h>

extern QueueHandle_t xQ_Brake;
extern QueueHandle_t xQ_Turn;
extern QueueHandle_t xQ_Emer;
extern QueueHandle_t xQ_Head;

#define VCP_DATA_READY_BIT (1 << 0)

#define BrakeLEDPIN       GPIO_GPA(6)
#define LeftSignalLEDPIN  GPIO_GPA(7)
#define RightSignalLEDPIN GPIO_GPA(28)
#define HeadLEDPIN 		  GPIO_GPA(29)

/* ------------------------------- Brake Light ON/OFF ------------------------------- */
void BrakeLED_ON(void)
{
    GPIO_Set(BrakeLEDPIN, 1);
}

void BrakeLED_OFF(void)
{
    GPIO_Set(BrakeLEDPIN, 0);
}

/* -------------------------- Turn Signal Light ON/OFF -------------------------- */
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

/* ------------------------------- Head Light ON/OFF ------------------------------- */
void HeadLightLED_ON(void)
{
    GPIO_Set(HeadLEDPIN, 1);
}

void HeadLightLED_OFF(void)
{
    GPIO_Set(HeadLEDPIN, 0);
}


/* ------------------------------- Control Light ------------------------------- */
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

void ControlHeadLight(boolean bTurnOn)
{
	static boolean binit = FALSE;

	mcu_printf("[LIGHT] Controlling Head Light\r\n");
	if (binit == FALSE) {
        GPIO_Config(HeadLEDPIN, GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
        HeadLightLED_OFF();
        binit = TRUE;
    }

    if (bTurnOn) {
        HeadLightLED_ON();
        mcu_printf("[Head Light] ON\r\n");
    } else {
        HeadLightLED_OFF();
        mcu_printf("[Head Light] OFF\r\n");
    }
}

/* ------------------------------- Task ------------------------------- */

void BrakeLightTask(void *pvParameters) 
{
    uint8 recvBuf[2];

    (void)pvParameters;

    for (;;) 
    {
        if (xQueueReceive(xQ_Brake, recvBuf, portMAX_DELAY) == pdPASS) 
        {
            if (recvBuf[0] == VCP_IO_ACTION_ON) 
            {
                ControlBrakeLight(TRUE);
            }
            else if (recvBuf[0] == VCP_IO_ACTION_OFF) 
            {
                ControlBrakeLight(FALSE);
            }
        }
    }
}

void TurnSignalTask(void *pvParameters)
{
    uint8 recvBuf[2];

    (void)pvParameters;

    for (;;)
    {
        if (xQueueReceive(xQ_Turn, recvBuf, portMAX_DELAY) == pdPASS)
        {
            // [LEFT] 왼쪽 방향지시등 제어
            if (recvBuf[0] == VCP_IO_SUB_LEFT)
            {
                ControlSignalLight(TRUE, (recvBuf[1] == VCP_IO_ACTION_ON));
            }
            // [RIGHT] 오른쪽 방향지시등 제어
            else if (recvBuf[0] == VCP_IO_SUB_RIGHT)
            {
                ControlSignalLight(FALSE, (recvBuf[1] == VCP_IO_ACTION_ON));
            }
        }
    }
}

void EmergencySignalTask(void *pvParameters) 
{
    uint8 recvBuf[2];
    boolean isEmergencyActive = FALSE;
    boolean bLedState = FALSE;
    TickType_t xTimeout;

    (void)pvParameters;

    for (;;) 
    {
        if (isEmergencyActive == TRUE) {
            xTimeout = pdMS_TO_TICKS(500);
        } else {
            xTimeout = portMAX_DELAY;
        }

        if (xQueueReceive(xQ_Emer, recvBuf, xTimeout) == pdPASS) 
        {
            if (recvBuf[0] == VCP_IO_ACTION_ON) 
            {
                isEmergencyActive = TRUE;
                bLedState = TRUE;
                ControlSignalLight(TRUE, TRUE);
                ControlSignalLight(FALSE, TRUE);
            }
            else if (recvBuf[0] == VCP_IO_ACTION_OFF) 
            {
                isEmergencyActive = FALSE;
                bLedState = FALSE;
                ControlSignalLight(TRUE, FALSE);
                ControlSignalLight(FALSE, FALSE);
            }
        }
        else 
        {
            if (isEmergencyActive == TRUE) 
            {
                bLedState = !bLedState;
                ControlSignalLight(TRUE, bLedState);
                ControlSignalLight(FALSE, bLedState);
            }
        }
    }
}

void HeadLightTask(void *pvParameters) 
{
    uint8 recvBuf[2];

    (void)pvParameters;

    for (;;) 
    {
        if (xQueueReceive(xQ_Head, recvBuf, portMAX_DELAY) == pdPASS) 
        {
            ControlHeadLight(recvBuf[0] == VCP_IO_ACTION_ON);
        }
    }
}
