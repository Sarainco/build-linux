/***
*字符设备就是一个一个字节，按照字节流进行读写操作的设备，读写数据是分先后顺序的
* 1.驱动模块的加载和卸载
* 2.字符设备的注册和注销
* 3.实现设备具体的操作函数
* 4.添加LICENSE和作者信息
*/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>


#define CHRDEVBASE_MAJOR    200     //设备号
#define CHRDEVBASE_NAME     "chrdevbase"    //设备名


static char readbuf[100];
static char writebuf[100];

static char kerneldata[] = {"kernel data"};

static int chrdevbase_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int chrdevbase_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t chrdevbase_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue = 0;

    memcpy(readbuf, kerneldata, sizeof(kerneldata));

    retvalue = copy_to_user(buf, readbuf, cnt);
    if(retvalue < 0) {

    }

    return retvalue;
}

static ssize_t chrdevbase_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue = 0;

    retvalue = copy_from_user(writebuf, buf, cnt);
    if(retvalue < 0) {

    }

    return retvalue;
}

static struct file_operations chrdevbase_fops = {
	.owner = THIS_MODULE,
	.open = chrdevbase_open,
	.release = chrdevbase_release,
	.read = chrdevbase_read,
	.write = chrdevbase_write,
};

static int __init chrdevbase_init(void)
{
    int retvalue = 0;

    retvalue = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
    if(retvalue < 0) {

    }

    return 0;
}

static void __exit chrdevbase_exit(void)
{
    unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
}

module_init(chrdevbase_init);
module_exit(chrdevbase_exit);


MODULE_LICENSE("GPL")
MODULE_AUTHOR("yuji");
