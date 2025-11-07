// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_demo.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <app_cfg.h>

#include "bsp.h"
#include "gic.h"
#include "gpio.h"
#include "debug.h"

#include "can_config.h"
#include "can_reg.h"
#include "can.h"
#include "can_drv.h"
#include "can_porting.h"
#include "can_demo.h"
#include "can_vcp_ctrl.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static CANDemoTestInfo_t sTestInfo = {
    .tiRecv = TRUE
};

static CANFlagValue_t gCompletedFlag[3];
static CANFlagValue_t gErrorFlag[3];
static uint32 gReceiveFlag[3];

#ifdef CAN_DEMO_RESPONSE_TEST
static CANMessage_t sTxPreMessageInfo =
{
#if 1 /* Misra2012:9.3 - Partially Uninitialized Array */
 /* BufferType,                  Index, ESI, ExtendedID, RTR, ID,    FD, BRS, MM,   EventFIFO, DLC, DATA */
    CAN_TX_BUFFER_TYPE_FIFO,     0,     0,   0,          0,   0xFF,  1,  1,   0xFF, 1,         5,   {0, /* Data is definded as much as DLC in sending function */}
#else
 /* BufferType,                  Index, ESI, ExtendedID, RTR, ID,    FD, BRS, MM,   EventFIFO, DLC, DATA */
    CAN_TX_BUFFER_TYPE_FIFO,     0,     0,   0,          0,   0xFF,  1,  1,   0xFF, 1,         5,   {[0]=0x11,[1]=0x22,[2]=0x33,[3]=0x44,[4]=0x55}
#endif
};
#endif

static CANMessage_t sTxMessageInfo[CAN_MAX_TEST_MSG_NUM] =
{
#if 1 /* Misra2012:9.3 - Partially Uninitialized Array */
   /* BufferType,                 Index, ESI, ExtendedID, RTR, ID,    FD, BRS, MM,   EventFIFO, DLC, DATA */
    { CAN_TX_BUFFER_TYPE_DBUFFER, 0,     0,   0,          0,   0x11,  1,  1,   0xFF, 1,         1,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 1,     0,   1,          0,   0x22,  1,  1,   0xFF, 1,         2,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 2,     0,   0,          0,   0x33,  1,  1,   0xFF, 1,         3,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 3,     0,   1,          0,   0x44,  1,  1,   0xFF, 1,         4,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x55,  1,  1,   0xFF, 1,         5,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x66,  1,  1,   0xFF, 1,         6,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x77,  1,  1,   0xFF, 1,         7,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x88,  1,  1,   0xFF, 1,         8,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x99,  1,  1,   0xFF, 1,         12,  {0, /* Data is definded as much as DLC in sending function */} },

#else
   /* BufferType,                 Index, ESI, ExtendedID, RTR, ID,    FD, BRS, MM,   EventFIFO, DLC, DATA */
    { CAN_TX_BUFFER_TYPE_DBUFFER, 0,     0,   0,          0,   0x11,  1,  1,   0xFF, 1,         1,   {[0]=0x88} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 1,     0,   1,          0,   0x22,  1,  1,   0xFF, 1,         2,   {[0]=0x77,[1]=0x88} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 2,     0,   0,          0,   0x33,  1,  1,   0xFF, 1,         3,   {[0]=0x66,[1]=0x77,[2]=0x88} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 3,     0,   1,          0,   0x44,  1,  1,   0xFF, 1,         4,   {[0]=0x55,[1]=0x66,[2]=0x77,[3]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x55,  1,  1,   0xFF, 1,         5,   {[0]=0x44,[1]=0x55,[2]=0x66,[3]=0x77,[4]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x66,  1,  1,   0xFF, 1,         6,   {[0]=0x33,[1]=0x44,[2]=0x55,[3]=0x66,[4]=0x77,[5]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x77,  1,  1,   0xFF, 1,         7,   {[0]=0x22,[1]=0x33,[2]=0x44,[3]=0x55,[4]=0x66,[5]=0x77,[6]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x88,  1,  1,   0xFF, 1,         8,   {[0]=0x11,[1]=0x22,[2]=0x33,[3]=0x44,[4]=0x55,[5]=0x66,[6]=0x77,[7]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x99,  1,  1,   0xFF, 1,         12,  {[0]=0x00,[1]=0x11,[2]=0x22,[3]=0x33,[4]=0x44,[5]=0x55,[6]=0x66,[7]=0x77,[8]=0x88,[9]=0x99,[10]=0x10,[11]=0x12} },
#endif
};


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static void CAN_DemoCallbackTxEvent
(
    uint8                               ucCh,
    CANTxInterruptType_t                uiIntType
);

static void CAN_DemoCallbackRxEvent
(
    uint8                               ucCh,
    uint32                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
);

static void CAN_DemoCallbackErrorEvent
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
);

static void CAN_DemoSend
(
    uint8                               ucCh
);

static void CAN_DemoReceive
(
    uint8                               ucCh
);

static void CAN_DemoSendReceive
(
    void
);

static void CAN_DemoUsage
(
    void
);

static void CAN_DemoTask
(
    void *                              pArg
);


/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

static void CAN_DemoCallbackTxEvent
(
    uint8                               ucCh,
    CANTxInterruptType_t                uiIntType
)
{
    if( uiIntType == CAN_TX_INT_TYPE_TRANSMIT_COMPLETED )
    {
        gCompletedFlag[ucCh] = CAN_FLAG_TRUE;
    }
}

static void CAN_DemoCallbackRxEvent
(
    uint8                               ucCh,
    uint32                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
)
{
    if( uiError == CAN_ERROR_NONE )
    {
        gReceiveFlag[ucCh] = uiRxIndex + 1UL;
    }

    ( void ) uiRxBufferType;
}

static void CAN_DemoCallbackErrorEvent
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
)
{
    CAN_D( "[CAN ] chennel %d Error Event type: %d, ", ucCh, uiError );

    switch( uiError )
    {
        case CAN_ERROR_INT_ACCESS_RESERVED_ADDRESS:
        {
            CAN_E( "ACCESS_RESERVED_ADDRESS Error\r\n" );

            break;
        }

        case CAN_ERROR_INT_PROTOCOL:
        {
            CAN_E( "PROTOCO Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_BUS_OFF:
        {
            CAN_E( "BUS_OFF Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_WARNING:
        {
            CAN_E( "WARNING Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_PASSIVE:
        {
            CAN_E( "PASSIVE Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_BIT:
        {
            CAN_E( "BIT Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_TIMEOUT:
        {
            CAN_E( "TIMEOUT Error \r\n" );

            break;
        }

        case CAN_ERROR_INT_RAM_ACCESS_FAIL:
        {
            CAN_E( "RAM_ACCESS_FAIL \r\n" );

            break;
        }

        case CAN_ERROR_INT_TX_EVENT_FULL:
        {
            CAN_E( "TX_EVENT_FULL Error \r\n" );

            break;
        }

        case CAN_ERROR_INT_TX_EVENT_LOST:
        {
            CAN_E( "TX_EVENT_LOST Error \r\n" );

            break;
        }

        default:
        {
            CAN_E( "Error \r\n" );

            break;
        }
    }

    gErrorFlag[ucCh] = CAN_FLAG_TRUE;
}

static void CAN_DemoSend
(
    uint8                               ucCh
)
{
    uint8           ucTxMsgDlc;
    uint8           ucTxBufferIndex;
    uint32          uiTimeout;
    uint32          uiTxMsgCnt;
    uint32          uiTxEvtMsgCnt;
    CANMessage_t *  psTxMsg;
    CANTxEvent_t    sTxEvtMsg;
    CANErrorType_t  result;

    psTxMsg         = NULL_PTR;
    uiTxEvtMsgCnt   = 0;

    for( uiTxMsgCnt = 0UL ; uiTxMsgCnt < CAN_MAX_TEST_MSG_NUM ; uiTxMsgCnt++ )
    {
        uiTimeout = 10; //ms

        /* Fill data */
        for( ucTxMsgDlc = 0U ; ucTxMsgDlc < sTxMessageInfo[ uiTxMsgCnt ].mDataLength ; ucTxMsgDlc++ )
        {
            sTxMessageInfo[ uiTxMsgCnt ].mData[ ucTxMsgDlc ] = ucTxMsgDlc;
        }

        /* Send Tx message */
        psTxMsg = &sTxMessageInfo[ uiTxMsgCnt ];

        ( void ) CAN_SendMessage( ucCh, psTxMsg, &ucTxBufferIndex );

        while ( ( gCompletedFlag[ucCh] == CAN_FLAG_FALSE ) && ( uiTimeout > 0UL ) )
        {
            CAN_PortingDelay( 1 );

            uiTimeout--;
        }

        /* Check send message */
        if( ( gCompletedFlag[ucCh] == CAN_FLAG_TRUE ) && ( uiTimeout != 0UL ) )
        {
            if( psTxMsg != NULL_PTR )
            {
                CAN_D( "[CAN DEMO] SEND     : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh, uiTxMsgCnt, psTxMsg->mId );

                uiTxEvtMsgCnt = CAN_CheckNewTxEvent( ucCh );

                if( 0UL < uiTxEvtMsgCnt )
                {
                    /* Get & Check Tx event message */
                    ( void ) SAL_MemSet( &sTxEvtMsg, 0, sizeof( CANTxEvent_t ) );

                    result = CAN_GetNewTxEvent( ucCh, &sTxEvtMsg );

                    if( result == CAN_ERROR_NONE )
                    {
                        if( psTxMsg->mId == sTxEvtMsg.teId )
                        {
                            CAN_D( "[CAN DEMO] TX_EVENT : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh, uiTxMsgCnt, psTxMsg->mId );
                        }
                        else
                        {
                            CAN_D( "[CAN DEMO] TX_EVENT : Fail(ID not match) - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh, uiTxMsgCnt, psTxMsg->mId );
                        }
                    }
                    else
                    {
                        CAN_D( "[CAN DEMO] TX_EVENT : Fail to get Tx Event Message \r\n" );
                    }
                }
            }

            gCompletedFlag[ucCh] = CAN_FLAG_FALSE;
        }
    }
}

static void CAN_DemoReceive
(
    uint8                               ucCh
)
{
    uint32          uiRxMsgNum;
    uint8           ucMsgLength;
    CANMessage_t    sRxMsg;

    uiRxMsgNum      = 0;

    if( 0UL < gReceiveFlag[ ucCh ] )
    {
        while( 1 )
        {
            uiRxMsgNum = CAN_CheckNewRxMessage( ucCh );

            if( 0UL < uiRxMsgNum )
            {
                ( void ) CAN_GetNewRxMessage( ucCh, &sRxMsg );

                mcu_printf( "[CAN DEMO]\n" );
                mcu_printf( "< Channel %d Received Message Information >\n", ucCh );
                mcu_printf( "***********************************************************************************\n" );
                mcu_printf( "[ID] : 0x%X, [DATA SIZE] : %d, [DATA] : \r\n", sRxMsg.mId, sRxMsg.mDataLength );

                for( ucMsgLength = 1U ; ucMsgLength < ( sRxMsg.mDataLength + 1U ) ; ucMsgLength++ )
                {
                    mcu_printf( "0x%02X ", sRxMsg.mData[ ucMsgLength - 1U ] );

                    if( ( ucMsgLength % 16U ) == 0U ) {
                        mcu_printf( "\n" );
                    }
                }

                mcu_printf( "\n" );
                mcu_printf( "***********************************************************************************\n" );
                mcu_printf( "\n" );
				ControlBreadBoardSensors(sRxMsg.mId, sRxMsg.mDataLength, (sint8*)sRxMsg.mData);
            }
            else
            {
                mcu_printf(" [CAN DEMO] No message\n" );
                break;
            }
        }

        gReceiveFlag[ ucCh ] = 0;
    }
}

static void CAN_DemoSendReceive
(
    void
)
{
    uint8           ucCh1;
    uint8           ucCh2;
    uint8           ucMsgLength;
    uint8           ucTxMsgDlc;
    uint8           ucTxBufferIndex;
    uint32          uiTxMsgCnt;
    uint32          uiRxMsgNum;
    uint32          uiTxEvtMsgCnt;
    uint32          uiTimeout;
    CANMessage_t *  psTxMsg;
    CANMessage_t    sRxMsg;
    CANTxEvent_t    sTxEvtMsg;
    CANErrorType_t  result;

    /* the messages are sent to another channel */
    for( ucCh1 = 0U ; ucCh1 < CAN_CONTROLLER_NUMBER ; ucCh1++ )
    {
        ( void ) CAN_InitMessage( ucCh1 ); //clear remain messages

        for( uiTxMsgCnt = 0UL ; uiTxMsgCnt < CAN_MAX_TEST_MSG_NUM ; uiTxMsgCnt++ )
        {
            uiTimeout = 10;

            /* Fill data */
            for( ucTxMsgDlc = 0U ; ucTxMsgDlc < sTxMessageInfo[ uiTxMsgCnt ].mDataLength ; ucTxMsgDlc++ )
            {
                sTxMessageInfo[ uiTxMsgCnt ].mData[ ucTxMsgDlc ] = ucTxMsgDlc;
            }

            /* Send Tx message */
            psTxMsg = &sTxMessageInfo[ uiTxMsgCnt ];
            ( void ) CAN_SendMessage( ucCh1, psTxMsg, &ucTxBufferIndex );

            while( ( gCompletedFlag[ucCh1] == CAN_FLAG_FALSE ) && ( uiTimeout > 0UL ) )
            {
                CAN_PortingDelay( 1 );

                uiTimeout--;
            }

            /* Check send message */
            if( ( gCompletedFlag[ ucCh1 ] == CAN_FLAG_TRUE ) && ( uiTimeout != 0UL ) )
            {
                CAN_D( "[CAN DEMO] SEND     : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );

                uiTxEvtMsgCnt = CAN_CheckNewTxEvent( ucCh1 );
                if( 0UL < uiTxEvtMsgCnt )
                {
                    /* Get & Check Tx event message */
                    ( void ) SAL_MemSet( &sTxEvtMsg, 0, sizeof( CANTxEvent_t ) );

                    result = CAN_GetNewTxEvent( ucCh1, &sTxEvtMsg );

                    if( result == CAN_ERROR_NONE )
                    {
                        if( psTxMsg->mId == sTxEvtMsg.teId )
                        {
                            CAN_D( "[CAN DEMO] TX_EVENT : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
                        }
                        else
                        {
                            CAN_D( "[CAN DEMO] TX_EVENT : Fail(ID not match) - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
                        }
                    }
                    else
                    {
                        CAN_D( "[CAN DEMO] TX_EVENT : Fail to get Tx Event Message \r\n" );
                    }
                }

                gCompletedFlag[ucCh1] = CAN_FLAG_FALSE;
            }
            else
            {
                CAN_D( "[CAN DEMO] SEND        : Fail(Timeout) - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
            }

            /* Check receive message for other channel */
            for( ucCh2 = 0U ; ucCh2 < CAN_CONTROLLER_NUMBER ; ucCh2++ )
            {
                if( ucCh2 != ucCh1 )
                {
                    if( 0UL < gReceiveFlag[ ucCh2 ] )
                    {
                        uiRxMsgNum = CAN_CheckNewRxMessage( ucCh2 );

                        if( 0UL < uiRxMsgNum )
                        {
                            ( void ) CAN_GetNewRxMessage( ucCh2, &sRxMsg );

                            if( psTxMsg->mId == sRxMsg.mId )
                            {
                                if( sRxMsg.mDataLength == 0U )
                                {
                                    CAN_D( "[CAN DEMO] RECEIVE  : Fail(No data) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                }
                                else
                                {
                                    for( ucMsgLength = 0 ; ucMsgLength < sRxMsg.mDataLength ; ucMsgLength++ )
                                    {
                                        if( psTxMsg->mData[ ucMsgLength ] != sRxMsg.mData[ ucMsgLength ] )
                                        {
                                            CAN_D( "[CAN DEMO] RECEIVE  : Fail(Data not match) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                        }
                                    }
                                    CAN_D( "[CAN DEMO] RECEIVE  : Success - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                }
                            }
                            else
                            {
                                CAN_D( "[CAN DEMO] RECEIVE  : Fail(ID not match) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                            }
                        }

                        gReceiveFlag[ ucCh2 ] = 0;
                    }
                    else
                    {
                        CAN_D( "[CAN DEMO] RECEIVE  : Fail(Timeout) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                    }
                }
            }
            CAN_D( "\r\n" );
        }
    }
}

sint32 CAN_DemoInitialize
(
    void
)
{
    sint32          ret;
    CANErrorType_t  result;

#ifdef CAN_DEMO_RESPONSE_TEST
    uint8           ucCh; /* can response channel : 0 */
    uint8           ucTxMsgDlc;
    uint8           ucTxBufferIndex;
    CANMessage_t *  psMsg; /* can response message */

    ucCh            = 0;
    psMsg           = NULL_PTR;
#endif

    ret             = 0;

    CAN_D( "[CAN DEMO] CAN Initialize START!!\n" );

    ( void ) CAN_RegisterCallbackFunctionTx( &CAN_DemoCallbackTxEvent );

    ( void ) CAN_RegisterCallbackFunctionRx( &CAN_DemoCallbackRxEvent );

    ( void ) CAN_RegisterCallbackFunctionError( &CAN_DemoCallbackErrorEvent );

    result = CAN_Init();

    if(result == CAN_ERROR_NONE)
    {
        CAN_D( "[CAN DEMO] CAN Initialize SUCCESS\n" );

#ifdef CAN_DEMO_RESPONSE_TEST
        /* Fill data */
        for( ucTxMsgDlc = 0U ; ucTxMsgDlc < sTxPreMessageInfo.mDataLength ; ucTxMsgDlc++ )
        {
            sTxPreMessageInfo.mData[ ucTxMsgDlc ] = ucTxMsgDlc;
        }

        psMsg = &sTxPreMessageInfo;
        ( void ) CAN_SendMessage( ucCh, psMsg, &ucTxBufferIndex );
#endif

    }
    else
    {
        CAN_D( "[CAN DEMO] CAN Initialize FAIL\n" );

        result = CAN_ERROR_NOT_INIT;
    }

    CAN_D( "[CAN DEMO] CAN Initialize END!!\n" );

    if( result != CAN_ERROR_NONE ) {
        ret = -1;
    }

    return ret;
}

static void CAN_DemoUsage
(
    void
)
{
    mcu_printf( "=== USAGE INFO ===\n\n" );
    mcu_printf( "=== CMD LIST ===\n" );
    mcu_printf( " can receive start\n" );
    mcu_printf( " can receive stop\n" );
    mcu_printf( " can send\n" );
    mcu_printf( " can send receive \n" );
    mcu_printf( "\r\n" );
    mcu_printf( "=== CMD INFORMATION ===\n" );
    mcu_printf( "receive start:            Start receiving message, received message is printed via log\n" );
    mcu_printf( "receive stop:             Stop receiving message.\n" );
    mcu_printf( "send:                     Send pre defined test message\n" );
    mcu_printf( "send receive:             Integrated test, All channel send and receive\n" );
}

void CAN_DemoTest
(
    uint8                               ucArgc,
    void *                              pArgv[]
)
{
    const uint8 *   pucStr1;
    const uint8 *   pucStr2;
    sint32          ret;

    pucStr1         = NULL_PTR;
    pucStr2         = NULL_PTR;

    if( pArgv != NULL_PTR )
    {
        pucStr1 = ( const uint8 * ) pArgv[ 0 ];
        pucStr2 = ( const uint8 * ) pArgv[ 1 ];

        if( ucArgc == 1UL )
        {
            if( pucStr1 != NULL_PTR )
            {
                if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "send", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    CAN_DemoSend( 0 ); //send pre defined message from channel 0
                }
            }
            else
            {
                CAN_D( "Wrong argument\n" );

                CAN_DemoUsage();
            }
        }
        else if( ucArgc == 2UL )
        {
            if( ( pucStr1 != NULL_PTR ) && ( pucStr2 != NULL_PTR ) )
            {
                if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "receive", 7, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    if( ( SAL_StrNCmp( pucStr2, ( const uint8 * ) "start", 5, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        sTestInfo.tiRecv = TRUE;

                        CAN_D( "[CAN DEMO] CAN receive START\n" );
                    }
                    else if( ( SAL_StrNCmp( pucStr2, ( const uint8 * ) "stop", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        sTestInfo.tiRecv = FALSE;

                        CAN_D( "[CAN DEMO] CAN receive STOP\n" );
                    }
                    else
                    {
                        CAN_D( "[CAN DEMO] Wrong argument\n" );
                    }
                }
                else if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "send", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    if( ( SAL_StrNCmp( pucStr2, ( const uint8 * ) "receive", 7, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        CAN_DemoSendReceive(); //send and receive pre defined message from all channel to other channels
                    }
                    else
                    {
                        CAN_D( "Wrong argument\n" );

                        CAN_DemoUsage();
                    }
                }
                else
                {
                    CAN_D( "Wrong argument\n" );

                    CAN_DemoUsage();
                }
            }
        }
        else
        {
            CAN_D( "Wrong argument\n" );

            CAN_DemoUsage();
        }
    }
}

void CAN_DemoCreateApp
(
    void
)
{
    static uint32 uiCanDemoAppTaskID;
    static uint32 uiCanDemoAppTaskStk[CAN_DEMO_TASK_STK_SIZE];

    ( void ) SAL_TaskCreate( &uiCanDemoAppTaskID,
                   ( const uint8 * ) "Can Demo Task",
                   ( SALTaskFunc ) &CAN_DemoTask,
                   ( uint32 * const ) &uiCanDemoAppTaskStk[0],
                   CAN_DEMO_TASK_STK_SIZE,
                   SAL_PRIO_CAN_DEMO,
                   NULL_PTR);
}

static void CAN_DemoTask
(
    void *                              pArg
)
{
    uint8 ucCh;

    ( void ) pArg;

	InitSensorControls();

    while( 1 )
    {
        if( sTestInfo.tiRecv == TRUE )
        {
            for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
            {
                CAN_DemoReceive( ucCh );
            }
        }

        ( void ) SAL_TaskSleep( 1 );
    }
}

#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

