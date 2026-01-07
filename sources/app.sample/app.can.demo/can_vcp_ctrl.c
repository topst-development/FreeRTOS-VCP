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

#include "can_vcp_ctrl.h"

/**************************************************************************************************
*                                           LOCAL FUNCTIONS
**************************************************************************************************/

/* LED (Class) */
static void LedGpioInitOnce(void);
static uint32 LedPinFromClassId(uint32 mId);
static void ControlClassLedByCanId(uint32 mId, uint8 action);


/* ------------------------------- CLASS LED ------------------------------- */

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

/* one-hot 표시: 특정 CLASS ON이면 나머지는 OFF (데모에 가장 직관적) */
static void ControlClassLedByCanId(uint32 mId, uint8 action)
{
    
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
    }
    else
    {
        /* OFF면 전체 OFF 유지 */
        mcu_printf("[LED] id=0x%x -> OFF\r\n", (unsigned)mId);
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

#endif /* MCU_BSP_SUPPORT_CAN_DEMO == 1 */
