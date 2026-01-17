#include "platform.h"
#include "led_driver.h"

struct led_hw {
    volatile unsigned int *port;
    unsigned int pin;
};

static struct led_hw *g_led = 0;

static int led_probe(struct platform_device *pdev) {
    g_led = (struct led_hw *)pdev->dev_data;
    led_off();
    return 0;
}

void led_on(void) {
    if (g_led)
        *(g_led->port) |= g_led->pin;
}

void led_off(void) {
    if (g_led)
        *(g_led->port) &= ~(g_led->pin);
}

static struct platform_driver led_drv = {
    .name = "led",
    .probe = led_probe,
    //.remove = 
};

void led_driver_init(void) {
    platform_driver_register(&led_drv);
}


#if 0
#define MAX_LED 4
static struct led_hw *g_leds[MAX_LED];

static int led_probe(struct platform_device *pdev) {
    int id = pdev->id;

    if (id >= MAX_LED) return -1;

    g_leds[id] = (struct led_hw *)pdev->dev_data;

    led_off(id);
    return 0;
}

#endif
