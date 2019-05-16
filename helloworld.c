#include <linux/kernel.h>
#include <linux/module.h>

static int __init helloworld_init(void)
{
	printk(KERN_ERR "Hello World Kernel Module Enter!\n");
	return 0;
}

static void __exit helloworld_exit(void)
{
	printk(KERN_ERR "Hello World Kernel Module Exit!\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_VERSION("1.0");
MODULE_AUTHOR("ZiLin Chen");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A Simple Hello World Kernel Module");
