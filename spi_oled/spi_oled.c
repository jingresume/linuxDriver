#include <linux/cdev.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include "spi_oled.h"

// cdev define
#define DEV_CNT 1
#define DEV_NAME "myoled"
static dev_t oled_devno;
struct cdev oled_cdev;
struct class* class_oled;
struct device* device_oled;

// spi define
struct device_node *oled_node;
struct spi_device *oled_spi;
static int    oled_dc_gpio;
static int    oled_reset_gpio;

u8 oled_init_data[] = {
	0xae, 0xae, 0x00, 0x10, 0x40,
	0x81, 0xcf, 0xa1, 0xc8, 0xa6,
	0xa8, 0x3f, 0xd3, 0x00, 0xd5,
	0x80, 0xd9, 0xf1, 0xda, 0x12,
	0xdb, 0x40, 0x20, 0x02, 0x8d,
	0x14, 0xa4, 0xa6, 0xaf};

/* oled ops*/

static int oled_send_cmds(struct spi_device *spi, u8* cmd, u16 len)
{
    int error = 0;
    
    struct spi_message *msg;
    struct spi_transfer *trans;

    gpio_direction_output(oled_dc_gpio, 0);

    msg = kzalloc(sizeof(struct spi_message), GFP_KERNEL);
    trans = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);

    spi_message_init(msg);
    trans->tx_buf = cmd;
    trans->len    = len;
    spi_message_add_tail(trans, msg);

    error = spi_sync(spi, msg);

    kfree(msg);
    kfree(trans);

    if (error != 0)
    {
        printk(KERN_EMERG "oled sen cmd[%u] error", *cmd);
    }

    return error;
}

static int oled_send_cmd(struct spi_device *spi, u8 cmd)
{
    return oled_send_cmds(spi, &cmd, 1);
}

static int oled_send_data(struct spi_device *spi, u8* data, u16 len)
{
     int error  = 0;
     int offset = 0;
	 u16 maxLen = 30;

     struct spi_message *msg;
     struct spi_transfer *trans;

     gpio_direction_output(oled_dc_gpio, 1);

     msg = kzalloc(sizeof(struct spi_message), GFP_KERNEL);
     trans = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);

     while (len > 0)
     {
        u16 cur_len = min(maxLen, len);
        spi_message_init(msg);
        trans->tx_buf = data + offset;
        trans->len    = cur_len;
        spi_message_add_tail(trans, msg);
        error = spi_sync(spi, msg);

        if (error != 0)
        {
            printk(KERN_EMERG "oled send data failed\n");
            goto trans_err;
        }
        offset += cur_len;
        len -= cur_len;
     }

trans_err:
     kfree(msg);
     kfree(trans);
     return error;
}


static int set_coordinate(struct spi_device *spi, int x, int y)
{
    int error = 0;
	error += oled_send_cmd(oled_spi, 0xb0 + y);
	error += oled_send_cmd(oled_spi, ((x & 0xf0) >> 4) | 0x10);
	error += oled_send_cmd(oled_spi, (x & 0x0f) | 0x01);
    return error;
}

static int oled_fill(u8 data)
{
    int error = 0;
    u8 x, y;
    for (y = 0; y < 8; y++)
    {
        error += set_coordinate(oled_spi, 0, y);
        for (x = 0; x < OLED_WIDTH; x++)
        {
            error += oled_send_data(oled_spi, &data, 1);
        }
    }
    return error;
}

static int oled_display(u8 *buf, int x, int y, int len)
{
    int offset = 0;
    int error = 0;
	int cur_len = 0;

    while (len > 0)
    {
        error += set_coordinate(oled_spi, x, y);
        cur_len = OLED_WIDTH - x;
        if (len > cur_len)
        {
            error += oled_send_data(oled_spi, buf + offset, cur_len);
            x = 0;
            y++;
            offset += cur_len;
            len -= cur_len;
        }
        else
        {
            error += oled_send_data(oled_spi, buf + offset, len);
            offset += len;
            len = 0;
        }
    }

    if (error != 0)
    {
        printk(KERN_EMERG "oled_display error %d", error);
    }

    return error;
}

void oled_init(void)
{
    oled_send_cmds(oled_spi, oled_init_data, sizeof(oled_init_data));

    oled_fill(0x00);
}

/* cdev fops */

static int oled_open(struct inode *inode, struct file *filp)
{
    oled_init();
    return 0;
}

static int oled_release(struct inode *inode, struct file *filp)
{
    oled_send_cmd(oled_spi, 0xae);
    return 0;
}

static int oled_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *off)
{
    struct oled_display_data* write_data;
	unsigned long ret = 0;
    write_data = (struct oled_display_data*)kzalloc(cnt, GFP_KERNEL);
    ret = copy_from_user(write_data, buf, cnt);
    // printk( "oled write data x %d y %d len %d\n", write_data->x, write_data->y, write_data->len);
    oled_display(write_data->display_buffer, write_data->x, write_data->y, write_data->len);
    kfree(write_data);
    return cnt;
}


static const struct file_operations oled_cdev_fops = {
    .open = oled_open,
    .release = oled_release,
    .write = oled_write,
    .owner = THIS_MODULE,
};

/* spi driver*/
static int oled_probe(struct spi_device* spi)
{
	int ret = 0;
    printk("espi oled match success\n");
    oled_node = of_find_node_by_path("/soc/aips-bus@2000000/spba-bus@2000000/ecspi@2008000/ecspi_oled@0");
    if (oled_node == NULL)
    {
        printk(KERN_EMERG "Can't find oled node\n");
        return -1;
    }

    oled_dc_gpio = of_get_named_gpio(oled_node, "d_c_control_pin", 0);
    gpio_direction_output(oled_dc_gpio, 1);

    oled_reset_gpio = of_get_named_gpio(oled_node, "spi_reset_pin", 0);
    gpio_direction_output(oled_reset_gpio, 1);

    printk(KERN_EMERG "oled_dc_gpio %d oled_reset_gpio %d", oled_dc_gpio, oled_reset_gpio);

    // init spi
    oled_spi = spi;
    oled_spi->mode = SPI_MODE_0;
    oled_spi->max_speed_hz = 2000000;
    spi_setup(oled_spi);

	printk("max_speed_hz = %d\n", oled_spi->max_speed_hz);
	printk("chip_select = %d\n", (int)oled_spi->chip_select);
	printk("bits_per_word = %d\n", (int)oled_spi->bits_per_word);
	printk("mode = %02X\n", oled_spi->mode);
	printk("cs_gpio = %02X\n", oled_spi->cs_gpio);

    // cdev add
    ret = alloc_chrdev_region(&oled_devno, 0, DEV_CNT, DEV_NAME);
    if (ret < 0)
    {
        printk(KERN_EMERG "Alloc chrdev region failed\n");
        goto alloc_err;
    }

    oled_cdev.owner = THIS_MODULE;
    cdev_init(&oled_cdev, &oled_cdev_fops);
    ret = cdev_add(&oled_cdev, oled_devno, DEV_CNT);
    if (ret < 0)
    {
        printk(KERN_EMERG "Add cdev failed\n");
        goto add_err;
    }

    // device add
    class_oled = class_create(THIS_MODULE, DEV_NAME);
    device_oled = device_create(class_oled, NULL, oled_devno, NULL, DEV_NAME);

    return 0;
add_err:
    unregister_chrdev_region(oled_devno, DEV_CNT);

alloc_err:
    return -1;
}

static int oled_remove(struct spi_device* spi)
{
    printk("oled driver remove\n");
    device_destroy(class_oled, oled_devno);
    class_destroy(class_oled);
    cdev_del(&oled_cdev);
    unregister_chrdev_region(oled_devno, DEV_CNT);
    return 0;
}


const struct spi_device_id oled_device_id[] = {
    {"fsl,ecspi_oled", 0},
    {},
};

const struct of_device_id oled_of_match_table[] = {
    {.compatible = "fsl,ecspi_oled"},
    {},
};

struct spi_driver oled_driver = {
    .probe = oled_probe,
    .remove = oled_remove,
    .id_table = oled_device_id,
    .driver = {
        .name = "fsl,ecspi_oled",
        .owner = THIS_MODULE,
        .of_match_table = oled_of_match_table,
    },
};

static int __init oled_driver_init(void)
{
    int error = 0;
    printk("oled driver init\n");
    error = spi_register_driver(&oled_driver);
    return error;
}

module_init(oled_driver_init);

static void __exit oled_driver_exit(void)
{
    printk("oled driver exit\n");
    spi_unregister_driver(&oled_driver);
}

module_exit(oled_driver_exit);

MODULE_AUTHOR("JING");
MODULE_LICENSE("GPL");