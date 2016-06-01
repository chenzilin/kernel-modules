#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>


#define LEN_MSG 160
#define NAME_NODE "procfstest"
#define NAME_DIR  "procfstest_dir"

static char *get_rw_buf(void)
{
   static char buf_msg[LEN_MSG + 1] =
          ".........1.........2.........3.........4.........5\n";
   return buf_msg;
}

static ssize_t procfstest_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	char *buf_msg = get_rw_buf();
	int res;

	pr_info("read: %d bytes (ppos=%lld)", count, *ppos);
	if(*ppos >= strlen(buf_msg)) {
		*ppos = 0;
		pr_info("EOF");
		return 0;
	}
	if(count > strlen(buf_msg) - *ppos)
		count = strlen(buf_msg) - *ppos;
	res = copy_to_user((void*)buf, buf_msg + *ppos, count);
	*ppos += count;
	pr_info("return %d bytes", count);
	return count;
}

static ssize_t procfstest_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	char *buf_msg = get_rw_buf();
	int res, len = count < LEN_MSG ? count : LEN_MSG;
	pr_info("write: %d bytes\n", (int)count);
	res = copy_from_user(buf_msg, (void*)buf, len);
	buf_msg[len] = '\0';
	pr_info("put %d bytes\n", len);
	return len;
}

static const struct file_operations procfstest_fops = {
	.owner = THIS_MODULE,
	.read = procfstest_read,
	.write = procfstest_write
};

static int __init procfstest_init(void)
{
	struct proc_dir_entry *procfstest_proc_file;

	procfstest_proc_file = proc_create("procfstest", 0666, NULL, &procfstest_fops);
	if(procfstest_proc_file == NULL) {
		pr_err("create /proc/%s failed!\n", NAME_NODE);
		return -ENOENT;
	}
	pr_info("create /proc/%s success!\n", NAME_NODE);
	return 0;
}

static void __exit procfstest_exit(void)
{
	remove_proc_entry(NAME_NODE, NULL);
	pr_info("exit /proc/%s module.\n", NAME_NODE);
}

module_init(procfstest_init);
module_exit(procfstest_exit);

MODULE_VERSION("1.0");
MODULE_AUTHOR("ZiLin Chen");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A Simple Module Param Module");
