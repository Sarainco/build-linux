/**新字符设备驱动
 * 1.字符设备结构；使用cdev结构体表示一个字符设备，主要是ops和dev
 * 2.cdev_init函数
 * 3.cdev_add函数；向linux添加字符设备
 * 4.cdev_del函数；删除字符设备
 * 5.自动创建节点
 * 6.mdev机制：udev是一个用户程序，在linux下通过udev来实现设备文件的创建和删除，udev可以检测系统中的硬件设备状态，可以根据其状态来创建或删除设备文件
 * 7.Linux系统中的热插拔事件也由mdev管理 echo /sbin/mdev > /proc/sys/kernel/hotplug
 * 8.创建和删除类
 * 9.创建设备
 * 10.设置文件私有数据**/


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

#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#ifdef DEVICE_USE_OF
/**设备树
 * 1.系统总线
 * 2.I2C控制器、SPI控制器等等
 * 3.设备
 * 4.描述设备树的文件叫DTS
 * 5.DTS、DTB、DTC**/

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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#endif // DEVICE_USE_OF


#define NEWCHRLED_CNT			1		  	/* 设备号个数 */
#define NEWCHRLED_NAME			"newchrled"	/* 名字 */
#define LEDOFF 					0			/* 关灯 */
#define LEDON 					1			/* 开灯 */

struct newchrled_dev {
    dev_t devid;    //设备号
    struct cdev cdev;   //cdev
    struct class *class;    //类
    struct device *device;  //设备
    int major;  //主设备号
    int minor;  //次设备号
};

struct newchrled_dev newchrled;


static int led_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    return 0;
}

static struct file_operations newchrled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .read = led_read,
    .write = led_write,
};

static int __init led_init(void)
{

#ifdef DEVICE_USE_OF
    u32 val = 0;
    int ret;
    u32 regdata[14];
    const char *str;
    struct property *proper;

    /**获取设备树中的设备属性**/
    //1.获取设备节点
    dtsled.nd = of_find_node_by_path("/atkrk3568-led");
    if(dtsled.nd == NULL) {

    } else {

    }
    //2.获取compatible属性内容
    proper = of_find_property(dtsled.nd, "compatible", NULL);
    if(proper == NULL) {

    } else {

    }
    //3.获取sataus属性内容
    ret = of_proper_read_string(dtsled.nd, "status", &str);
    if (ret < 0) {

    } else {

    }
    //4.获取reg属性内容
    ret = of_proper_read_u32_array(dtsled.nd, "reg", regdata, 10);
    if(ret < 0) {

    } else {

    }

#endif

    /*注册字符设备驱动*/
    //1.创建设备号
    if(newchrled.major) {
        newchrled.devid = MKDEV(newchrled.major, 0);
        register_chrdev_region(newchrled.devid, NEWCHRLED_CNT, NEWCHRLED_NAME);
    } else {
        alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLED_CNT, NEWCHRLED_NAME);//申请设备号
        newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid);
    }
    printk("newcheled major=%d,minor=%d\r\n",newchrled.major, newchrled.minor);	

    //2.初始化cdev
    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &newchrled_fops);

    //3.添加一个cdev
    cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_CNT);

    //4.创建类
    newchrled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
    if(IS_ERR(newchrled.class)) {
        return PTR_ERR(newchrled.class);
    }

    //5.创建设备
    newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, NEWCHRLED_NAME);
	if (IS_ERR(newchrled.device)) {
		return PTR_ERR(newchrled.device);
	}

    return 0;
}

static void __exit led_exit(void)
{
    //注销字符设备驱动
    cdev_del(&newchrled.cdev); //删除cdev
    unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT); //注销设备号

    device_destory(newchrled.class, newchrled.devid);
    class_destroy(newchrled.class);

}

mddule_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yuji");

#ifdef GPIO_LED
/**gpio和pinctl子系统**/
/**pinctl子系统的主要工作内容如下
 * 1.获取设备树中pin信息
 * 2.设置pin的复用功能
 * 3.设置pin的电气特性，如驱动能力
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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

/* gpioled设备结构体 */
struct gpioled_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */
	int led_gpio;			/* led所使用的GPIO编号		*/
};

struct gpioled_dev gpioled;	/* led设备 */

static struct file_operations gpioled_fops = {

};

static int __init led_init(void)
{
    int ret = 0;

    gpioled.nd = of_find_node_by_path("/gpioled");
    if(gpioled.nd == NULL) {

    }

    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
    if(gpioled.led_gpio < 0) {

    }

    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if(ret < 0) {

    }

    return 0;
}

static void __exit led_exit(void)
{

}

module_init(led_init);
module_exit(led_exit);

#endif // GPIO_LED