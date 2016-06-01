#include <linux/kernel.h> /* #:insmod moduleparam.ko age=12 name=LiuWeiKang */
#include <linux/module.h>

static int age = 25;
static char *name = "ZiLin Chen";
module_param(age, int, S_IRUGO);
module_param(name, charp, S_IRUGO);

static int __init moduleparam_init(void)
{
	printk(KERN_INFO "My age is %d\n", age);
	printk(KERN_INFO "My name is %s\n", name);
	return 0;
}

static void __exit moduleparam_exit(void)
{
	printk(KERN_INFO "Module Exit!\n");
}

module_init(moduleparam_init);
module_exit(moduleparam_exit);

MODULE_VERSION("1.0");
MODULE_AUTHOR("ZiLin Chen");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A Simple Module Param Module");
