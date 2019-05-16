#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>

struct timer_list mytimer;

static void timer_timeout_handler(struct timer_list *timer)
{
	printk(KERN_ERR "timer: Running a timer function @ %lu\n", jiffies);

	mod_timer(timer, jiffies + 10*HZ);
}

static int __init timer_init(void)
{
	printk(KERN_ERR "timer: %s\n", __FUNCTION__);

	timer_setup(&mytimer, timer_timeout_handler, 0);
	mytimer.expires = jiffies + 10*HZ;	// timeout after 10s
	add_timer(&mytimer);

	return 0;
}

static void __exit timer_exit(void)
{
	printk(KERN_INFO "timer: %s\n", __FUNCTION__);
	del_timer(&mytimer);
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_VERSION("1.0");
MODULE_AUTHOR("ZiLin Chen");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A Simple Timer Test Module");
