#include "gpio.h"
#include "stdio.h"
#include "bsp.h"
#include "gpio_ctrl.h"
#include <FreeRTOS.h>
#include <event_groups.h>
#include <task.h>
#include <vcp_types.h>

extern EventGroupHandle_t xVcpEventGroup;
extern VcpMessage_t       g_vcp_shared_buf;

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

void BrakeLightTask(void *pvParameters) {
    VcpMessage_t brakeMsg;

    for (;;) {
        xEventGroupWaitBits(xVcpEventGroup, VCP_DATA_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
        brakeMsg = g_vcp_shared_buf;

        if (brakeMsg.mId == VCP_IO_BREAK_LIGHT) {
            if (brakeMsg.data[0] == VCP_IO_ACTION_ON) {
                // turn on the break light
                ControlBrakeLight(TRUE);
            }
            else if (brakeMsg.data[0] == VCP_IO_ACTION_OFF) {
                // turn off the break light
                ControlBrakeLight(FALSE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1)); 
    }
}

void TurnSignalTask(void *pvParameters) {
    VcpMessage_t turnsignalMsg;
    for (;;) {
        xEventGroupWaitBits(xVcpEventGroup, VCP_DATA_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
        turnsignalMsg = g_vcp_shared_buf;

        if (turnsignalMsg.mId == VCP_IO_TURN_SIGNAL) {
            // ucMsgData[0]은 SUBTYPE(LEFT/RIGHT), [1]은 ACTION(ON/OFF)
            if (turnsignalMsg.data[0] == VCP_IO_SUB_LEFT) {
                ControlSignalLight(TRUE, (turnsignalMsg.data[1] == VCP_IO_ACTION_ON));
            }
            else if (turnsignalMsg.data[0] == VCP_IO_SUB_RIGHT) {
                ControlSignalLight(FALSE, (turnsignalMsg.data[1] == VCP_IO_ACTION_ON));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void EmergencySignalTask(void *pvParameters) {
    VcpMessage_t emergencyMsg;
    boolean isEmergencyActive = FALSE;
    boolean bLedState = FALSE;
    EventBits_t uxBits;
    TickType_t xTimeout;

    for (;;) {
        if (isEmergencyActive == TRUE) {
            xTimeout = pdMS_TO_TICKS(500);
        } else {
            xTimeout = portMAX_DELAY;
        }

        uxBits = xEventGroupWaitBits(xVcpEventGroup, VCP_DATA_READY_BIT, pdFALSE, pdTRUE, xTimeout);

        if ((uxBits & VCP_DATA_READY_BIT) == VCP_DATA_READY_BIT) {
            
            emergencyMsg = g_vcp_shared_buf;

            if (emergencyMsg.mId == VCP_IO_EMER_SIGNAL) {
                if (emergencyMsg.data[0] == VCP_IO_ACTION_ON) {
                    isEmergencyActive = TRUE;
                    mcu_printf("[EMER] Active ON\n");
                } 
                else if (emergencyMsg.data[0] == VCP_IO_ACTION_OFF) {
                    isEmergencyActive = FALSE;
                    bLedState = FALSE;
                    ControlSignalLight(TRUE, FALSE);  // 왼쪽 끄기
                    ControlSignalLight(FALSE, FALSE); // 오른쪽 끄기
                    mcu_printf("[EMER] Active OFF\n");
                }
            }
        }
        else if (isEmergencyActive == TRUE) {
            bLedState = !bLedState;
            ControlSignalLight(TRUE, bLedState);
            ControlSignalLight(FALSE, bLedState);
        }
    }
}

void HeadLightTask(void *pvParameters) {
    VcpMessage_t headMsg;
    for (;;) {
        xEventGroupWaitBits(xVcpEventGroup, VCP_DATA_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
        headMsg = g_vcp_shared_buf;

        if (headMsg.mId == VCP_IO_HEAD_LIGHT) {
            ControlHeadLight(headMsg.data[0] == VCP_IO_ACTION_ON);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
