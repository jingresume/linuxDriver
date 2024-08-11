#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#define CCM_CCGR1                               0x020C406C  //时钟控制寄存器
#define IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04        0x020E006C  //GPIO1_04复用功能选择寄存器
#define IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04        0x020E02F8  //PAD属性设置寄存器
#define GPIO1_GDIR                              0x0209C004  //GPIO方向设置寄存器（输入或输出）
#define GPIO1_DR                                0x0209C000  //GPIO输出状态寄存器

#define CCM_CCGR3                               0x020C4074
#define GPIO4_GDIR                              0x020A8004
#define GPIO4_DR                                0x020A8000

#define IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO020       0x020E01E0
#define IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO020       0x020E046C

#define IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO019       0x020E01DC
#define IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO019       0x020E0468

/*
static struct led_chrdev led_cdev[DEV_CNT] = {
    {.pa_dr = 0x0209C000,.pa_gdir = 0x0209C004,.pa_iomuxc_mux =
     0x20E006C,.pa_ccm_ccgrx = 0x20C406C,.pa_iomux_pad =
     0x20E02F8,.led_pin = 4,.clock_offset = 26},
    {.pa_dr = 0x20A8000,.pa_gdir = 0x20A8004,.pa_iomuxc_mux =
     0x20E01E0,.pa_ccm_ccgrx = 0x20C4074,.pa_iomux_pad =
     0x20E046C,.led_pin = 20,.clock_offset = 12},
    {.pa_dr = 0x20A8000,.pa_gdir = 0x20A8004,.pa_iomuxc_mux =
     0x20E01DC,.pa_ccm_ccgrx = 0x20C4074,.pa_iomux_pad =
     0x20E0468,.led_pin = 19,.clock_offset = 12},
};
*/

static struct resource rled_resource[] = {
    [0] = DEFINE_RES_MEM(GPIO1_DR, 4),
    [1] = DEFINE_RES_MEM(GPIO1_GDIR, 4),
    [2] = DEFINE_RES_MEM(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04, 4),
    [3] = DEFINE_RES_MEM(CCM_CCGR1, 4),
    [4] = DEFINE_RES_MEM(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04, 4),
};

unsigned int rled_hwinfo[2] = {4, 26};

static struct resource gled_resource[] = {
    [0] = DEFINE_RES_MEM(GPIO4_DR, 4),
    [1] = DEFINE_RES_MEM(GPIO4_GDIR, 4),
    [2] = DEFINE_RES_MEM(IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO020, 4),
    [3] = DEFINE_RES_MEM(CCM_CCGR3, 4),
    [4] = DEFINE_RES_MEM(IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO020, 4),
};

unsigned int gled_hwinfo[2] = {20, 12};

static struct resource bled_resource[] = {
    [0] = DEFINE_RES_MEM(GPIO4_DR, 4),
    [1] = DEFINE_RES_MEM(GPIO4_GDIR, 4),
    [2] = DEFINE_RES_MEM(IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO019, 4),
    [3] = DEFINE_RES_MEM(CCM_CCGR3, 4),
    [4] = DEFINE_RES_MEM(IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO019, 4),
};

unsigned int bled_hwinfo[2] = {19, 12};

static int led_cdev_release(struct inode* inode, struct file* filp)
{
    return 0;
}

//void	(*release)(struct device *dev);
void led_release(struct device* dev)
{
    return 0;
}

static struct platform_device rled_pdev = {
    .name          = "led_pdev",
    .id            = 0,
    .num_resources = ARRAY_SIZE(rled_resource),
    .resource      = rled_resource,
    .dev           = {
        .release = led_release,
        .platform_data = rled_hwinfo,
    },
};

static struct platform_device gled_pdev = {
    .name          = "led_pdev",
    .id            = 1,
    .num_resources = ARRAY_SIZE(rled_resource),
    .resource      = gled_resource,
    .dev           = {
        .release = led_release,
        .platform_data = gled_hwinfo,
    },
};

static struct platform_device bled_pdev = {
    .name          = "led_pdev",
    .id            = 2,
    .num_resources = ARRAY_SIZE(rled_resource),
    .resource      = bled_resource,
    .dev           = {
        .release = led_release,
        .platform_data = bled_hwinfo,
    },
};

static __init int led_pdev_init(void)
{
    printk("pdev init\n");
    platform_device_register(&rled_pdev);
    platform_device_register(&gled_pdev);
    platform_device_register(&bled_pdev);
    return 0;
}

module_init(led_pdev_init);

static __exit void led_pdev_exit(void)
{
    printk("pdev exit\n");
    platform_device_unregister(&rled_pdev);
    platform_device_unregister(&gled_pdev);
    platform_device_unregister(&bled_pdev);
}

module_exit(led_pdev_exit);

MODULE_AUTHOR("jing");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("the example for platform driver");