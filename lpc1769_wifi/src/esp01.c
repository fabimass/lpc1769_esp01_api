
#include "esp01.h"

/* Ring buffer to receive data */
volatile uint8_t rx_buffer[RX_BUFFER_LENGTH];

/* This index mantains the current position in the ring buffer */
volatile uint8_t index = 0;

/* ESP01 state flag */
volatile ESP01_STATE esp01_flag = ESP01_READY;


/* UART Interruption handler */
void UART_HANDLER (void)
{
	/* This is a mini buffer where I momentarily store the read from the FIFO */
	static uint8_t rx[8];


	/* This mantains the value of the last byte received */
	static uint8_t last_char;


	/* Read 8 bytes from the FIFO */
	Chip_UART_Read(UART_POINTER, rx, 8);


	/* Saves the data received into the ring buffer and checks for the end of frame */
	for ( uint8_t i=0 ; i<8 ; i++) {

		if ( esp01_flag == ESP01_BUSY )
			rx_buffer[index] = rx[i];
		else
			rx_buffer[index] = '\0';

		index++;
		if ( index >= RX_BUFFER_LENGTH ) { index = 0; }

		/* Checks for the OK statement */
		if ( last_char=='O' && rx[i]=='K' ) { esp01_flag = ESP01_READY; }

		/* Update the last byte analyzed */
		last_char = rx[i];

	}


}


/* Module initialization */
ESP01_STATE esp01_init( void ){

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


	uint8_t answer[4];

	/* Clean answer array */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) { answer[i]='\0'; }

	/* Disabling echo */
	esp01_command( "ATE0", 4, answer, sizeof(answer) );

	/* Clean answer array */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) { answer[i]='\0'; }

	/* Check communication */
	esp01_command( "AT", 2, answer, sizeof(answer) );

	/* Check response */
	if ( answer[0]=='\r' && answer[1]=='\n' && answer[2]=='O' && answer[3]=='K' )
		return ESP01_OK;

	else
		return ESP01_ERROR;

}


/* Send a command to the module */
void esp01_command( uint8_t* command, uint32_t numBytesToSend, uint8_t* answer, uint32_t numBytesToRead ){

	/* Saves the current position in the ring buffer */
	uint32_t start=index;

	/* Set the state to busy before starting the communication */
	esp01_flag = ESP01_BUSY;

	/* Go trough the string received sending each byte through the UART */
	for ( uint32_t i=0 ; i<numBytesToSend ; i++ ){

		Chip_UART_SendByte(UART_POINTER, command[i]);
	}

	/* The command has to end with \r\n in order to be recognized by the ESP01 */
	Chip_UART_SendByte(UART_POINTER, '\r');
	Chip_UART_SendByte(UART_POINTER, '\n');

	/* Wait until it receives the answer */
	while ( esp01_flag != ESP01_READY );

	/* Fill the passed array */
	for ( uint32_t i=0 ; i<numBytesToRead ; i++){

		answer[i] = rx_buffer[start + i];
	}

}


/* Set the ESP01 in host mode */
ESP01_STATE esp01_host_mode( void ){

	uint8_t answer[4];

	/* Clean answer array */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) { answer[i]='\0'; }

	/* Configure the module to act as an access point */
	esp01_command( "AT+CWMODE=2", 11, answer, sizeof(answer) );

	/* Check response */
	if ( answer[0]=='\r' && answer[1]=='\n' && answer[2]=='O' && answer[3]=='K' )
		return ESP01_OK;

	else
		return ESP01_ERROR;
}


/* Check the access point settings */
ESP01_STATE esp01_host_check( void ){

	uint8_t answer[64];

	/* Clean answer array */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) { answer[i]='\0'; }

	/* Query configuration of ESP01 softAP mode */
	esp01_command( "AT+CWSAP?", 9, answer, sizeof(answer) );

	/* Check response */
	if ( answer[0]=='\r' && answer[1]=='\n' && answer[2]=='O' && answer[3]=='K' )
		return ESP01_OK;

	else
		return ESP01_ERROR;
}


/* Set the ESP01 in client mode */
ESP01_STATE esp01_client_mode( void ){

	uint8_t answer[4];

	/* Clean answer array */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) { answer[i]='\0'; }

	/* Configure the module to act as a client */
	esp01_command( "AT+CWMODE=1", 11, answer, sizeof(answer) );

	/* Check response */
	if ( answer[0]=='\r' && answer[1]=='\n' && answer[2]=='O' && answer[3]=='K' )
		return ESP01_OK;

	else
		return ESP01_ERROR;

}
