
#ifndef ESP01_H_
#define ESP01_H_



/**
 * @brief	Define the UART where the ESP01 is connected
 */
#define UART3

/**
 * @brief	Set the length of the receiver ring buffer
 */
#define RX_BUFFER_LENGTH	64


/**
 * @brief	UART Specific definitions
 */
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


/**
 * @brief	LPCOpen dependency
 */
#include "chip.h"


/**
 * @brief	Different states of the ESP01
 */
typedef enum{
	ESP01_READY,
	ESP01_BUSY,
	ESP01_OK,
	ESP01_ERROR
}ESP01_STATE;


/**
 * @brief	Access point configuration
 */
typedef struct{
	uint8_t ssid[20];    /* Name of the wifi access point (max. 20 characters) */
	uint8_t pwd[20];	 /* Password (max. 20 characters) */
	uint8_t chn[2];      /* Channel id, 1 2 3 ... 14 */
	uint8_t ecn;		 /* Encryption, 0 = OPEN, 2 = WPA_PSK, 3 = WPA2_PSK, 4 = WPA_WPA2_PSK */
}ESP01_AP;


/**
 * @brief	Initialize the ESP01
 * @return  OK if success or ERROR
 */
ESP01_STATE esp01_init( void );


/**
 * @brief	Send a command to the ESP01
 * @param	command	       : Pointer to the command to send
 * @param   numBytesToSend : Size of the passed command
 * @param	answer         : Pointer to bytes array to fill with the answer
 * @param	numBytesToRead : Size of the passed answer data array
 * @return  Nothing
 */
void esp01_command( uint8_t* command, uint32_t numBytesToSend, uint8_t* answer, uint32_t numBytesToRead );


/**
 * @brief	Set the ESP01 in host mode
 * @return  OK if success or ERROR
 */
ESP01_STATE esp01_host_mode( void );


/**
 * @brief	Check the access point settings
 * @return  Structure with the access point settings
 */
ESP01_AP esp01_host_check( void );


/**
 * @brief	Set up the access point
 * @param   settings : Structure with the access point settings to set
 * @return  OK if success or ERROR
 */
ESP01_STATE esp01_host_config( ESP01_AP settings );


/**
 * @brief	Set the ESP01 in client mode
 * @return  OK if success or ERROR
 */
ESP01_STATE esp01_client_mode( void );



#endif /* ESP01_H_ */
