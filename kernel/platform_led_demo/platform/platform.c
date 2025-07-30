#include <string.h>
#include "platform.h"

#define MAX_DEVICES 8
#define MAX_DRIVERS 8

static struct platform_device *device_table[MAX_DEVICES];
static struct platform_driver *driver_table[MAX_DRIVERS];

void platform_device_register(struct platform_device *dev) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (!device_table[i]) {
            device_table[i] = dev;
            for (int j = 0; j < MAX_DRIVERS; j++) {
                if (driver_table[j] && strcmp(driver_table[j]->name, dev->name) == 0) {
                    driver_table[j]->probe(dev);
                }
            }
            break;
        }
    }
}

void platform_driver_register(struct platform_driver *drv) {
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (!driver_table[i]) {
            driver_table[i] = drv;
            for (int j = 0; j < MAX_DEVICES; j++) {
                if (device_table[j] && strcmp(device_table[j]->name, drv->name) == 0) {
                    drv->probe(device_table[j]);
                }
            }
            break;
        }
    }
}