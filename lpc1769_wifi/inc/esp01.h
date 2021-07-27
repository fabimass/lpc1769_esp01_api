
#ifndef ESP01_H_
#define ESP01_H_

/* Define the UART where the ESP01 is connected */
#define UART3


/* UART specific definitions */

#ifdef UART0
#define UART_POINTER 	LPC_UART0
#define UART_HANDLER	UART0_IRQHandler
#define TX_PORT 0
#define TX_PIN  2
#define RX_PORT 0
#define RX_PIN  3
#endif

#ifdef UART1
#define UART_POINTER 	LPC_UART1
#define UART_HANDLER	UART1_IRQHandler
#define TX_PORT 0
#define TX_PIN  15
#define RX_PORT 0
#define RX_PIN  16
#endif

#ifdef UART2
#define UART_POINTER 	LPC_UART2
#define UART_HANDLER	UART2_IRQHandler
#define TX_PORT 0
#define TX_PIN  10
#define RX_PORT 0
#define RX_PIN  11
#endif

#ifdef UART3
#define UART_POINTER 	LPC_UART3
#define UART_HANDLER	UART3_IRQHandler
#define TX_PORT 0
#define TX_PIN  0
#define RX_PORT 0
#define RX_PIN  1
#endif


/* LPCOpen dependency */
#include "chip.h"


#define RX_BUFFER_LENGTH	64
#define TX_BUFFER_LENGTH	16

typedef enum{
	OK,
	WAITING
}ESP01_STATE;


#endif /* ESP01_H_ */
