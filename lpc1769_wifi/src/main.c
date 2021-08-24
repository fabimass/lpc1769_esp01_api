
#include "esp01.h"

#define CLIENT


int main(void) {

	SystemCoreClockUpdate();


#ifdef HOST

	/* Access point configuration */
	ESP01_AP ap = {
			.ssid = "MyWifi",
			.pwd = "Password123",
			.chn = '6',
			.ecn = '3',
	};

	/* This is for validating the result */
	ESP01_AP ap_test;

	/* Initialization */
    if ( esp01_init()== ESP01_OK ){

    	/* Put the module to work as an access point */
    	esp01_host_mode();

    	/* Check the current access point configuration */
    	ap_test = esp01_host_check();

    	/* Set our own configuration */
    	esp01_host_config(ap);

    	/* Check correct configuration */
    	ap_test = esp01_host_check();

    }

#endif


#ifdef CLIENT

	/* Initialization */
    if ( esp01_init()== ESP01_OK ){

    	/* Put the module to work as a client */
    	esp01_client_mode();

    	/* Check for network signal */
    	if ( esp01_client_check( "MyWifi" ) == ESP01_OK ){

    		/* Connects to the access point */
    		if ( esp01_client_connect( "MyWifi", "Password123" ) == ESP01_OK ){


    			/* Here you are connected to the wifi network */

    			uint8_t led_state[1];
    			uint32_t delay;

    			Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 22);

    			while(1){

    				/* Make an http request to the server, which returns the led state */
    				if ( esp01_client_httprequest( "GET", "www.myserver.com", "8080", "/led", led_state, 1 ) == ESP01_OK ){

    			    	/* Turn on/off the led according to the server response */
    					if(led_state[0] == '0'){
    			    		Chip_GPIO_SetPinState(LPC_GPIO, 0, 22, false);
    			    	}
    			    	if(led_state[0] == '1'){
    			    	    Chip_GPIO_SetPinState(LPC_GPIO, 0, 22, true);
    			    	}

    			    	/* Set a delay for not overloading the server */
    			    	delay = 10000000;

    			    	while(delay>0){
    			    		delay--;
    			    	}

    			    }

    				else{

    					break;
    				}
    			}

    		}

    		/* Disconnects from the access point */
    		esp01_client_disconnect();

    	}

    }

#endif



    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
        // "Dummy" NOP to allow source level single
        // stepping of tight while() loop
        __asm volatile ("nop");
    }
    return 0 ;
}
