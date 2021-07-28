
#include "esp01.h"


int main(void) {

	uint32_t aa;

    SystemCoreClockUpdate();

    if ( esp01_init() == ESP01_OK ){

    	esp01_host_mode();

    }

    //uint8_t answer[RX_BUFFER_LENGTH];
    //esp01_command("AT", answer, RX_BUFFER_LENGTH);

    //for (uint32_t i=0; i<RX_BUFFER_LENGTH; i++){ answer[i]=test[i]; }

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
