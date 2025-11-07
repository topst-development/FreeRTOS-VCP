
#ifndef MCU_BSP_CAN_VCP_CTRL_HEADER
#define MCU_BSP_CAN_VCP_CTRL_HEADER

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/



/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/
#define BrakeLEDPIN       GPIO_GPA(6)
#define LeftSignalLEDPIN  GPIO_GPA(7)  
#define RightSignalLEDPIN GPIO_GPA(28)
#define HeadLEDPIN 		  GPIO_GPA(29)

#define IN1   GPIO_GPA(22)
#define IN2   GPIO_GPA(21)
#define IN3   GPIO_GPA(20)
#define IN4   GPIO_GPA(19)

#define ENA_SEL      0
#define ENA_PORT    GPIO_PERICH_CH0
#define ENB_SEL      4
#define ENB_PORT    GPIO_PERICH_CH1

#define PDM_PERIOD_NS    (250000)
#define SPEED_MAX    80
#define DUTY_MAX     80
#define MIN_ON_NS    15000

#define I2C_CH     0
#define I2C_PORT   0
#define LCD_ADDR   0x27        
#define I2C_SPEED  100          // kHz
#define LCD_CMD    0
#define LCD_DATA   1

enum VCP_IO_TYPE {
	VCP_IO_BREAK_LIGHT=0x101,
	VCP_IO_TURN_SIGNAL=0x102,
	VCP_IO_EMER_SIGNAL=0x103,
	VCP_IO_HEAD_LIGHT=0x104,
	VCP_IO_FUEL_LEVEL=0x105,
	VCP_IO_MOTOR_SPEED=0x106,
	VCP_IO_MOTOR_WHEEL=0x107
};

	
enum VCP_IO_ACTION {
	VCP_IO_ACTION_ON=0x01,
	VCP_IO_ACTION_OFF=0x02
};

enum VCP_IO_SUBTYPE {
	VCP_IO_SUB_LEFT=0x01,
	VCP_IO_SUB_RIGHT=0x02
};


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/
void ControlBreadBoardSensors(uint32 mId, uint8 nDataLength, sint8* pucData);
boolean InitSensorControls(void);


#endif  // ( MCU_BSP_CAN_VCP_CTRL_HEADER == 1 )

#endif  // MCU_BSP_CAN_DEMO_HEADER

