#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/jiffies.h>

#define PROC_NAME "tsulab"

static struct proc_dir_entry *tsulab_file;

static ssize_t tsulab_read(struct file *file, char __user *buffer, size_t buffer_length, loff_t *offset) {
	char message[128];
	ssize_t len;
	u64 uptime_seconds;

	if (*offset != 0)
		return 0;

	uptime_seconds = get_jiffies_64() / HZ;

	len = snprintf(message, sizeof(message), "Tomsk State University uptime: %llu seconds\n", (unsigned long long)uptime_seconds);

	if (len < 0)
		return len;

	if ((size_t)len > buffer_length)
		len = buffer_length;

	if (copy_to_user(buffer, message, len))
		return -EFAULT;

	*offset += len;
	return len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops tsulab_proc_ops = { .proc_read = tsulab_read, };
#else
static const struct file_operations tsulab_file_ops = { .read = tsulab_read, };
#endif

static int __init tsulab_init(void)
{
	pr_info("Welcome to the Tomsk State University\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	tsulab_file = proc_create(PROC_NAME, 0444, NULL, &tsulab_proc_ops);
#else
	tsulab_file = proc_create(PROC_NAME, 0444, NULL, &tsulab_file_ops);
#endif

	if (!tsulab_file) {
		pr_err("tsulab: failed to create /proc/%s\n", PROC_NAME);
		return -ENOMEM;
	}

	pr_info("tsulab: /proc/%s created\n", PROC_NAME);
	return 0;
}

static void __exit tsulab_exit(void)
{
	if (tsulab_file)
		proc_remove(tsulab_file);

	pr_info("Tomsk State University forever!\n");
}

module_init(tsulab_init);
module_exit(tsulab_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SVYATOSLAV");
MODULE_DESCRIPTION("TSU lab: /proc/tsulab example");
