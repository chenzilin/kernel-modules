#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/module.h>
#include <asm/uaccess.h>

#define DEV_SIZE 100
#define P_DEBUG(fmt, args...)   printk("<kernel>[%s]"fmt, __FUNCTION__, ##args)

struct _test_t{
    unsigned int major;
    unsigned int minor;
    char kbuf[DEV_SIZE];
    unsigned int cur_size;
    dev_t devno;
    struct cdev test_cdev;
};

int test_open(struct inode *node, struct file *filp)
{
    struct _test_t *dev;
    dev = container_of(node->i_cdev, struct _test_t, test_cdev);
    filp->private_data = dev;
    return 0;
}

int test_close(struct inode *node, struct file *filp)
{
    return 0;
}

ssize_t test_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    int ret;
    struct _test_t *dev = filp->private_data;

    if (*offset >= DEV_SIZE) {
        return count ? -ENXIO : 0;
    }

    if (*offset + count > DEV_SIZE) {
        count = DEV_SIZE - *offset;
    }

    if (copy_to_user(buf, dev->kbuf + *offset, count)) {
        ret = -EFAULT;
    }
    else {
        ret = count;
        dev->cur_size -= count;
        *offset += count;
        P_DEBUG("read %d bytes, cur_size:[%d]\n", count, dev->cur_size);
    }

    return ret;
}

ssize_t test_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    int ret;
    struct _test_t *dev = filp->private_data;

    if (*offset >= DEV_SIZE) {
        return count ? -ENXIO : 0;
    }

    if (*offset + count > DEV_SIZE) {
        count = DEV_SIZE - *offset;
    }

    if (copy_from_user(dev->kbuf + *offset, buf, count)) {
        ret = -EFAULT;
    }
    else {
        ret = count;
        dev->cur_size += count;
        *offset += count;
        P_DEBUG("write %d bytes, cur_size:[%d]\n", count, dev->cur_size);
        P_DEBUG("kbuf is [%s]\n", dev->kbuf);
    }

    return ret;
}

loff_t test_llseek(struct file *filp, loff_t offset, int whence)
{
    loff_t new_pos;
    loff_t old_pos = filp->f_pos;

    switch(whence){
    case SEEK_SET:
        new_pos = offset;
        break;
    case SEEK_CUR:
        new_pos = old_pos + offset;
        break;
    case SEEK_END:
        new_pos = DEV_SIZE + offset;
        break;
    default:
        P_DEBUG("unknow whence\n");
        return -EINVAL;
    }

    if (new_pos < 0 || new_pos > DEV_SIZE) {
        P_DEBUG("f_pos failed\n");
        return -EINVAL;
    }

    filp->f_pos = new_pos;
    return new_pos;
}

struct file_operations test_fops = {
    .open = test_open,
    .release = test_close,
    .write = test_write,
    .read = test_read,
    .llseek = test_llseek,
};

struct _test_t my_dev;

static int __init test_init(void)
{
    int result = 0;
    my_dev.major = 0;
    my_dev.minor = 0;
    my_dev.cur_size = 0;

    if (my_dev.major) {
        my_dev.devno = MKDEV(my_dev.major, my_dev.minor);
        result = register_chrdev_region(my_dev.devno, 1, "test new driver");
    }
    else {
        result = alloc_chrdev_region(&my_dev.devno, my_dev.minor, 1, "test alloc diver");
        my_dev.major = MAJOR(my_dev.devno);
        my_dev.minor = MINOR(my_dev.devno);
    }

    if (result < 0) {
        P_DEBUG("register devno errno!\n");
        return result;
    }

    printk("major[%d] minor[%d]\n", my_dev.major, my_dev.minor);

    cdev_init(&my_dev.test_cdev, &test_fops);
    my_dev.test_cdev.owner = THIS_MODULE;

    result = cdev_add(&my_dev.test_cdev, my_dev.devno, 1);
    if (result < 0) {
        P_DEBUG("cdev_add errno!\n");

        unregister_chrdev_region(my_dev.devno, 1);
        return result;
    }

    return result;
}

static void __exit test_exit(void)
{
    cdev_del(&my_dev.test_cdev);
    unregister_chrdev_region(my_dev.devno, 1);
}

module_init(test_init);
module_exit(test_exit);

MODULE_VERSION("1.0");
MODULE_AUTHOR("ChenZiLin");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A Simple Test Module");
