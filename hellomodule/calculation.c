#include "calculation.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int __init calculation_init(void)
{
    printk(KERN_ALERT "calculation init\n");
    printk(KERN_ALERT "itype     = %d", itype);
    printk(KERN_ALERT "itype + 1 = %d", my_add(itype, 1));
    printk(KERN_ALERT "itype + 1 = %d", my_sub(itype, 1));
    return 0;
}

static void __exit calculation_exit(void)
{
    printk(KERN_ALERT "calculation exit\n");
    return 0;
}

module_init(calculation_init);
module_exit(calculation_exit);

MODULE_LICENSE("GPL2");
MODULE_AUTHOR("JING");
MODULE_DESCRIPTION("calculation module");
MODULE_ALIAS("test1_module");