
#include "esp01.h"

/* Communication buffers */
volatile uint8_t rx_buffer[RX_BUFFER_LENGTH];
volatile uint8_t tx_buffer[TX_BUFFER_LENGTH];

/* ESP01 state flag */
volatile ESP01_STATE esp01_flag = ESP01_WAITING;


/* UART Interruption handler */
void UART_HANDLER (void)
{
	/* This is a mini buffer where I momentarily store the read from the FIFO */
	static uint8_t rx[8];

	/* This index mantains the current position in the ring buffer */
	static uint8_t index = 0;

	/* This mantains the value of the last byte received */
	static uint8_t last_char;


	/* Read 8 bytes from the FIFO */
	Chip_UART_Read(UART_POINTER, rx, 8);


	/* Saves the data received into the ring buffer and checks for the end of frame */
	for ( uint8_t i ; i<8 ; i++) {

		rx_buffer[index] = rx[i];
		index++;
		if ( index >= RX_BUFFER_LENGTH ) { index = 0; }

		/* Checks for the OK statement */
		if ( last_char=='O' && rx[i]=='K' ) { esp01_flag = ESP01_READY; }

	}


}


/* Module initialization */
void esp01_init( void ){

	/* Initialize the UART pin */
	Chip_IOCON_PinMux(LPC_IOCON,TX_PORT,TX_PIN,IOCON_MODE_INACT,PIN_FUNCTION);
	Chip_IOCON_PinMux(LPC_IOCON,RX_PORT,RX_PIN,IOCON_MODE_INACT,PIN_FUNCTION);

	/* Set up UART in 115200 bauds, 8 bits transmission + 1 stop bit */
	Chip_UART_Init(UART_POINTER);
	Chip_UART_SetBaud(UART_POINTER, 115200);
	Chip_UART_ConfigData(UART_POINTER, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));

	/* Enable and reset FIFOs. Set interruption trigger level of 8 bytes */
	Chip_UART_SetupFIFOS(UART_POINTER, (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV2));

	/* Enable data transmission */
	Chip_UART_TXEnable(UART_POINTER);

	/* Enable UART interruption */
	Chip_UART_IntEnable(UART_POINTER, UART_IER_RBRINT);
	NVIC_SetPriority(UART_INTERRUPT, 1);
	NVIC_EnableIRQ(UART_INTERRUPT);


}


/* Send a command to the module */
uint8_t* esp01_command( uint8_t* command ){

	uint8_t aaa;
	//Chip_UART_SendByte(LPC_UART3, tx[i]);

	uint32_t i = 0;

	while ( command[i] != '\0' ){

		aaa = command[i];
		i++;
	}

	return command;
}
