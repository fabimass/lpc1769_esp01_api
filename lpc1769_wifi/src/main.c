
#include "esp01.h"

#define HOST


int main(void) {

	SystemCoreClockUpdate();


#ifdef HOST

	/* Access point configuration */
	ESP01_AP ap = {
			.ssid = "MY_ACCESS_POINT",
			.pwd = "test_wifi",
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
