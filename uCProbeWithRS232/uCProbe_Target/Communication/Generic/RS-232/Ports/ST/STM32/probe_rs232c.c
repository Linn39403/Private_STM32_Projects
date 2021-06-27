/*
*********************************************************************************************************
*                                       uC/Probe Communication
*
*                         (c) Copyright 2007-2008; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        COMMUNICATION: RS-232
*
*                                          PORT FOR ST STM32
*
* Filename      : probe_rs232c.c
* Version       : V2.20
* Programmer(s) : BAN
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include "main.h"
#include  <probe_rs232.h>
//#include  <includes.h>


/*
*********************************************************************************************************
*                                               ENABLE
*
* Note(s) : (1) See 'probe_rs232.h  ENABLE'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)                     /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be  PROBE_RS232_UART_1]     "
  #error  "                                  [     ||  PROBE_RS232_UART_2]     "
  #error  "                                  [     ||  PROBE_RS232_UART_3]     "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_3)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be  PROBE_RS232_UART_1]     "
  #error  "                                  [     ||  PROBE_RS232_UART_2]     "
  #error  "                                  [     ||  PROBE_RS232_UART_3]     "
#endif

#ifndef    PROBE_RS232_UART_1_REMAP

  #error  "PROBE_RS232_UART_1_REMAP          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be  DEF_TRUE ]              "
  #error  "                                  [     ||  DEF_FALSE]              "
#endif

//#ifndef    PROBE_RS232_UART_2_REMAP

//  #error  "PROBE_RS232_UART_2_REMAP          not #define'd in 'probe_com_cfg.h'"
//  #error  "                                  [MUST be  DEF_TRUE ]              "
//  #error  "                                  [     ||  DEF_FALSE]              "
//#endif

//#ifndef    PROBE_RS232_UART_3_REMAP_PARTIAL

//  #error  "PROBE_RS232_UART_3_REMAP_PARTIAL  not #define'd in 'probe_com_cfg.h'"
//  #error  "                                  [MUST be  DEF_TRUE ]              "
//  #error  "                                  [     ||  DEF_FALSE]              "
//#endif

//#ifndef    PROBE_RS232_UART_3_REMAP_FULL

//  #error  "PROBE_RS232_UART_3_REMAP_FULL     not #define'd in 'probe_com_cfg.h'"
//  #error  "                                  [MUST be  DEF_TRUE ]              "
//  #error  "                                  [     ||  DEF_FALSE]              "
//#endif


/*
*********************************************************************************************************
*                                       ProbeRS232_InitTarget()
*
* Description : Initialize the UART for Probe communication.
*
* Argument(s) : baud_rate       Intended baud rate of the RS-232.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_Init().
*
* Note(s)     : (1) Though the baud rate used may vary from application to application or target to
*                   target, other communication settings are constant.  The hardware must always be
*                   configured for the following :
*
*                   (a) No parity
*                   (b) One stop bit
*                   (c) Eight data bits.
*
*               (2) Neither receive nor transmit interrupts should be enabled by this function.
*
*               (3) The following constants control the GPIO remap for the USART control lines :
*
*                        PROBE_RS232_UART_1_REMAP
*                        PROBE_RS232_UART_2_REMAP
*                        PROBE_RS232_UART_3_REMAP_PARTIAL
*                        PROBE_RS232_UART_3_REMAP_FULL
*
*                    Though the #error directives in 'LOCAL CONFIGURATION ERRORS' will require that
*                    all are defined, the value of those bearing on the USART not used will have no
*                    effect.
*
*                (4) PROBE_RS232_UART_3_REMAP_PARTIAL has precedence over PROBE_RS232_UART_3_REMAP_FULL,
*                    if both are defined to DEF_TRUE.
*********************************************************************************************************
*/
extern UART_HandleTypeDef hDiscoUart;
void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
	
  hDiscoUart.Instance = DISCOVERY_COM1; 
  hDiscoUart.Init.BaudRate = baud_rate;
  hDiscoUart.Init.WordLength = UART_WORDLENGTH_8B;
  hDiscoUart.Init.StopBits = UART_STOPBITS_1;
  hDiscoUart.Init.Parity = UART_PARITY_NONE;
  hDiscoUart.Init.Mode = UART_MODE_TX_RX;
  hDiscoUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hDiscoUart.Init.OverSampling = UART_OVERSAMPLING_16;
  hDiscoUart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hDiscoUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  BSP_COM_DeInit(COM1, &hDiscoUart);
	SET_BIT(hDiscoUart.Instance->CR3, USART_CR3_OVRDIS); //overrun dis
  BSP_COM_Init(COM1, &hDiscoUart);
}



/*
*********************************************************************************************************
*                                     ProbeRS232_RxTxISRHandler()
*
* Description : Handle Rx and Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) If this port is used in an RTOS, the interrupt entrance/exit procedure should be
*                   performed by the ISR that calls this handler.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    CPU_INT08U      rx_data;
	if(READ_BIT(hDiscoUart.Instance->ISR, USART_ISR_RXNE ) == USART_ISR_RXNE )
	{
		rx_data = hDiscoUart.Instance->RDR & 0xFF; //Reading will auto clear RXNE flag
		ProbeRS232_RxHandler(rx_data);		
	}
	if(READ_BIT(hDiscoUart.Instance->ISR, USART_ISR_TXE ) == USART_ISR_TXE )
	{
		ProbeRS232_TxHandler();
		SET_BIT(hDiscoUart.Instance->RQR, USART_RQR_TXFRQ ); //Clear TXE bit
	}
}

void USART1_IRQHandler(void)
{
	CPU_INT08U      rx_data;
  if(((hDiscoUart.Instance->ISR & USART_ISR_TC) != RESET) && ((hDiscoUart.Instance->CR1 & USART_CR1_TCIE) != RESET))
	{
		ProbeRS232_TxHandler();
		SET_BIT(hDiscoUart.Instance->RQR, USART_RQR_TXFRQ ); //Clear TXE bit
	}
	else
	{
		rx_data = hDiscoUart.Instance->RDR & 0xFF; //Reading will auto clear RXNE flag
		ProbeRS232_RxHandler(rx_data);		
	}
	if((hDiscoUart.Instance->ISR & USART_ISR_ORE) != RESET)
	{
		///overrun err
		SET_BIT(hDiscoUart.Instance->ICR, USART_ICR_ORECF);
	} 
}
/*
*********************************************************************************************************
*                                      ProbeRS232_RxISRHandler()
*
* Description : Handle Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) This function is empty because receive interrupts are handled by ProbeRS232_RxTxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
}


/*
*********************************************************************************************************
*                                        ProbeRS232_RxIntDis()
*
* Description : Disable Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The application should call this function to stop communication.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
    //USART_ITConfig(USART2, USART_IT_RXNE, DISABLE
			CLEAR_BIT(hDiscoUart.Instance->CR1, USART_CR1_RXNEIE ); 
}


/*
*********************************************************************************************************
*                                        ProbeRS232_RxIntEn()
*
* Description : Enable Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The application should call this function to start communication.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntEn (void)
{
    //USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		SET_BIT(hDiscoUart.Instance->CR1, USART_CR1_RXNEIE ); 
		NVIC_EnableIRQ(DISCOVERY_COM1_IRQn);
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TxISRHandler()
*
* Description : Handle Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : This function is empty because transmit interrupts are handled by ProbeRS232_RxTxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
}


/*
*********************************************************************************************************
*                                          ProbeRS232_Tx1()
*
* Description : Transmit one byte.
*
* Argument(s) : c           The byte to transmit.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U c)
{
	hDiscoUart.Instance->TDR = c;
}


/*
*********************************************************************************************************
*                                        ProbeRS232_TxIntDis()
*
* Description : Disable Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
    //USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	CLEAR_BIT(hDiscoUart.Instance->CR1, USART_CR1_TCIE );	
}

/*
*********************************************************************************************************
*                                        ProbeRS232_TxIntEn()
*
* Description : Enable Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxStart().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntEn (void)
{
   // USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	SET_BIT(hDiscoUart.Instance->CR1, USART_CR1_TCIE );
	NVIC_EnableIRQ(DISCOVERY_COM1_IRQn);
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
