
#include "esp01.h"

/* Communication buffers */
uint8_t rx_buffer[RX_BUFFER_LENGTH];
uint8_t tx_buffer[TX_BUFFER_LENGTH];

/* ESP01 state flag */
uint8_t esp01_flag = ESP01_WAITING;


/* UART Interruption handler */
void UART3_IRQHandler(void)
{
	/* This is a mini buffer where I momentarily store the read from the FIFO */
	static uint8_t rx[8];

	/* This index mantains the current position in the ring buffer */
	static uint8_t index = 0;

	/* This mantains the value of the last byte received */
	static uint8_t last_char;


	/* Read 8 bytes from the FIFO */
	Chip_UART_Read(LPC_UART3, rx, 8);


	/* Saves the data received into the ring buffer and checks for the end of frame */
	for ( uint8_t i ; i<8 ; i++) {

		rx_buffer[index] = r[i];
		index++;
		if ( index >= RX_BUFFER_LENGTH ) { index = 0; }

		/* Checks for the OK statement */
		if ( last_char=='O' && rx[i]=='K' ) { esp01_flag = ESP01_OK; }

	}


}
