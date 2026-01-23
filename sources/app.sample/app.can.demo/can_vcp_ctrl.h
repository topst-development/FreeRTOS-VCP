#ifndef MCU_BSP_CAN_VCP_CTRL_HEADER
#define MCU_BSP_CAN_VCP_CTRL_HEADER

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/
#define CLASS_00_LED       GPIO_GPA(6)
#define CLASS_01_LED       GPIO_GPA(7)
#define CLASS_02_LED       GPIO_GPA(28)
#define CLASS_03_LED       GPIO_GPA(29)
#define IN1   GPIO_GPA(22)
#define IN2   GPIO_GPA(21)
#define IN3   GPIO_GPA(20)
#define IN4   GPIO_GPA(19)

#define ENA_SEL      0
#define ENA_PORT    GPIO_PERICH_CH0
#define ENB_SEL      4
#define ENB_PORT    GPIO_PERICH_CH1

#define PWM_PERIOD_NS    (20000)
#define SPEED_MAX    100
#define DUTY_MAX     100
#define MIN_ON_NS    60000
/* CAN Message IDs */
typedef enum
{
    CLASS_00 = 0x100,
    CLASS_01 = 0x101,
    CLASS_02 = 0x102,
    CLASS_03 = 0x103
} VCP_IO_TYPE;

/* Common action codes */
typedef enum
{
    VCP_IO_ACTION_ON  = 0x01,
    VCP_IO_ACTION_OFF = 0x02
} VCP_IO_ACTION;

/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/
void ControlBreadBoardSensors(uint32 mId, uint8 nDataLength, sint8* pucData);


#endif /* MCU_BSP_SUPPORT_CAN_DEMO == 1 */

#endif /* MCU_BSP_CAN_VCP_CTRL_HEADER */
