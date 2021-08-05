
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

	/* Set up UART in 9600 bauds, 8 bits transmission + 1 stop bit */
	Chip_UART_Init(UART_POINTER);
	Chip_UART_SetBaud(UART_POINTER, 9600);
	Chip_UART_ConfigData(UART_POINTER, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));

	/* Enable and reset FIFOs. Set interruption trigger level of 8 bytes */
	Chip_UART_SetupFIFOS(UART_POINTER, (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV2));

	/* Enable data transmission */
	Chip_UART_TXEnable(UART_POINTER);

	/* Enable UART interruption */
	Chip_UART_IntEnable(UART_POINTER, UART_IER_RBRINT);
	NVIC_SetPriority(UART_INTERRUPT, 1);
	NVIC_EnableIRQ(UART_INTERRUPT);


	uint8_t answer[8];


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

	uint32_t timeout_counter = 10000000;

	/* Wait until it receives the answer */
	while ( esp01_flag == ESP01_BUSY ){

		timeout_counter--;

		if ( timeout_counter==0 ){ esp01_flag = ESP01_ERROR; }

	}

	uint32_t j=0;

	/* Fill the passed array */
	for ( uint32_t i=0 ; i<numBytesToRead ; i++){

		if ( (start + j) >= RX_BUFFER_LENGTH ){
			start = 0;
			j = 0;
		}
		answer[i] = rx_buffer[start + j];
		j++;
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
ESP01_AP esp01_host_check( void ){

	uint8_t answer[96];

	/* Clean answer array */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) { answer[i]='\0'; }

	/* Query configuration of ESP01 softAP mode */
	esp01_command( "AT+CWSAP?", 9, answer, sizeof(answer) );

	uint32_t delimiters[4];
	uint32_t j=0;

	/* Find the delimiters inside the response */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) {

		if ( answer[i]==':' || answer[i]==',' ){

			delimiters[j] = i;
			j++;
			if (j>=4) break;
		}

	}

	ESP01_AP ap_settings;

	/* Initialize the structure clean */
	ap_settings.chn[0] = '\0';
	ap_settings.chn[1] = '\0';
	ap_settings.ecn = '\0';
	for ( uint32_t i=0 ; i<20 ; i++ ){
		ap_settings.ssid[i] = '\0';
		ap_settings.pwd[i] = '\0';
	}

	/* Get the network name */
	for( uint32_t i=0 ; i<delimiters[1]-delimiters[0]-3 ; i++ ){ ap_settings.ssid[i]=answer[delimiters[0]+2+i]; }

	/* Get the network password */
	for( uint32_t i=0 ; i<delimiters[2]-delimiters[1]-3 ; i++ ){ ap_settings.pwd[i]=answer[delimiters[1]+2+i]; }

	/* Get the channel used */
	for( uint32_t i=0 ; i<delimiters[3]-delimiters[2]-1 ; i++ ){ ap_settings.chn[i]=answer[delimiters[2]+1+i]; }

	/* Get the network security */
	ap_settings.ecn = answer[delimiters[3]+1];


	return ap_settings;
}


/* Check the access point settings */
ESP01_STATE esp01_host_config( ESP01_AP settings ){

	uint8_t answer[4];

	/* Clean answer array */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) { answer[i]='\0'; }

	uint8_t command[96];

	/* Construct the command */
	command[0] = 'A';
	command[1] = 'T';
	command[2] = '+';
	command[3] = 'C';
	command[4] = 'W';
	command[5] = 'S';
	command[6] = 'A';
	command[7] = 'P';
	command[8] = '=';
	command[9] = '"';

	uint32_t j = 0;
	while ( settings.ssid[j] != '\0' ){
		command[j+10] = settings.ssid[j];
		j++;
	}

	command[j+10] = '"';
	command[j+11] = ',';
	command[j+12] = '"';

	uint32_t k = 0;
	while ( settings.pwd[k] != '\0' ){
		command[j+13+k] = settings.pwd[k];
		k++;
	}

	command[j+13+k] = '"';
	command[j+14+k] = ',';

	uint32_t l = 0;
	while ( settings.chn[l] != '\0' ){
		command[j+15+k+l] = settings.chn[l];
		l++;
	}

	command[j+15+k+l] = ',';
	command[j+16+k+l] = settings.ecn;


	/* Set configuration of softAP mode */
	esp01_command( command, j+16+k+l+1, answer, sizeof(answer) );

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
