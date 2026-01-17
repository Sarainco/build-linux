#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/semaphore.h>

#define GPIOLED_CNT			1		  	/* 设备号个数 */
#define GPIOLED_NAME		"gpioled"	/* 名字 */
#define LEDOFF 				0			/* 关灯 */
#define LEDON 				1			/* 开灯 */

struct gpioled_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    int led_gpio;
#ifdef ATOMIC_LOCK_T
    atomic_t lock;  //原子锁;一般原子操作作用于变量或者位操作；整形数据；原子操作只能对整形变量或者位进行保护
#endif

#ifdef SPIN_LOCK_T
    int dev_stats;
    spinlock_t lock;
#endif // SPIN_LOCK_T

#ifdef SEM_LOCK_T
    struct semaphore sem;
#endif //

#ifdef MUTEX_LOCK_T
    struct mutex lock;
#endif // DEBUG
};

struct gpioled_dev gpioled;

static int led_open(struct inode *inode, struct file *filp)
{
#ifdef ATOMIC_LOCK_T
    if(!atomic_dec_and_test(&gpioled.lock)) {
        atomic_inc(&gpioled.lock);
        return -EBUSY;
    }
#endif
#ifdef SPIN_LOCK_T
    unsigned log flags;
    spin_lock_irqsave(&gpioled.lock, flags);//上锁
    if(gpioled.dev_stats) {
        spin_unlock_irqrestore(&gpioled.lock, flags);//解锁
        return -EBUSY;
    }
    gpioled.dev_stats++;
    spin_unlock_irqrestore(&gpioled.lock, flags);
#endif // SPIN_LOCK_T

#ifdef SEM_LOCK_T
    if(down_interruptible(&gpioled.sem)) {
        return -ERESTARTSYS;
    }
#endif //

#ifdef MUTEX_LOCK_T
    if(mutex_lock_interruptible(&gpioled.lock)) {
        return -ERESTARTSYS;
    }
#endif //DEBUG

    filp->private_data = &gpioled;
    return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned char ledstat;
    struct gpioled_dev *dev = filp->private_data;

    retvalue = copy_from_user(databuf, buf, cnt);
    if(retvalue < 0) {

    }
    ledstat = databuf[0];

    if(ledstat == LEDON) {
        gpio_set_value(dev->led_gpio, 0);
    } else if(ledstat == LEDOFF) {
        gpio_set_value(dev->led_gpio, 1);
    }

    return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
    struct gpioled_dev *dev = filp->private_data;

#ifdef ATOMIC_LOCK_T
    atomic_inc(&dev->lock);
#endif

#ifdef SPIN_LOCK_T
    spin_lock_irqsave(&dev->lock, flags);
    if(dev->dev_stats) {
        dev->dev_stats--;
    }
    spin_unlock_irqrestore(&dev->lock, flags);
#endif // SPIN_LOCK_T

#ifdef SEM_LOCK_T
    up(&dev->sem);
#endif //

#ifdef MUTEX_LOCK_T
    mutex_unlock(&dev->lock);
#endif // DEBUG

    return 0;
}

static struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

static int __init led_intt(void)
{
    int ret = 0;

#ifdef ATOMIC_LOCK_T
    //初始化原子变量
    atomic_set(&gpioled.lock, 1);
#endif

#ifdef SPIN_LOCK_T
    spin_lock_init(&gpioled.lock);
#endif // SPIN_LOCK_T

#ifdef SEM_LOCK_T
    sema_init(&gpioled.sem, 1);
#endif //

#ifdef MUTEX_LOCK_T
    mutex_init(&gpioled->lock);
#endif // DEBUG

    gpioled.nd = of_find_node_by_path("/gpioled");
    if(gpioled.nd == NULL) {

    } else {

    }

    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
    if(gpioled.led_gpio < 0) {

    } else {

    }

    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if(ret < 0) {

    }

    if(gpioled.major) {
        gpioled.devid = MKDEV(gpioled.major, 0);
        register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
    } else {
        alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.devid);
    }

    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);

    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);

    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);

    return 0;
}

static void __exit led_exit(void)
{
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);

    device_destory(gpioled.class, gpioled.devid);
    class_destory(gpioled.class);
}

module_init(led_intt);
module_exit(led_exit);
MODULE_LICENSE("GPL");
