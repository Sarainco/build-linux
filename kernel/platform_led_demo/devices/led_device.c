#include "platform.h"
#include "led_device.h"

#if 1
struct led_hw {
    volatile unsigned int *port;
    unsigned int pin;
};

static struct led_hw led1 = {
    .port = (unsigned int *)0x4001080C, // 示例 GPIO ODR 地址
    .pin = (1 << 5), // GPIOA PIN5
};

static struct platform_device led_dev = {
    .name = "led",
    .id = 0,
    .dev_data = &led1,
};

void led_device_init(void) {
    platform_device_register(&led_dev);
}

#else
struct led_hw {
    volatile unsigned int *port;
    unsigned int pin;
};

// 硬件资源数组
static struct led_hw led_hw_array[] = {
    { .port = (unsigned int *)0x4001080C, .pin = (1 << 5) }, // GPIOA5
    { .port = (unsigned int *)0x4001080C, .pin = (1 << 6) }, // GPIOA6
    { .port = (unsigned int *)0x4001080C, .pin = (1 << 7) }, // GPIOA7
};


#define LED_NUM (sizeof(led_hw_array) / sizeof(led_hw_array[0]))

// 每个 LED 对应一个 platform_device
static struct platform_device led_devs[LED_NUM];


void led_device_init(void) {
    for (int i = 0; i < LED_NUM; i++) {
        led_devs[i].name = "led";
        led_devs[i].id = i;
        led_devs[i].dev_data = &led_hw_array[i];
        platform_device_register(&led_devs[i]);
    }
}

#endif