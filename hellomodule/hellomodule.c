#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

// pass param to kernel module
// sudo insmod module_param.ko itype=123 btype=1 ctype=200 stype=abc
static int itype = 0;
module_param(itype, int, 0);

static bool btype = false;
module_param(btype, bool, 0644);

static char ctype = 0;
module_param(ctype, byte, 0);

static char* stype = 0;
module_param(stype, charp, 0644);

EXPORT_SYMBOL(itype);

int my_add(int a, int b)
{
    return a + b;
}

EXPORT_SYMBOL(my_add);

int my_sub(int a, int b)
{
    return a - b;
}

EXPORT_SYMBOL(my_sub);


static int __init hello_init(void)
{
    printk(KERN_EMERG "[KERN_EMERG] Hello Module Init\n");
    printk("[default] Hello Module Init\n");

    printk(KERN_ALERT "param init!\n");
    printk(KERN_ALERT "itype=%d\n", itype);
    printk(KERN_ALERT "btype=%d\n", btype);
    printk(KERN_ALERT "ctype=%d\n", ctype);
    printk(KERN_ALERT "stype=%s\n", stype);
    return 0;
}

static void __exit hello_exit(void)
{
    printk("[default] GoodBye Hello Module\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL2");
MODULE_AUTHOR("JING");
MODULE_DESCRIPTION("hello module");
MODULE_ALIAS("test_module");