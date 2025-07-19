#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>

struct gpio_test_data {
    struct gpio_desc *gpio1; // GPIO2_A2
    struct gpio_desc *gpio2; // GPIO1_D6
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
    __ATTR(gpio1_value, 0664, gpio1_value_show, gpio1_value_store);

static struct kobj_attribute gpio2_value_attr =
    __ATTR(gpio2_value, 0664, gpio2_value_show, gpio2_value_store);

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
    g_data->kobj = kobject_create_and_add("gpio_test", kernel_kobj);
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

