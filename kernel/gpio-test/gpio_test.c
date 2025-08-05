#if 0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>

struct gpio_test_data {
    struct gpio_desc *gpio1; // GPIO2_A2fan
    struct gpio_desc *gpio2; // GPIO1_D6kill
    struct kobject *kobj;
    struct mutex lock; // 保护操作
};

static struct gpio_test_data *g_data;

static ssize_t gpio1_value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int val = gpiod_get_value(g_data->gpio1);
    return sprintf(buf, "%d\n", val);
}

static ssize_t gpio1_value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int val;
    int ret = kstrtoint(buf, 0, &val);
    if (ret)
        return ret;

    if (val != 0 && val != 1)
        return -EINVAL;

    mutex_lock(&g_data->lock);
    gpiod_direction_output(g_data->gpio1, val);
    dev_info(NULL, "gpio1 set value %d\n", val);
    mutex_unlock(&g_data->lock);

    return count;
}

static ssize_t gpio2_value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int val = gpiod_get_value(g_data->gpio2);
    return sprintf(buf, "%d\n", val);
}

static ssize_t gpio2_value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int val;
    int ret = kstrtoint(buf, 0, &val);
    if (ret)
        return ret;

    if (val != 0 && val != 1)
        return -EINVAL;

    mutex_lock(&g_data->lock);
    gpiod_direction_output(g_data->gpio2, val);
    dev_info(NULL, "gpio2 set value %d\n", val);
    mutex_unlock(&g_data->lock);

    return count;
}

static struct kobj_attribute gpio1_value_attr =
    __ATTR(fan_ctl, 0664, gpio1_value_show, gpio1_value_store);

static struct kobj_attribute gpio2_value_attr =
    __ATTR(kill_ctl, 0664, gpio2_value_show, gpio2_value_store);

static struct attribute *gpio_attrs[] = {
    &gpio1_value_attr.attr,
    &gpio2_value_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = gpio_attrs,
};

static int gpio_test_probe(struct platform_device *pdev)
{
    int ret;

    g_data = devm_kzalloc(&pdev->dev, sizeof(*g_data), GFP_KERNEL);
    if (!g_data)
        return -ENOMEM;

    mutex_init(&g_data->lock);

    g_data->gpio1 = devm_gpiod_get(&pdev->dev, "fan", GPIOD_OUT_LOW);
    if (IS_ERR(g_data->gpio1)) {
        dev_err(&pdev->dev, "Failed to get gpio1\n");
        return PTR_ERR(g_data->gpio1);
    }

    g_data->gpio2 = devm_gpiod_get(&pdev->dev, "kill", GPIOD_OUT_HIGH);
    if (IS_ERR(g_data->gpio2)) {
        dev_err(&pdev->dev, "Failed to get gpio2\n");
        return PTR_ERR(g_data->gpio2);
    }

    // 默认值
    // gpiod_set_value(g_data->gpio1, 1);
    // gpiod_set_value(g_data->gpio2, 1);

    // 创建 sysfs kobject
    g_data->kobj = kobject_create_and_add("gpio_ctl", kernel_kobj);
    if (!g_data->kobj) {
        dev_err(&pdev->dev, "Failed to create kobject\n");
        return -ENOMEM;
    }

    ret = sysfs_create_group(g_data->kobj, &attr_group);
    if (ret) {
        dev_err(&pdev->dev, "Failed to create sysfs group\n");
        kobject_put(g_data->kobj);
        return ret;
    }

    platform_set_drvdata(pdev, g_data);

    dev_info(&pdev->dev, "gpio_test driver probed\n");
    return 0;
}

static int gpio_test_remove(struct platform_device *pdev)
{
    struct gpio_test_data *data = platform_get_drvdata(pdev);

    sysfs_remove_group(data->kobj, &attr_group);
    kobject_put(data->kobj);

    gpiod_set_value(data->gpio1, 0);
    gpiod_set_value(data->gpio2, 0);

    dev_info(&pdev->dev, "gpio_test driver removed\n");
    return 0;
}

static const struct of_device_id gpio_test_of_match[] = {
    { .compatible = "demo,gpio-test", },
    { },
};
MODULE_DEVICE_TABLE(of, gpio_test_of_match);

static struct platform_driver gpio_test_driver = {
    .probe = gpio_test_probe,
    .remove = gpio_test_remove,
    .driver = {
        .name = "gpio_test",
        .of_match_table = gpio_test_of_match,
    },
};

module_platform_driver(gpio_test_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yuji");
MODULE_DESCRIPTION("RK3568 GPIO Test Driver with sysfs");
#endif


#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>

#define GPIO_IOC_MAGIC 'G'

#define GPIO_SET_OUT1   _IOW(GPIO_IOC_MAGIC, 0, int)  // 设置 GPIO91 输出值
#define GPIO_GET_IN1    _IOR(GPIO_IOC_MAGIC, 1, int)  // 获取 GPIO93 输入值
#define GPIO_GET_IN2    _IOR(GPIO_IOC_MAGIC, 2, int)  // 获取 GPIO95 输入值


struct gpio_data {
    struct gpio_desc *in1;
    struct gpio_desc *in2;
    struct gpio_desc *out1;

    struct mutex lock;
    dev_t devt;
    struct cdev cdev;
    struct class *class;
};

static int gpio_open(struct inode *inode, struct file *file)
{
    struct gpio_data *data = container_of(inode->i_cdev, struct gpio_data, cdev);
    file->private_data = data;
    return 0;
}

static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct gpio_data *data = file->private_data;
    int val = 0;

    mutex_lock(&data->lock);

    switch (cmd) {
    case GPIO_SET_OUT1:
        if (copy_from_user(&val, (int __user *)arg, sizeof(int))) {
            mutex_unlock(&data->lock);
            return -EFAULT;
        }
        gpiod_set_value(data->out1, val ? 1 : 0);
        break;

    case GPIO_GET_IN1:
        val = gpiod_get_value(data->in1);
        if (copy_to_user((int __user *)arg, &val, sizeof(int))) {
            mutex_unlock(&data->lock);
            return -EFAULT;
        }
        break;

    case GPIO_GET_IN2:
        val = gpiod_get_value(data->in2);
        if (copy_to_user((int __user *)arg, &val, sizeof(int))) {
            mutex_unlock(&data->lock);
            return -EFAULT;
        }
        break;

    default:
        mutex_unlock(&data->lock);
        return -EINVAL;
    }

    mutex_unlock(&data->lock);
    return 0;
}

static const struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = gpio_ioctl,
    .open = gpio_open,
};

static int gpio_probe(struct platform_device *pdev)
{
    int ret;
    dev_t dev;
    struct gpio_data *data;

    data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    mutex_init(&data->lock);

    data->in1 = devm_gpiod_get(&pdev->dev, "in1", GPIOD_IN);
    if (IS_ERR(data->in1))
        return PTR_ERR(data->in1);

    data->in2 = devm_gpiod_get(&pdev->dev, "in2", GPIOD_IN);
    if (IS_ERR(data->in2))
        return PTR_ERR(data->in2);

    data->out1 = devm_gpiod_get(&pdev->dev, "out1", GPIOD_OUT_LOW);
    if (IS_ERR(data->out1))
        return PTR_ERR(data->out1);

    ret = alloc_chrdev_region(&dev, 0, 1, "gpio_ctl");
    if (ret)
        return ret;

    data->devt = dev;
    cdev_init(&data->cdev, &gpio_fops);
    ret = cdev_add(&data->cdev, dev, 1);
    if (ret) {
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    data->class = class_create(THIS_MODULE, "gpio_ctl");
    if (IS_ERR(data->class)) {
        cdev_del(&data->cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(data->class);
    }

    device_create(data->class, NULL, dev, NULL, "gpio_ctl");

    platform_set_drvdata(pdev, data);
    return 0;
}

static int gpio_remove(struct platform_device *pdev)
{
    struct gpio_data *data = platform_get_drvdata(pdev);

    device_destroy(data->class, data->devt);
    class_destroy(data->class);
    cdev_del(&data->cdev);
    unregister_chrdev_region(data->devt, 1);

    return 0;
}

static const struct of_device_id gpio_of_match[] = {
    { .compatible = "demo,gpio-test" },
    { },
};
MODULE_DEVICE_TABLE(of, gpio_of_match);

static struct platform_driver gpio_driver = {
    .probe = gpio_probe,
    .remove = gpio_remove,
    .driver = {
        .name = "gpio_ioctl",
        .of_match_table = gpio_of_match,
    },
};

module_platform_driver(gpio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yuji");
MODULE_DESCRIPTION("GPIO IOCTL Driver for GPIO91/93/95 without global variable");


#ifdef USER_APP
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
//#include "gpio_ioctl.h"

#define GPIO_IOC_MAGIC 'G'

#define GPIO_SET_OUT1   _IOW(GPIO_IOC_MAGIC, 0, int)  // 设置 GPIO91 输出值
#define GPIO_GET_IN1    _IOR(GPIO_IOC_MAGIC, 1, int)  // 获取 GPIO93 输入值
#define GPIO_GET_IN2    _IOR(GPIO_IOC_MAGIC, 2, int)  // 获取 GPIO95 输入值


#define DEV_NODE "/dev/gpio_ctl"

void usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s set <0|1>     # Set GPIO91 output low or high\n", prog);
    printf("  %s get1          # Get GPIO93 input value\n", prog);
    printf("  %s get2          # Get GPIO95 input value\n", prog);
}

int main(int argc, char *argv[])
{
    int fd, val, ret;

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    fd = open(DEV_NODE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 3) {
            usage(argv[0]);
            close(fd);
            return 1;
        }
        val = atoi(argv[2]) ? 1 : 0;
        ret = ioctl(fd, GPIO_SET_OUT1, &val);
        if (ret < 0) {
            perror("ioctl GPIO_SET_OUT1");
        } else {
            printf("Set GPIO91 to %d\n", val);
        }
    } else if (strcmp(argv[1], "get1") == 0) {
        ret = ioctl(fd, GPIO_GET_IN1, &val);
        if (ret < 0) {
            perror("ioctl GPIO_GET_IN1");
        } else {
            printf("GPIO93 = %d\n", val);
        }
    } else if (strcmp(argv[1], "get2") == 0) {
        ret = ioctl(fd, GPIO_GET_IN2, &val);
        if (ret < 0) {
            perror("ioctl GPIO_GET_IN2");
        } else {
            printf("GPIO95 = %d\n", val);
        }
    } else {
        usage(argv[0]);
    }

    close(fd);
    return 0;
}

#endif // USER_APP