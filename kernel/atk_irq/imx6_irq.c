/**
 * 在linux系统上终端无法嵌套，既当前中断A没处理完之前，不会响应另一个中断（即使它的优先级更高） || 不能嵌套
 * 越快越好
 * 在中断的处理过程中，该CPU是不能进行进程调度的，所以终端的处理越快越好，尽早让其它中断能被处理----进程调度靠定时器中断来实现
 * static inline int __must_checkrequest_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev)//使用中断
 * 要处理的事情太多，拆分为上半部、下半部；下半部的实现方式有很多种（主要为tasklet(小任务)、work queue(工作队列)）
 * 中断上半部，用来处理紧急的事，它是在关中断的状态下执行
 * 中断下半部，用来处理耗时的、不那么紧急的事情，它是在开中断的状态下执行的
 * 中断下半部执行时，有可能会被多次打断，有可能在发生同一个中断
 * 中断上半部执行完后，触发中断下半部的处理
 * 中断上半部、下半部的执行过程中，不能休眠，需要有人调度进程
 * 工作队列：kworker线程
 * 很耗时的中断处理，应该放到线程里面去
 * 在中断上半部调用static inline bool schedule_work(struct work_struct *work)函数，触发work的处理，在线程运行，对应的函数可以休眠
 * int request_threaded_irq(unsigned int irq, irq_handler_t handler, irq_handler_t thread_fn, unsigned long irqflags, const char *devname, void *dev_id)
 * irq是中断号，handler是中断触发时首先要执行的code，类似于顶半部，该函数最后会return IRQ_WAKE_THREAD来唤醒中断线程
 * **/
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
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define IMX6UIRQ_CNT		1			/* 设备号个数 	*/
#define IMX6UIRQ_NAME		"imx6uirq"	/* 名字 		*/
#define KEY0VALUE			0X01		/* KEY0按键值 	*/
#define INVAKEY				0XFF		/* 无效的按键值 */
#define KEY_NUM				1			/* 按键数量 	*/

struct irq_keydesc {
    int gpio;
    int irqnum;
    unsigned char value;
    char name[10];
    irqreturn_t (*handler)(int, void *);
};

struct imx6uirq_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    atomic_t keyvalue;
    atomic_t releasekey;
    struct timer_list timer;
    struct irq_keydesc irqkeydesc[KEY_NUM];
    unsigned char curkeynum;
};

struct imx6uirq_dev imx6uirq;

static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)dev_id;

    dev->curkeynum = 0;
    dev->timer.data = (volatile long)dev_id;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));

    return IRQ_RETVAL(IRQ_HANDLED);
}

void timer_function(unsigned long arg)
{
    unsigned char value;
    unsigned char num;
    struct irq_keydesc *keydesc;
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)arg;

    num = dev->curkeynum;
    keydesc = &dev->irqkeydesc[num];

    value = gpio_get_value(keydesc->gpio);
	if(value == 0){ 						/* 按下按键 */
		atomic_set(&dev->keyvalue, keydesc->value);
	} else {/* 按键松开 */
		atomic_set(&dev->keyvalue, 0x80 | keydesc->value);
		atomic_set(&dev->releasekey, 1);	/* 标记松开按键，即完成一次完整的按键过程 */			
	}	
}

static int keyio_init()
{
    unsigned char i = 0;
    int ret = 0;

    imx6uirq.nd = of_find_node_by_path("/key");
    if(imx6uirq.nd == NULL) {

    }

    for(i = 0; i < KEY_NUM; i++) {
        imx6uirq.irq_keydesc[i].gpio = of_get_named_gpio(imx6uirq.nd, "key-gpio", i);
        if(imx6uirq.irq_keydesc[i].gpio < 0) {

        }
    }

    for(i = 0; i < KEY_NUM; i++) {
        memset(imx6uirq.irq_keydesc[i].name, 0, sizeof(imx6uirq.irq_keydesc[i].name));
        sprintf(imx6uirq.irq_keydesc[i].name, "KEY%d", i);
        gpio_requst(imx6uirq.irq_keydesc[i].gpio, imx6uirq.irq_keydesc[i].name);
        gpio_direction_input(imx6uirq.irq_keydesc[i].gpio);
        imx6uirq.irq_keydesc[i].irqnum = irq_of_parse_and_map(imx6uirq.nd, i);
    }

    imx6uirq.irq_keydesc[0].handler = key0_handler;
    imx6uirq.irq_keydesc[0].value = KEY0VALUE;

    for(i = 0; i < KEY_NUM; i++) {
        ret |= request_irq(imx6uirq.irq_keydesc[i].irqnum, imx6uirq.irq_keydesc[0].handler, 
                        IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, imx6uirq.irq_keydesc[i].name, &imx6uirq);
    }

    init_timer(&imx6uirq.timer);
    imx6uirq.timer.function = timer_function;

    return 0;
}

static int imx6uirq_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &imx6uirq;
    return 0;
}

static ssize_t imx6uirq_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    int ret =0;
    unsigned char keyvalue = 0;
    unsigned char releasekey = 0;
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)filp->private_data;

    keyvalue = atomic_read(&dev->keyvalue);
    releasekey = atomic_read(&dev->releasekey);

    if (releasekey) { /* 有按键按下 */	
		if (keyvalue & 0x80) {
			keyvalue &= ~0x80;
			ret = copy_to_user(buf, &keyvalue, sizeof(keyvalue));
		} else {
			goto data_error;
		}
		atomic_set(&dev->releasekey, 0);/* 按下标志清零 */
	} else {
		goto data_error;
	}
	return 0;
	
data_error:
	return -EINVAL;
    return 0;
}

static struct file_operations imx6uirq_fops = {
    .owner = THIS_MODULE,
    .open = imx6uirq_open,
    .read = imx6uirq_read,
};

static int __init imx6uirq_init(void)
{
    if(imx6uirq.major) {
        imx6uirq.devid = MKDEV(imx6uirq.major, 0);
        register_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT, IMX6UIRQ_NAME);
    } else {
        allco_chedev_region(imx6uirq.devid, 0, IMX6UIRQ_CNT, IMX6UIRQ_NAME);
        imx6uirq.major = MAJOR(imx6uirq.devid);
        imx6uirq.minor = MINOR(imx6uirq.devid);
    }

    cdev_init(&imx6uirq.cdev, &imx6uirq_fops);
    cdev_add(&imx6uirq.cdev, imx6uirq.devid, IMX6UIRQ_CNT);

    imx6uirq.class = class_create(THIS_MODULE, IMX6UIRQ_NAME);
    if(IS_ERR(imx6uirq.class)) {

    }

    imx6uirq.device = device_create(imx6uirq.class, NULL, imx6uirq.devid, NULL, IMX6UIRQ_NAME);
    if(IS_ERR(imx6uirq.device)) {

    }

    atomic_set(&imx6uirq.keyvalue, INVAKEY);
    atomic_set(&imx6uirq.releasekey, 0);
    keyio_init();

    return 0;
}

static void __exit imx6uirq_exit(void)
{
    unsigned int i = 0;
    del_timer_sync(&imx6uirq.timer);

    for(i = 0; i < KEY_NUM; i++) {
        free_irq(imx6uirq.irq_keydesc[i].irqnum, &imx6uirq);
        gpio_free(imx6uirq.irq_keydesc[i].gpio);
    }

    cdev_del(&imx6uirq.cdev);
    unregister_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT);
    device_destory(imx6uirq.class, imx6uirq.devid);
    class_destory(imx6uirq.class);
}

module_init(imx6uirq_init);
module_exit(imx6uirq_exit);
MODULE_LICENSE("GPL");