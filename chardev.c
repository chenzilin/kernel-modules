#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define BUF_SIZE 100

struct simple_dev {
	size_t major;
	size_t minor;
	dev_t devno;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	size_t cur_size;
	char kbuf[BUF_SIZE];
};

static int simple_cdev_open(struct inode *inode, struct file *file)
{
	struct simple_dev *dev;

	dev = container_of(inode->i_cdev, struct simple_dev, cdev);
	file->private_data = dev;

	return 0;
}

static int simple_cdev_close(struct inode *inode, struct file *file)
{
	struct simple_dev *dev;

	dev = file->private_data;

	return 0;
}

static ssize_t simple_cdev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	int ret;
	struct simple_dev *dev = filp->private_data;

	if (*offset >= BUF_SIZE) {
		return count ? -ENXIO : 0;
	}

	if (*offset + count > BUF_SIZE) {
		count = BUF_SIZE - *offset;
	}

	if (copy_to_user(buf, dev->kbuf + *offset, count)) {
		ret = -EFAULT;
	}
	else {
		ret = count;
		dev->cur_size -= count;
		*offset += count;
		printk(KERN_ERR "read %zu bytes, cur_size:[%zu]\n", count, dev->cur_size);
	}

	return ret;
}

static ssize_t simple_cdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	int ret;
	struct simple_dev *dev = filp->private_data;

	if (*offset >= BUF_SIZE) {
		return count ? -ENXIO : 0;
	}

	if (*offset + count > BUF_SIZE) {
		count = BUF_SIZE - *offset;
	}

	if (copy_from_user(dev->kbuf + *offset, buf, count)) {
		ret = -EFAULT;
	}
	else {
		ret = count;
		dev->cur_size += count;
		*offset += count;
		printk(KERN_ERR "write %zu bytes, cur_size:[%zu]\n", count, dev->cur_size);
	}

	return ret;
}

static loff_t simple_cdev_llseek(struct file *filp, loff_t offset, int whence)
{
	loff_t new_pos;
	loff_t old_pos = filp->f_pos;

	switch (whence) {
	case SEEK_SET:
		new_pos = offset;
	break;
	case SEEK_CUR:
		new_pos = old_pos + offset;
	break;
	case SEEK_END:
		new_pos = BUF_SIZE + offset;
	break;
	default:
		printk(KERN_ERR "unknow whence\n");
	return -EINVAL;
	}

	if (new_pos < 0 || new_pos > BUF_SIZE) {
		printk(KERN_ERR "f_pos failed\n");
		return -EINVAL;
	}

	filp->f_pos = new_pos;
	return new_pos;
}

static const struct file_operations simple_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= simple_cdev_open,
	.release	= simple_cdev_close,
	.write		= simple_cdev_write,
	.read		= simple_cdev_read,
	.llseek		= simple_cdev_llseek,
};

static struct simple_dev my_dev;

static int __init simple_cdev_init(void)
{
	int ret;
	my_dev.major = 0;
	my_dev.minor = 0;
	my_dev.cur_size = 0;

	if (my_dev.major) {
		my_dev.devno = MKDEV(my_dev.major, my_dev.minor);
		ret = register_chrdev_region(my_dev.devno, 1, "test new driver");
	}
	else {
		ret = alloc_chrdev_region(&my_dev.devno, my_dev.minor, 1, "test alloc diver");
		my_dev.major = MAJOR(my_dev.devno);
		my_dev.minor = MINOR(my_dev.devno);
	}

	if (ret < 0) {
		printk(KERN_ERR "alloc chrdev_region failed\n");
		goto alloc_chrdev_failed;
	}

	cdev_init(&my_dev.cdev, &simple_dev_fops);
	my_dev.cdev.owner = THIS_MODULE;

	ret = cdev_add(&my_dev.cdev, my_dev.devno, 1);
	if (ret < 0) {
		printk(KERN_ERR "cdev_add failed\n");
		goto cdev_add_failed;
	}

	my_dev.class = class_create(THIS_MODULE, "simple_cdev");
	if (!&my_dev.class) {
		printk(KERN_ERR "class_create failed\n");
		goto class_create_failed;
	}

	my_dev.device = device_create(my_dev.class, NULL, my_dev.devno, NULL, "simple_cdev");
	if (!my_dev.device) {
		printk(KERN_ERR "device_create failed\n");
		goto device_create_failed;
	}

	return 0;

device_create_failed:
	class_destroy(my_dev.class);
class_create_failed:
	cdev_del(&my_dev.cdev);
cdev_add_failed:
	unregister_chrdev_region(my_dev.devno, 1);
alloc_chrdev_failed:
	return -1;
}

static void __exit simple_cdev_exit(void)
{
	device_destroy(my_dev.class, my_dev.devno);
	class_destroy(my_dev.class);
	cdev_del(&my_dev.cdev);
	unregister_chrdev_region(my_dev.devno, 1);
}

module_init(simple_cdev_init);
module_exit(simple_cdev_exit);

MODULE_VERSION("1.0");
MODULE_AUTHOR("ZiLin Chen");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A Simple CharDev Test Module");