
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

	/* Arrays for response checking */
	static uint8_t r_ok[] = "OK";
	static uint8_t r_error[] = "ERROR";
	static uint8_t r_fail[] = "FAIL";
	static uint8_t r_closed[] = "CLOSED";
	static uint8_t index_ok = 0;
	static uint8_t index_error = 0;
	static uint8_t index_fail = 0;
	static uint8_t index_closed = 0;


	/* Read 8 bytes from the FIFO */
	Chip_UART_Read(UART_POINTER, rx, 8);


	/* Saves the data received into the ring buffer and checks for the end of frame */
	for ( uint8_t i=0 ; i<8 ; i++) {

		rx_buffer[index] = rx[i];

		index++;
		if ( index >= RX_BUFFER_LENGTH ) { index = 0; }


		/* Checks for the OK statement */
		if ( rx[i] == r_ok[index_ok] ){

			index_ok++;
			if ( index_ok >= sizeof(r_ok)-1 ){
				esp01_flag = ESP01_OK;
			}

		}
		else{
			index_ok = 0;
		}

		/* Checks for the ERROR statement */
		if ( rx[i] == r_error[index_error] ){

			index_error++;
			if ( index_error >= sizeof(r_error)-1 ){
				esp01_flag = ESP01_ERROR;
			}

		}
		else{
			index_error = 0;
		}

		/* Checks for the FAIL statement */
		if ( rx[i] == r_fail[index_fail] ){

			index_fail++;
			if ( index_fail >= sizeof(r_fail)-1 ){
				esp01_flag = ESP01_ERROR;
			}

		}
		else{
			index_fail = 0;
		}

		/* Checks for the CLOSED statement */
		if ( rx[i] == r_closed[index_closed] ){

			index_closed++;
			if ( index_closed >= sizeof(r_closed)-1 ){
				esp01_flag = ESP01_CLOSED;
			}

		}
		else{
			index_closed = 0;
		}


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
	Chip_UART_ConfigData(UART_POINTER, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));

	/* Enable and reset FIFOs. Set interruption trigger level of 8 bytes */
	Chip_UART_SetupFIFOS(UART_POINTER, (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV2));

	/* Enable data transmission */
	Chip_UART_TXEnable(UART_POINTER);

	/* Enable UART interruption */
	Chip_UART_IntEnable(UART_POINTER, UART_IER_RBRINT);
	NVIC_SetPriority(UART_INTERRUPT, 1);
	NVIC_EnableIRQ(UART_INTERRUPT);


	uint8_t answer[8];


	/* Disabling echo */
	if ( esp01_command( "ATE0", 4, answer, sizeof(answer) ) != ESP01_OK ) { return ESP01_ERROR; }


	/* Check communication */
	if ( esp01_command( "AT", 2, answer, sizeof(answer) ) != ESP01_OK ) { return ESP01_ERROR; }


	return ESP01_OK;

}


/* Send a command to the module */
ESP01_STATE esp01_command( uint8_t* command, uint32_t numBytesToSend, uint8_t* answer, uint32_t numBytesToRead ){

	/* Saves the current position in the ring buffer */
	uint32_t start=index;

	/* Set the state to busy before starting the communication */
	esp01_flag = ESP01_BUSY;

	/* Go trough the string received sending each byte through the UART */
	for ( uint32_t i=0 ; i<numBytesToSend+2 ; i++ ){

		/* Add a delay every 16 bytes, to let the FIFO empty */
		if ( (i!=0) && (i%16==0) ){
			for (uint32_t delay=0 ; delay<1000000 ; delay++);
		}

		/* The command has to end with \r\n in order to be recognized by the ESP01 */
		if ( i>=numBytesToSend ) {

			if ( i==numBytesToSend ) { Chip_UART_SendByte(UART_POINTER, '\r'); }
			if ( i==numBytesToSend+1 ) { Chip_UART_SendByte(UART_POINTER, '\n'); }
		}

		else{
			Chip_UART_SendByte(UART_POINTER, command[i]);
		}

	}


	uint32_t timeout_counter = 100000000;

	/* Wait until it receives the answer */
	while ( esp01_flag == ESP01_BUSY ){

		timeout_counter--;

		if ( timeout_counter==0 ){ esp01_flag = ESP01_TIMEOUT; }

	}

	if ( esp01_flag == ESP01_OK ){

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

		esp01_flag = ESP01_READY;
		return ESP01_OK;
	}

	if ( esp01_flag == ESP01_ERROR ){
		esp01_flag = ESP01_READY;
		return ESP01_ERROR;
	}

	if ( esp01_flag == ESP01_TIMEOUT ){
		esp01_flag = ESP01_READY;
		return ESP01_TIMEOUT;
	}

}


/* Set the ESP01 in host mode */
ESP01_STATE esp01_host_mode( void ){

	uint8_t answer[4];

	/* Configure the module to act as an access point */
	if ( esp01_command( "AT+CWMODE=2", 11, answer, sizeof(answer) ) != ESP01_OK ) { return ESP01_ERROR; };


	return ESP01_OK;

}


/* Check the access point settings */
ESP01_AP esp01_host_check( void ){

	uint8_t answer[96];

	/* Clean answer array */
	for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) { answer[i]='\0'; }

	ESP01_AP ap_settings;

	/* Initialize the structure clean */
	ap_settings.chn[0] = '\0';
	ap_settings.chn[1] = '\0';
	ap_settings.ecn = '\0';
	for ( uint32_t i=0 ; i<20 ; i++ ){
		ap_settings.ssid[i] = '\0';
		ap_settings.pwd[i] = '\0';
	}

	/* Query configuration of ESP01 softAP mode */
	if ( esp01_command( "AT+CWSAP?", 9, answer, sizeof(answer) ) == ESP01_OK ){

		uint32_t delimiters[] = {0,0,0,0};
		uint32_t j=0;

		/* Find the delimiters inside the response */
		for ( uint32_t i=0 ; i<sizeof(answer) ; i++ ) {

			if ( (answer[i]==':' && j==0) || (answer[i]==',' && j>0) ){

				delimiters[j] = i;
				j++;
				if (j>=4) break;
			}

		}

		/* Get the network name */
		if ( (delimiters[1] > delimiters[0]) && (delimiters[1] < sizeof(answer)) )
			for( uint32_t i=0 ; i<delimiters[1]-delimiters[0]-3 ; i++ ){ ap_settings.ssid[i]=answer[delimiters[0]+2+i]; }

		/* Get the network password */
		if ( (delimiters[2] > delimiters[1]) && (delimiters[2] < sizeof(answer)) )
			for( uint32_t i=0 ; i<delimiters[2]-delimiters[1]-3 ; i++ ){ ap_settings.pwd[i]=answer[delimiters[1]+2+i]; }

		/* Get the channel used */
		if ( (delimiters[3] > delimiters[2]) && (delimiters[3] < sizeof(answer)) )
			for( uint32_t i=0 ; i<delimiters[3]-delimiters[2]-1 ; i++ ){ ap_settings.chn[i]=answer[delimiters[2]+1+i]; }

		/* Get the network security */
		if ( (delimiters[3] < sizeof(answer)) )
			ap_settings.ecn = answer[delimiters[3]+1];


	}

	return ap_settings;
}


/* Set the access point settings */
ESP01_STATE esp01_host_config( ESP01_AP settings ){

	uint8_t answer[4];

	uint8_t command[56 + MAX_SSID + MAX_PWD];

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

	uint32_t j = 10;
	uint32_t k = 0;

	/* Add the network name */
	while ( settings.ssid[k] != '\0' && k<MAX_SSID){
		command[j] = settings.ssid[k];
		j++;
		k++;
	}

	command[j] = '"';
	j++;
	command[j] = ',';
	j++;
	command[j] = '"';
	j++;

	uint32_t l = 0;

	/* Add the password */
	while ( settings.pwd[l] != '\0' && l<MAX_PWD){
		command[j] = settings.pwd[l];
		j++;
		l++;
	}

	command[j] = '"';
	j++;
	command[j] = ',';
	j++;

	uint32_t m = 0;

	/* Add the wifi channel */
	while ( settings.chn[m] != '\0' && m<2){
		command[j] = settings.chn[m];
		j++;
		m++;
	}

	command[j] = ',';
	j++;
	command[j] = settings.ecn;
	j++;


	/* Set configuration of softAP mode */
	if ( esp01_command( command, j, answer, sizeof(answer) ) != ESP01_OK ) { return ESP01_ERROR; }

	return ESP01_OK;

}


/* Set the ESP01 in client mode */
ESP01_STATE esp01_client_mode( void ){

	uint8_t answer[4];

	/* Configure the module to act as a client */
	if ( esp01_command( "AT+CWMODE=1", 11, answer, sizeof(answer) ) != ESP01_OK ) { return ESP01_ERROR; }

	return ESP01_OK;

}


/* Check for a specific access point */
ESP01_STATE esp01_client_check( uint8_t* network ){

	uint8_t command[11 + MAX_SSID];

	/* Construct the command */
	command[0] = 'A';
	command[1] = 'T';
	command[2] = '+';
	command[3] = 'C';
	command[4] = 'W';
	command[5] = 'L';
	command[6] = 'A';
	command[7] = 'P';
	command[8] = '=';
	command[9] = '"';

	uint32_t j = 10;
	uint32_t k = 0;

	/* Add the network name */
	while ( network[k] != '\0' && k<MAX_SSID){
		command[j] = network[k];
		j++;
		k++;
	}

	command[j] = '"';
	j++;

	uint8_t answer[96];

	/* Check signal */
	if ( esp01_command( command, j, answer, sizeof(answer) ) != ESP01_OK ) { return ESP01_ERROR; }


	uint32_t index=0;
	uint32_t i=0;

	/* Checks for the network name in the answer */
	for ( i=0 ; i<sizeof(answer) ; i++ ){

		if ( answer[i] == network[index] ){

			index++;
			if ( index >= sizeof(network)-1 ){
				esp01_flag = ESP01_OK;
				break;
			}

		}
		else{
			index = 0;
		}
	}

	if ( i==sizeof(answer) ) { return ESP01_ERROR; }

	return ESP01_OK;

}


/* Connect to an access point */
ESP01_STATE esp01_client_connect( uint8_t* network, uint8_t* password ){

	uint8_t command[11 + MAX_SSID + MAX_PWD];

	/* Construct the command */
	command[0] = 'A';
	command[1] = 'T';
	command[2] = '+';
	command[3] = 'C';
	command[4] = 'W';
	command[5] = 'J';
	command[6] = 'A';
	command[7] = 'P';
	command[8] = '=';
	command[9] = '"';

	uint32_t j=10;
	uint32_t k=0;

	/* Add the network name */
	while ( network[k] != '\0' && k<MAX_SSID){
		command[j] = network[k];
		j++;
		k++;
	}

	command[j] = '"';
	j++;
	command[j] = ',';
	j++;
	command[j] = '"';
	j++;

	uint32_t l = 0;

	/* Add the password */
	while ( password[l] != '\0' && l<MAX_PWD){
		command[j] = password[l];
		j++;
		l++;
	}

	command[j] = '"';
	j++;

	uint8_t answer[96];

	/* Connects to the AP */
	if ( esp01_command( command, j, answer, sizeof(answer) ) != ESP01_OK ) { return ESP01_ERROR; }

	return ESP01_OK;

}


/* Disconnect from an access point */
ESP01_STATE esp01_client_disconnect( void ){

	uint8_t answer[4];

	/* Disconnects from the AP */
	if ( esp01_command( "AT+CWQAP", 8, answer, sizeof(answer) ) != ESP01_OK ) { return ESP01_ERROR; }

	return ESP01_OK;

}


/* Send an http request */
ESP01_STATE esp01_client_httprequest( uint8_t* request, uint8_t* host, uint8_t* port, uint8_t* endpoint, uint8_t* answer, uint32_t numBytesToRead ){

	uint8_t answer1[4];

	/* Configures single connection mode */
	if ( esp01_command( "AT+CIPMUX=0", 11, answer1, sizeof(answer1) ) != ESP01_OK ) { return ESP01_ERROR; }


	uint8_t command[ 21 + MAX_HOST + MAX_PORT ];

	/* Construct the command */
	command[0] = 'A';
	command[1] = 'T';
	command[2] = '+';
	command[3] = 'C';
	command[4] = 'I';
	command[5] = 'P';
	command[6] = 'S';
	command[7] = 'T';
	command[8] = 'A';
	command[9] = 'R';
	command[10] = 'T';
	command[11] = '=';
	command[12] = '"';
	command[13] = 'T';
	command[14] = 'C';
	command[15] = 'P';
	command[16] = '"';
	command[17] = ',';
	command[18] = '"';

	uint32_t j=19;
	uint32_t k=0;

	/* Add the host name */
	while ( host[k] != '\0' && k<MAX_HOST){
		command[j] = host[k];
		j++;
		k++;
	}

	command[j] = '"';
	j++;
	command[j] = ',';
	j++;

	uint32_t l = 0;

	/* Add the port */
	while ( port[l] != '\0' && l<MAX_PORT){
		command[j] = port[l];
		j++;
		l++;
	}

	uint8_t answer2[16];

	/* Establishes TCP connection */
	if ( esp01_command( command, j, answer2, sizeof(answer2) ) != ESP01_OK ) { return ESP01_ERROR; }


	/* Construct the data */

	uint8_t data[ 12 + MAX_REQ + MAX_ENDP ];

	j=0;
	k=0;

	/* Start by adding the request type */
	while ( request[k] != '\0' && k<MAX_REQ){
		data[j] = request[k];
		j++;
		k++;
	}

	data[j] = ' ';
	j++;

	l=0;

	/* Add the endpoint */
	while ( endpoint[l] != '\0' && l<MAX_ENDP){
		data[j] = endpoint[l];
		j++;
		l++;
	}

	data[j] = ' ';
	j++;
	data[j] = 'H';
	j++;
	data[j] = 'T';
	j++;
	data[j] = 'T';
	j++;
	data[j] = 'P';
	j++;
	data[j] = '/';
	j++;
	data[j] = '1';
	j++;
	data[j] = '.';
	j++;
	data[j] = '1';
	j++;
	data[j] = '\r';
	j++;
	data[j] = '\n';
	j++;

	/* Convert j to ASCII */
	uint8_t j_ascii[] = { '0' , '0' , '0' };
	uint8_t j_dec[] = { 0 , 0 , 0 };

	uint32_t value = j+2;

	j_dec[0] = value / 100;
	value = value % 100;

	j_dec[1] = value / 10;
	value = value % 10;

	j_dec[2] = value;

	for ( uint32_t i=0 ; i<3 ; i++ ){
		j_ascii[i] = j_dec[i] + 48;
	}


	/* Construct the command */

	uint8_t command2[14];

	command2[0] = 'A';
	command2[1] = 'T';
	command2[2] = '+';
	command2[3] = 'C';
	command2[4] = 'I';
	command2[5] = 'P';
	command2[6] = 'S';
	command2[7] = 'E';
	command2[8] = 'N';
	command2[9] = 'D';
	command2[10] = '=';

	uint32_t i = 11;

	if (j >= 100){
		command2[i] = j_ascii[0];
		i++;
	}

	if (j >= 10){
		command2[i] = j_ascii[1];
		i++;
	}

	command2[i] = j_ascii[2];
	i++;


	uint8_t answer3[4];
	uint8_t answer4[96];

	/* Send data */
	if ( esp01_command( command2, i, answer3, sizeof(answer3) ) != ESP01_OK ) { return ESP01_ERROR; }
	if ( esp01_client_httpdata( data, j ) != ESP01_OK ) { return ESP01_ERROR; }

	/* Search for the word CLOSED in the response that indicates the end of the frame */
	uint8_t word_to_match[] = "CLOSED";
	uint32_t matches = 0;
	uint32_t match_flag = 0;
	uint32_t match_pos = 0;

	for (uint32_t z=0 ; z<RX_BUFFER_LENGTH ; z++){

		for ( uint32_t x=0 ; x<6 ; x++ ){

			if ( (z+x) >= RX_BUFFER_LENGTH ){

				if ( rx_buffer[z+x-RX_BUFFER_LENGTH] == word_to_match[x] ){

					matches++;
					if (matches == 6){
						match_flag = 1;
						match_pos = z;
					}
				}
				else{
					matches=0;
				}
			}
			else{

				if ( rx_buffer[z+x] == word_to_match[x] ){

					matches++;
					if (matches == 6){
						match_flag = 1;
						match_pos = z;
					}
				}
				else{
					matches=0;
				}
			}


		}

	}

	/* Once we found the word, we have to read as many previous characters as the value indicated by numBytesToRead */
	if (match_flag == 1){

		for (uint32_t y=0 ; y<numBytesToRead ; y++){

			answer[y] = rx_buffer[match_pos-numBytesToRead+y];
		}

		return ESP01_OK;

	}

	else{

		return ESP01_ERROR;
	}

}


/* Send data through http */
ESP01_STATE esp01_client_httpdata( uint8_t* data, uint32_t numBytesToSend ){

	/* Set the state to busy before starting the communication */
	esp01_flag = ESP01_BUSY;

	/* Go trough the string received sending each byte through the UART */
	for ( uint32_t i=0 ; i<numBytesToSend+2 ; i++ ){

		/* Add a delay every 16 bytes, to let the FIFO empty */
		if ( (i!=0) && (i%16==0) ){
			for (uint32_t delay=0 ; delay<1000000 ; delay++);
		}

		/* The command has to end with \r\n in order to be recognized by the ESP01 */
		if ( i>=numBytesToSend ) {

			if ( i==numBytesToSend ) { Chip_UART_SendByte(UART_POINTER, '\r'); }
			if ( i==numBytesToSend+1 ) { Chip_UART_SendByte(UART_POINTER, '\n'); }
		}

		else{
			Chip_UART_SendByte(UART_POINTER, data[i]);
		}

	}

	uint32_t timeout_counter = 100000000;

	/* Wait the connection is closed */
	while ( esp01_flag != ESP01_CLOSED && esp01_flag != ESP01_TIMEOUT ){

		timeout_counter--;

		if ( timeout_counter==0 ){ esp01_flag = ESP01_TIMEOUT; }

	}

	if ( esp01_flag == ESP01_CLOSED ){

		return ESP01_OK;
	}
	else{

		return ESP01_ERROR;
	}
}

