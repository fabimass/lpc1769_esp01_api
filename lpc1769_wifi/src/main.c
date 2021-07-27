
#include "esp01.h"


int main(void) {


    SystemCoreClockUpdate();

    esp01_init();

    esp01_command("ATFA");

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
