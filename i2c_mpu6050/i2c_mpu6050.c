#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/i2c.h>

#define SMPLRT_DIV    0x19
#define CONFIG        0x1A
#define GYRO_CONFIG   0x1B
#define ACCEL_CONFIG  0x1C
#define ACCEL_XOUT_H  0x3B
#define ACCEL_XOUT_L  0x3C
#define ACCEL_YOUT_H  0x3D
#define ACCEL_YOUT_L  0x3E
#define ACCEL_ZOUT_H  0x3F
#define ACCEL_ZOUT_L  0x40
#define TEMP_OUT_H    0x41
#define TEMP_OUT_L    0x42
#define GYRO_XOUT_H   0x43
#define GYRO_XOUT_L   0x44
#define GYRO_YOUT_H   0x45
#define GYRO_YOUT_L   0x46
#define GYRO_ZOUT_H   0x47
#define GYRO_ZOUT_L   0x48
#define PWR_MGMT_1    0x6B
#define WHO_AM_I      0x75

// cdev define
#define DEV_CNT 1
#define DEV_NAME "my_mpu6050"

static dev_t mpu_devno;
static struct cdev mpu_chr_dev;
struct class* mpu_class;
struct device* mpu_device;

// i2c define
static struct i2c_client *mpu_client;

static int i2c_write_mpu6050(struct i2c_client *client, u8 addr, u8 data)
{
    int error = 0;
    u8 buf[2] = {addr, data};
    struct i2c_msg msg;

    msg.addr  = client->addr;
    msg.buf   = buf;
    msg.flags = 0;
    msg.len   = 2;

    error = i2c_transfer(client->adapter, &msg, 1);
    if (error != 1)
    {
        printk(KERN_DEBUG "i2c transfer error %d\n", error);
        return -1;
    }
    return 0;
}

static int i2c_read_mpu6050(struct i2c_client *client, u8 addr, void *data, ssize_t size)
{
    int error = 0;
    struct i2c_msg msg[2];

    msg[0].addr  = client->addr;
    msg[0].buf  = &addr;
    msg[0].flags = 0;
    msg[0].len   = 1;

    msg[1].addr  = client->addr;
    msg[1].buf  = data;
    msg[1].flags = I2C_M_RD;
    msg[1].len   = size;

    error = i2c_transfer(client->adapter, msg, 2);
    if (error != 2)
    {
        printk(KERN_DEBUG "i2c transfer error\n");
        return -1;
    }
    return 0;
}

int mpu6050_init(void)
{
    int ret = 0;
    // 配置电源管理，0x00正常启动
    ret += i2c_write_mpu6050(mpu_client, PWR_MGMT_1, 0x00);
    // 配置采样频率
    ret += i2c_write_mpu6050(mpu_client, SMPLRT_DIV, 0x07);
    // 设置低通滤波、帧同步采样
    ret += i2c_write_mpu6050(mpu_client, CONFIG, 0x06);
    // 设置量程和加速度自检
    ret += i2c_write_mpu6050(mpu_client, ACCEL_CONFIG, 0x01);

    if (ret < 0)
    {
        printk("init mpu6050 fail\n");
        return -1;
    }
    return 0;
}

int mpu6050_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    printk("open mpu6050\n");
    ret = mpu6050_init();
    return ret;
}

ssize_t mpu6050_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    char data_H, data_L;
    int error = 0;
    short mpu6050_result[6];
    int i = 0;

    i2c_read_mpu6050(mpu_client, ACCEL_XOUT_H, &data_H, 1);
    i2c_read_mpu6050(mpu_client, ACCEL_XOUT_L, &data_L, 1);
    mpu6050_result[i++] = (data_H << 8) | data_L;

    i2c_read_mpu6050(mpu_client, ACCEL_YOUT_H, &data_H, 1);
    i2c_read_mpu6050(mpu_client, ACCEL_YOUT_L, &data_L, 1);
    mpu6050_result[i++] = (data_H << 8) | data_L;

    i2c_read_mpu6050(mpu_client, ACCEL_ZOUT_H, &data_H, 1);
    i2c_read_mpu6050(mpu_client, ACCEL_ZOUT_L, &data_L, 1);
    mpu6050_result[i++] = (data_H << 8) | data_L;

    i2c_read_mpu6050(mpu_client, GYRO_XOUT_H, &data_H, 1);
    i2c_read_mpu6050(mpu_client, GYRO_XOUT_L, &data_L, 1);
    mpu6050_result[i++] = (data_H << 8) | data_L;

    i2c_read_mpu6050(mpu_client, GYRO_YOUT_H, &data_H, 1);
    i2c_read_mpu6050(mpu_client, GYRO_YOUT_L, &data_L, 1);
    mpu6050_result[i++] = (data_H << 8) | data_L;

    i2c_read_mpu6050(mpu_client, GYRO_ZOUT_H, &data_H, 1);
    i2c_read_mpu6050(mpu_client, GYRO_ZOUT_L, &data_L, 1);
    mpu6050_result[i++] = (data_H << 8) | data_L;

    error = copy_to_user(buf, mpu6050_result, cnt);
    if (error != 0)
    {
        printk("copy to user error");
        return -1;
    }

    return cnt;
}

static const struct file_operations mpu6050_chr_dev_fops = {
    .owner = THIS_MODULE,
    .open = mpu6050_open,
    .read = mpu6050_read,
};

static const struct i2c_device_id mpu6050_id_table[] = {
    {"fire,i2c_mpu6050", 0},
    {}
};

static const struct of_device_id mpu6050_of_match_table[] = {
    {.compatible = "fire,i2c_mpu6050"},
    {}
};

static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    printk(KERN_EMERG "\t match sucessed \n");

    ret = alloc_chrdev_region(&mpu_devno, 0, DEV_CNT, DEV_NAME);
    if (ret < 0)
    {
        printk("fail to alloc mpu devno\n");
        goto alloc_err;
    }

    mpu_chr_dev.owner = THIS_MODULE;
    cdev_init(&mpu_chr_dev, &mpu6050_chr_dev_fops);

    ret = cdev_add(&mpu_chr_dev, mpu_devno, DEV_CNT);
    if (ret < 0)
    {
        printk("fail to add cdev\n");
        goto add_err;
    }

    mpu_class = class_create(THIS_MODULE, DEV_NAME);
    mpu_device = device_create(mpu_class, NULL, mpu_devno, NULL, DEV_NAME);

    mpu_client = client;

    return 0;

add_err:
    unregister_chrdev_region(mpu_devno, DEV_CNT);
alloc_err:
    return -1;
}

static int mpu6050_remove(struct i2c_client *client)
{
    printk(KERN_EMERG "\t mpu6050 remove");
    device_destroy(mpu_class, mpu_devno);
    class_destroy(mpu_class);
    cdev_del(&mpu_chr_dev);
    unregister_chrdev_region(mpu_devno, DEV_CNT);
    return 0;
}

struct i2c_driver mpu6050_driver = {
    .probe = mpu6050_probe,
    .remove = mpu6050_remove,
    .id_table = mpu6050_id_table,
    .driver = {
        .name = "fire,i2c_mpu6050",
        .owner = THIS_MODULE,
        .of_match_table = mpu6050_of_match_table,
    },
};

static int __init mpu6050_driver_init(void)
{
    int ret = 0;
    printk("mpu6050 driver init\n");
    ret = i2c_add_driver(&mpu6050_driver);
    return ret;
}

static void __exit mpu6050_driver_exit(void)
{
    printk("mpu6050 driver exit");
    i2c_del_driver(&mpu6050_driver);
}

module_init(mpu6050_driver_init);
module_exit(mpu6050_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JING");