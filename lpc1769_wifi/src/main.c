
#include "esp01.h"


int main(void) {

	uint32_t aa;

	ESP01_AP my_ap = {
			.ssid = "ESP_FABI",
			.pwd = "test_wifi",
			.chn = '6',
			.ecn = '3',
	};

	ESP01_AP test;

	uint8_t answer[32];

    SystemCoreClockUpdate();

    if ( esp01_init() == ESP01_OK ){

    	esp01_host_mode();

    	//esp01_host_config( my_ap );

    	esp01_command( "AT+CWSAP_DEF=\"FABI\",\"AAAABBBB\",5,3", 34, answer, sizeof(answer) );

    	test = esp01_host_check();

    }
    else{
    	aa =0;
    }


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
