# 内核API

## 1 字符设备

+ struct cdev

```C++
struct cdev 
{
    struct kobject kobj;
    struct module *owner;
    const struct file_operations *ops;
    struct list_head list;
    dev_t dev;
    unsigned int count;
}
```

字符设备基本对象，里面记录了`file_operations`，通过cdev_init将`file_operations`记录到cdev对象中。

+ register_chrdev_region  
`int register_chrdev_region(dev_t from, unsigned count, const char *name)`  
作用：将指定的设备号注册到内核中，防止设备号冲突  
from：指定的设备号  
count：次设备号数量  
name：设备号名称  

+ alloc_chrdev_region  
`int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)`  
作用：向内核申请一个设备号  
dev：申请的结果  
baseminor：次设备号的最小值  
count：次设备号的数量  
name：设备号的名称  

+ unregister_chrdev_region  
`void unregister_chrdev_region(dev_t from, unsigned count)`  
作用：通知内核注销一个设备号  
from：需要注销的设备号  
count：次设备号的数量  

+ cdev_init  
`void cdev_init(struct cdev *cdev, const struct file_operations *fops)`  
作用：初始化cdev，内核通过主次设备号记录`file_operations`。  
cdev：需要初始化的cdev  
fops：需要绑定的`file_operations`  

+ cdev_add  
`int cdev_add(struct cdev *p, dev_t dev, unsigned count)`  
作用：将cdev绑定设备号，并且注册到内核中  
p：需要注册到内核中的cdev  
dev：设备号  
count：次设备号的数量  

+ cdev_del  
`void cdev_del(struct cdev *p)`  
作用：从内核中注销cdev  

+ device_create  

```C
struct device *device_create(struct class *class, struct device *parent,
            dev_t devt, void *drvdata, const char *fmt, ...)
```

作用: 创建一个设备并将其注册到文件系统  

+ device_destroy  

```C
void device_destroy(struct class *class, dev_t devt)
```

作用：删除使用device_create函数创建的设备  

+ container_of  
