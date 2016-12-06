#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>		/* for __init and __exit macros */
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for copy_from_user */
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/string.h>	/* for memset */

#define DRIVER_AUTHOR "Nikita Atroshenko <nikitatroshenko@github.com>"
#define DRIVER_DESC "Simple calculator. Reads left operand from /dev/left "\
			"right from /dev/right and operator from /dev/operator"\
			". Results are stored in /proc/result."

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

static ssize_t proc_read(struct file *, char *, size_t, loff_t *);

static ssize_t dev_left_write(struct file *, const char *, size_t, loff_t *);
static ssize_t dev_right_write(struct file *, const char *, size_t, loff_t *);
static ssize_t dev_operator_write(struct file *, const char *, size_t, loff_t *);


#define DEV_OPERATOR_NAME "operator"
#define DEV_LEFT_NAME "left"
#define DEV_RIGHT_NAME "right"
#define BUF_LEN 80		/* Max length of the message from the device */


static int Major_operator;	/* Operator dev major number */
static int Major_left;		/* Left operand dev major number */
static int Major_right;		/* Right operand dev major number */

static char result[BUF_LEN];
static char left[BUF_LEN];
static char right[BUF_LEN];
static char operator[BUF_LEN];


static struct file_operations proc_fops = {
	.read = proc_read,
};

static struct file_operations dev_left_fops = {
	.write = dev_left_write,
};

static struct file_operations dev_right_fops = {
	.write = dev_right_write,
};

static struct file_operations dev_operator_fops = {
	.write = dev_operator_write,
};

static struct proc_dir_entry *proc_entry_result;


static int __init calc_module_init(void)
{
	memset(result, '0', 3);
	proc_entry_result = proc_create( "result", 0, NULL, &proc_fops);

	Major_operator = register_chrdev(0, DEV_OPERATOR_NAME,
						&dev_operator_fops);
	Major_left = register_chrdev(0, DEV_LEFT_NAME, &dev_left_fops);
	Major_right = register_chrdev(0, DEV_RIGHT_NAME, &dev_right_fops);

	if (Major_operator < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n",
			Major_operator);
		return Major_operator;
	}

	if (Major_left < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n",
			Major_left);
		return Major_left;
	}


	if (Major_right < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n",
			Major_right);
		return Major_right;
	}


	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEV_OPERATOR_NAME,
		Major_operator);
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEV_LEFT_NAME,
		Major_left);
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEV_RIGHT_NAME,
		Major_right);

	return 0;
}

static void __exit calc_module_exit(void)
{
	unregister_chrdev(Major_operator, DEV_OPERATOR_NAME);
	unregister_chrdev(Major_left, DEV_LEFT_NAME);
	unregister_chrdev(Major_right, DEV_RIGHT_NAME);

	remove_proc_entry("result", NULL);
}

module_init(calc_module_init);
module_exit(calc_module_exit);

static ssize_t proc_read(struct file *filp,	/* see include/linux/fs.h   */
				char *buffer,	/* buffer to fill with data */
				size_t length,	/* length of the buffer	 */
				loff_t * offset)
{
	ssize_t cnt = strlen(result);
	ssize_t ret;
	int nleft;
	int nright;
	int nresult;

	sscanf(left, "%d", &nleft);
	sscanf(right, "%d", &nright);

	if (operator[0] == '*') {
		nresult = nleft * nright;
		sprintf(result, "%3d", nresult);
	} else if (operator[0] == '/') {
		nresult = nleft / nright;
		sprintf(result, "%3d", nresult);
	} else if (operator[0] == '-') {
		nresult = nleft - nright;
		sprintf(result, "%3d", nresult);
	} else if (operator[0] == '+') {
		nresult = nleft + nright;
		sprintf(result, "%3d", nresult);
	}

	ret = copy_to_user(buffer, result, cnt);
	*offset += cnt - ret;

	if (*offset > cnt)
		return 0;
	else
		return cnt;
}

static ssize_t dev_left_write(struct file *filp,
				const char *buff,
				size_t len,
				loff_t * off)
{
	if (copy_from_user(left, buff, len))
		return -EFAULT;
	return len;
}

static ssize_t dev_right_write(struct file *filp,
				const char *buff,
				size_t len,
				loff_t * off)
{
	if (copy_from_user(right, buff, len))
		return -EFAULT;
	return len;
}

static ssize_t dev_operator_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	if (copy_from_user(operator, buff, len))
		return -EFAULT;
	return len;
}

