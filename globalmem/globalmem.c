#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>

#define MEM_CLEAR 0

#define GLOBALMEM_SIZE 512
#define GLOBALMEM_MAJOR 255

static struct class *globalmem_class;

static int globalmem_major = GLOBALMEM_MAJOR;
module_param(globalmem_major, int, S_IRUGO);

struct globalmem_dev
{
	struct cdev cdev;
	char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *globalmem_devp;

static int globalmem_open(struct inode *inode, struct file *filp)
{
	filp->private_data = globalmem_devp;
	return 0;
}

static int globalmem_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static long globalmen_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct globalmem_dev *dev = filp->private_data;

	switch(cmd) {
	case MEM_CLEAR:
		memset(dev->mem, 0, GLOBALMEM_SIZE);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p =  *ppos;
	unsigned int count = size;

	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;

	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (copy_to_user(buf, dev->mem + p, count)) {
		ret =  - EFAULT;
	}
	else {
		*ppos += count;
		ret = count;
	}
	return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p =  *ppos;
	unsigned int count = size;

	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;

	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
	count = GLOBALMEM_SIZE - p;

	if (copy_from_user(dev->mem + p, buf, count))
		ret =  - EFAULT;
	else {
		*ppos += count;
		ret = count;
	}

	return ret;
}

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
	loff_t ret = 0;

	switch (orig) {
	case 0:
		if (offset < 0) {
		ret =  - EINVAL;
		break;
		}
		if ((unsigned int)offset > GLOBALMEM_SIZE) {
			ret = - EINVAL;
			break;
		}
		filp->f_pos = (unsigned int)offset;
		ret = filp->f_pos;
		break;
	case 1:
		if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
			ret = - EINVAL;
			break;
		}
		if ((filp->f_pos + offset) < 0) {
			ret = - EINVAL;
			break;
		}
		filp->f_pos += offset;
		ret = filp->f_pos;
		break;
	default:
		ret = - EINVAL;
		break;
	}

	return ret;
}

static const struct file_operations globalmem_fops =
{
	.owner = THIS_MODULE,
	.llseek = globalmem_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	.unlocked_ioctl = globalmen_ioctl,
	.open = globalmem_open,
	.release = globalmem_release,
};

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
	int err;
	int devno = MKDEV(globalmem_major, index);

	cdev_init(&dev->cdev, &globalmem_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &globalmem_fops;

	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding globalmem%d", err, index);
}

static int __init globalmem_init(void)
{
	int result;
	dev_t devno = MKDEV(globalmem_major, 0);

	if (globalmem_major)
		result = register_chrdev_region(devno, 1, "globalmem");
	else {
		result = alloc_chrdev_region(&devno, 0, 1, "globalmem");
		globalmem_major = MAJOR(devno);
	}

	if (result < 0)
		return result;

	globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
	if (!globalmem_devp) {
		result =  - ENOMEM;
		goto fail_malloc;
	}
	memset(globalmem_devp, 0, sizeof(struct globalmem_dev));

	globalmem_setup_cdev(globalmem_devp, 0);

	globalmem_class = class_create(THIS_MODULE, "globalmem");
	if(IS_ERR(globalmem_class)) {
		printk(KERN_NOTICE"create globalmem_class failed!");
		return -1;
	}

	device_create(globalmem_class, NULL, MKDEV(globalmem_major, 0), NULL, "globalmem");
	return 0;

	fail_malloc: unregister_chrdev_region(devno, 1);
	return result;
}

static void __exit globalmem_exit(void)
{
	cdev_del(&globalmem_devp->cdev);
	kfree(globalmem_devp);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
	class_destroy(globalmem_class);
}

module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_VERSION("1.0");
MODULE_AUTHOR("ZiLin Chen");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A Simple Char Device Module");
