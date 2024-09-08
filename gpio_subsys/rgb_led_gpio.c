#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#define DEV_MAJOR 230
#define DEV_NAME "my_rgb_led"
#define DEV_CNT 1

dev_t led_devno;
struct cdev led_chr_dev;
struct class* class_led;
struct device* device;


int rgb_led_red, rgb_led_blue, rgb_led_green;
struct device_node* rgb_led_device_node;

static int led_chr_dev_open(struct inode* inode, struct file* filp)
{
    printk("\t open from driver\n");
    return 0;
}

static ssize_t led_chr_dev_write(struct file* filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    unsigned long write_data = 0;
    int ret = kstrtoul_from_user(buf, cnt, 10, &write_data);
    if (ret < 0)
    {
        printk("\t get user data failed\n");
        return -1;
    }

    gpio_direction_output(rgb_led_red, !(write_data & 0x04));
    gpio_direction_output(rgb_led_green, !(write_data & 0x02));
    gpio_direction_output(rgb_led_green, !(write_data & 0x01));

    return cnt;
}

static struct file_operations led_chr_dev_fops = 
{
    .owner = THIS_MODULE,
    .open  = led_chr_dev_open,
    .write = led_chr_dev_write,
};

static int led_probe(struct platform_device* pdev)
{
    unsigned int register_data = 0;
    int ret = 0;

    printk(KERN_EMERG "\t match successed \n");

    rgb_led_device_node = of_find_node_by_path("/rgb_led_gpio");
    if (rgb_led_device_node == NULL)
    {
        printk(KERN_EMERG "\t get rgb_led failed! \n");
    }

    rgb_led_red = of_get_named_gpio(rgb_led_device_node, "rgb_led_red", 0);
    rgb_led_green = of_get_named_gpio(rgb_led_device_node, "rgb_led_green", 0);
    rgb_led_blue = of_get_named_gpio(rgb_led_device_node, "rgb_led_blue", 0);

    printk("rgb_led_red = %d,\n rgb_led_green = %d,\n rgb_led_blue = %d,\n", rgb_led_red, rgb_led_green, rgb_led_blue);

    gpio_direction_output(rgb_led_red, 1);
    gpio_direction_output(rgb_led_green, 1);
    gpio_direction_output(rgb_led_blue, 1);

    ret = alloc_chrdev_region(&led_devno, 0, DEV_CNT, DEV_NAME);
    if (ret < 0)
    {
        printk(KERN_EMERG "fail to alloc led_devno\n");
        goto alloc_err;
    }

    led_chr_dev.owner = THIS_MODULE;
    cdev_init(&led_chr_dev, &led_chr_dev_fops);

    ret = cdev_add(&led_chr_dev, led_devno, DEV_CNT);
    if (ret < 0)
    {
        printk("fail to add cdev\n");
        goto add_err;
    }

    class_led = class_create(THIS_MODULE, DEV_NAME);

    device = device_create(class_led, NULL, led_devno, NULL, DEV_NAME);

    return 0;

add_err:
    unregister_chrdev_region(led_devno, DEV_CNT);
    printk("\n error! \n");

alloc_err:
    return -1;
}

static const struct of_device_id rgb_led[] = {
    {.compatible = "fire,rgb-led-gpio"},
    {}
};

struct platform_driver led_platform_driver = {
    .probe = led_probe,
    .driver = {
        .name = "rgb-leds-platform",
        .owner = THIS_MODULE,
        .of_match_table = rgb_led,
    }
};

static int __init led_platform_driver_init(void)
{
    int error;

    error = platform_driver_register(&led_platform_driver);

    printk(KERN_EMERG "\tDriverState = %d\n", error);
    return 0;
}

static void __exit led_platform_driver_exit(void)
{
    printk(KERN_EMERG "platform_driver_exit\n");

    platform_driver_unregister(&led_platform_driver);
}

module_init(led_platform_driver_init);
module_exit(led_platform_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JING");