# lpc1769_esp01_api
API for interfacing the LPC1769 Cortex micro with the ESP01 wifi module.

### Initialize the ESP01
* @return  ESP01_OK if success or ESP01_ERROR if failure

`ESP01_STATE esp01_init( void );`


### Send a command to the ESP01
 * @param	command	       : Pointer to the command to send
 * @param   numBytesToSend : Size of the passed command
 * @param	answer         : Pointer to bytes array to fill with the answer
 * @param	numBytesToRead : Size of the passed answer data array
 * @return  ESP01_OK, ESP01_ERROR or ESP01_TIMEOUT

`ESP01_STATE esp01_command( uint8_t* command, uint32_t numBytesToSend, uint8_t* answer, uint32_t numBytesToRead );`


### Set the ESP01 in host mode
 * @return  OK if success or ERROR

`ESP01_STATE esp01_host_mode( void );`


### Check the access point settings
 * @return  Structure with the access point settings

`ESP01_AP esp01_host_check( void );`


### Set up the access point
 * @param   settings : Structure with the access point settings to set
 * @return  OK if success or ERROR

`ESP01_STATE esp01_host_config( ESP01_AP settings );`


### Set the ESP01 in client mode
 * @return  OK if success or ERROR

`ESP01_STATE esp01_client_mode( void );`


### Check for a specific access point
 * @param   network: String with the network name
 * @return  OK if success or ERROR

`ESP01_STATE esp01_client_check( uint8_t* network );`


### Connect to an access point
 * @param   network: String with the network name
 * @param   password: String with the network password
 * @return  OK if success or ERROR

`ESP01_STATE esp01_client_connect( uint8_t* network, uint8_t* password );`


### Disconnect from an access point
 * @return  OK if success or ERROR
 
`ESP01_STATE esp01_client_disconnect( void );`


###	Send an http request
 * @param   request 		: This is the request type. Ex: GET, POST, PUT or DELETE
 * @param   host 			: The host that you want to communicate with. Ex: www.myserver.com
 * @param   port			: Specific port. Ex: 8080
 * @param   endpoint 		: Specific endpoint. Ex: /my_endpoint or just /
 * @param	answer         	: Pointer to bytes array to fill with the answer
 * @param	numBytesToRead 	: Size of the passed answer data array
 * @return  OK if success or ERROR

`ESP01_STATE esp01_client_httprequest( uint8_t* request, uint8_t* host, uint8_t* port, uint8_t* endpoint, uint8_t* answer, uint32_t numBytesToRead );`


###	Send data through http
 * @param   data 			: Pointer to the data to send
 * @param	numBytesToSend 	: Number of bytes to send
 * @return  OK if success or ERROR

`ESP01_STATE esp01_client_httpdata( uint8_t* data, uint32_t numBytesToSend );`
