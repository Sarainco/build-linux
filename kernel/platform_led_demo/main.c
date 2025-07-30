#include "led_device.h"
#include "led_driver.h"


int main(void) {
    led_device_init();
    led_driver_init();

    while (1) {
        led_on();
        usleep(100);
        led_off();
        usleep(100);
    }
}