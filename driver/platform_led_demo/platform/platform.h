#ifndef PLATFORM_H
#define PLATFORM_H

struct platform_device {
    const char *name;
    int id;
    void *dev_data;
};

struct platform_driver {
    const char *name;
    int (*probe)(struct platform_device *pdev);
    int (*remove)(struct platform_device *pdev);
};

void platform_device_register(struct platform_device *dev);
void platform_driver_register(struct platform_driver *drv);

#endif