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

struct led_resource
{
    struct device_node* device_node;
    void __iomem *virtual_CCM_CCGR;
    void __iomem *virtual_IOMUXC_SW_MUX_CTL_PAD;
    void __iomem *virtual_IOMUXC_SW_PAD_CTL_PAD;
    void __iomem *virtual_DR;
    void __iomem *virtual_GDIR;
    uint32_t led_pin;
    uint32_t clock_offset;
};

struct led_resource led_red;
struct led_resource led_green;
struct led_resource led_blue;
struct device_node* rgb_led_device_node;

static dev_t led_devno;
static struct cdev led_chr_dev;
struct class* class_led;
struct device* device;

#define RED_MASK 0x04
#define GREEN_MASK 0x02
#define BLUE_MASK 0x01

#define DEV_MAJOR 230
#define DEV_NAME "my_rgb_led"
#define DEV_CNT 1

static int led_chr_dev_open(struct inode* inode, struct file* filp)
{
    printk("\n open from driver\n");
    return 0;
}

void led_on(struct led_resource* led)
{
    uint register_data = 0;
    register_data = readl(led->virtual_DR);
    register_data &= ~(0x01 << led->led_pin);
    writel(register_data, led->virtual_DR);
}

void led_off(struct led_resource* led)
{
    uint register_data = 0;
    register_data = readl(led->virtual_DR);
    register_data |= (0x01 << led->led_pin);
    writel(register_data, led->virtual_DR);
}

static ssize_t led_chr_dev_write(struct file* filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    unsigned long write_data = 0;
    int error = kstrtoul_from_user(buf, cnt, 10, &write_data);
    if (error < 0)
    {
        printk("get user data fail");
        return -1;
    }
    
    printk(KERN_INFO "get user input %lu\n", write_data);

    if (write_data & RED_MASK)
        led_on(&led_red);
    else
        led_off(&led_red);

    if (write_data & GREEN_MASK)
        led_on(&led_green);
    else
        led_off(&led_green);

    if (write_data & BLUE_MASK)
        led_on(&led_blue);
    else
        led_off(&led_blue);

    return cnt;
}

static int led_cdev_release(struct inode* inode, struct file* filp)
{
    return 0;
}

static struct file_operations led_chr_dev_fops = {
    .owner   = THIS_MODULE,
    .open    = led_chr_dev_open,
    .write   = led_chr_dev_write,
    .release = led_cdev_release,
};

void print_led(const struct led_resource* led, const char* name)
{
    printk(KERN_INFO "**************%s****************\n", name);

    printk(KERN_INFO "\t CCM_CCGR [%p][%u]\n", led->virtual_CCM_CCGR, readl(led->virtual_CCM_CCGR));
    printk(KERN_INFO "\t virtual_IOMUXC_SW_MUX_CTL_PAD [%p][%u]\n", led->virtual_IOMUXC_SW_MUX_CTL_PAD, readl(led->virtual_IOMUXC_SW_MUX_CTL_PAD));
    printk(KERN_INFO "\t virtual_IOMUXC_SW_PAD_CTL_PAD [%p][%u]\n", led->virtual_IOMUXC_SW_PAD_CTL_PAD, readl(led->virtual_IOMUXC_SW_PAD_CTL_PAD));
    printk(KERN_INFO "\t virtual_DR [%p][%u]\n", led->virtual_DR, readl(led->virtual_DR));
    printk(KERN_INFO "\t virtual_GDIR [%p][%u]\n", led->virtual_GDIR, readl(led->virtual_GDIR));
    printk(KERN_INFO "\t led_pin [%u]\n", led->led_pin);
    printk(KERN_INFO "\t clock_offset [%u]\n", led->clock_offset);

    printk(KERN_INFO "***********************************\n");
}

int get_led_resource(struct led_resource* led, const char* name)
{
    int ret = 0;
    led->device_node = of_find_node_by_name(rgb_led_device_node, name);
    if (led->device_node == NULL)
    {
        printk(KERN_ERR "\t get rgb_led_red failed\n");
        return -1;
    }

    led->virtual_CCM_CCGR              = of_iomap(led->device_node, 0);
    led->virtual_IOMUXC_SW_MUX_CTL_PAD = of_iomap(led->device_node, 1);
    led->virtual_IOMUXC_SW_PAD_CTL_PAD = of_iomap(led->device_node, 2);
    led->virtual_DR                    = of_iomap(led->device_node, 3);
    led->virtual_GDIR                  = of_iomap(led->device_node, 4);

    ret = of_property_read_u32(led->device_node, "led_pin", &(led->led_pin));
    if (ret != 0)
    {
        printk(KERN_ERR "\t get led_pin fail\n");
        return ret;
    }

    ret = of_property_read_u32(led->device_node, "clock_offset", &(led->clock_offset));
    if (ret != 0)
    {
        printk(KERN_ERR "\t get clock_offset fail\n");
        return ret;
    }

    print_led(led, name);

    return ret;
}

void init_led(struct led_resource* led)
{
    uint32_t register_data = 0;

    // open gpio clock
    register_data = readl(led->virtual_CCM_CCGR);
    register_data |= (0x03 << led->clock_offset);
    writel(register_data, led_red.virtual_CCM_CCGR);

    // set gpio
    register_data = readl(led->virtual_IOMUXC_SW_MUX_CTL_PAD);
    register_data &= ~(0xff);
    register_data |= (0x05);
    writel(register_data, led->virtual_IOMUXC_SW_MUX_CTL_PAD);

    // set gpio property
    register_data = readl(led->virtual_IOMUXC_SW_PAD_CTL_PAD);
    register_data = 0x10B0;
    writel(register_data, led->virtual_IOMUXC_SW_PAD_CTL_PAD);

    // set gpio1_04 output
    register_data = readl(led->virtual_GDIR);
    register_data |= (0x01 << led->led_pin);
    writel(register_data, led->virtual_GDIR);

    // set gpio1_04 high level
    register_data = readl(led->virtual_DR);
    register_data |= (0x01 << led->led_pin);
    writel(register_data, led->virtual_DR);
}

static int led_probe(struct platform_device* pdev)
{
    int ret = 0;

    printk(KERN_EMERG "\t match sucessed \n");

    rgb_led_device_node = of_find_node_by_path("/rgb_led");
    if (rgb_led_device_node == NULL)
    {
        printk(KERN_ERR "\t get rgb_led failed! \n");
        return -1;
    }

    ret = get_led_resource(&led_red, "rgb_led_red");
    ret = get_led_resource(&led_green, "rgb_led_green");
    ret = get_led_resource(&led_blue, "rgb_led_blue");

    init_led(&led_red);
    init_led(&led_green);
    init_led(&led_blue);

    ret = alloc_chrdev_region(&led_devno, 0, DEV_CNT, DEV_NAME);
    if (ret < 0)
    {
        printk("fail to alloc led_devno\n");
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
alloc_err:
    return -1;
}

static int led_remove(struct platform_device* pdev)
{
    iounmap(led_green.virtual_CCM_CCGR);
    iounmap(led_green.virtual_IOMUXC_SW_MUX_CTL_PAD);
    iounmap(led_green.virtual_IOMUXC_SW_PAD_CTL_PAD);
    iounmap(led_green.virtual_DR);
    iounmap(led_green.virtual_GDIR);    
    iounmap(led_red.virtual_CCM_CCGR);
    iounmap(led_red.virtual_IOMUXC_SW_MUX_CTL_PAD);
    iounmap(led_red.virtual_IOMUXC_SW_PAD_CTL_PAD);
    iounmap(led_red.virtual_DR);
    iounmap(led_red.virtual_GDIR);    
    iounmap(led_blue.virtual_CCM_CCGR);
    iounmap(led_blue.virtual_IOMUXC_SW_MUX_CTL_PAD);
    iounmap(led_blue.virtual_IOMUXC_SW_PAD_CTL_PAD);
    iounmap(led_blue.virtual_DR);
    iounmap(led_blue.virtual_GDIR);

    device_destroy(class_led, led_devno);
    class_destroy(class_led);
    cdev_del(&led_chr_dev);
    unregister_chrdev_region(led_devno, DEV_CNT);

    return 0;
}

static const struct of_device_id rgb_led[] = {
    {.compatible = "fire,rgb_led"},
    {}
};

struct platform_driver led_platform_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name           = "rgb-leds-platform",
        .owner          = THIS_MODULE,
        .of_match_table = rgb_led,
    }
};

static int __init led_platform_driver_init(void)
{
    int driverState = platform_driver_register(&led_platform_driver);
    printk(KERN_EMERG "\tDriverState is %d\n", driverState);
    return 0;
}
module_init(led_platform_driver_init);

static void __exit led_platform_driver_exit(void)
{
	platform_driver_unregister(&led_platform_driver);
	printk(KERN_EMERG "rgb dev exit!\n");
}

module_exit(led_platform_driver_exit);
MODULE_AUTHOR("jing");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("led platform driver");