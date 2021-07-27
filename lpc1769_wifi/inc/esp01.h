
#ifndef ESP01_H_
#define ESP01_H_


/* *******************************************
 *           User customizable
 *********************************************/

/* Define the UART where the ESP01 is connected */
#define UART3

/* Set the length of the buffers */
#define RX_BUFFER_LENGTH	64
#define TX_BUFFER_LENGTH	16



/* *******************************************
 *           Private declarations
 *********************************************/

/* UART specific definitions */

#ifdef UART0
#define UART_POINTER 	LPC_UART0
#define UART_HANDLER	UART0_IRQHandler
#define UART_INTERRUPT	UART0_IRQn
#define TX_PORT 0
#define TX_PIN  2
#define RX_PORT 0
#define RX_PIN  3
#define PIN_FUNCTION	IOCON_FUNC1
#endif

#ifdef UART1
#define UART_POINTER 	LPC_UART1
#define UART_HANDLER	UART1_IRQHandler
#define UART_INTERRUPT	UART1_IRQn
#define TX_PORT 0
#define TX_PIN  15
#define RX_PORT 0
#define RX_PIN  16
#define PIN_FUNCTION	IOCON_FUNC1
#endif

#ifdef UART2
#define UART_POINTER 	LPC_UART2
#define UART_HANDLER	UART2_IRQHandler
#define UART_INTERRUPT	UART2_IRQn
#define TX_PORT 0
#define TX_PIN  10
#define RX_PORT 0
#define RX_PIN  11
#define PIN_FUNCTION	IOCON_FUNC1
#endif

#ifdef UART3
#define UART_POINTER 	LPC_UART3
#define UART_HANDLER	UART3_IRQHandler
#define UART_INTERRUPT	UART3_IRQn
#define TX_PORT 0
#define TX_PIN  0
#define RX_PORT 0
#define RX_PIN  1
#define PIN_FUNCTION	IOCON_FUNC2
#endif


/* LPCOpen dependency */
#include "chip.h"


/* Different states of the ESP01 flag */
typedef enum{
	ESP01_READY,
	ESP01_WAITING
}ESP01_STATE;




/* *******************************************
 *           Available functions
 *********************************************/

/* Module initialization */
void esp01_init( void );

/* Send a command to the module */
uint8_t* esp01_command( uint8_t* command );



#endif /* ESP01_H_ */
