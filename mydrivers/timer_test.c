#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/sched.h>

struct timer_list mytimer;

static void timer_do_things(unsigned long data)
{
	printk(KERN_INFO "timer: Running a timer function @ %lu\n", jiffies);

	/* Renew timer */
	mytimer.expires = jiffies + (HZ * 5);
	add_timer(&mytimer);
}

static int __init timer_init(void)
{
	printk(KERN_INFO "timer: %s\n", __FUNCTION__);
	init_timer(&mytimer);

	mytimer.data = 0;
	mytimer.function = timer_do_things;
	mytimer.expires = jiffies + (HZ * 5);

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
