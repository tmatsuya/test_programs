#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/string.h>
#include <linux/wait.h>		/* wait_queue_head_t */
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>

//#define	DEBUG

#ifndef	DRV_NAME
#define	DRV_NAME	"kmem"
#endif
#ifndef	DRV_IDX
#define	DRV_IDX		(0)
#endif
#define	DRV_VERSION	"0.0.0"
#define	kmem_DRIVER_NAME	DRV_NAME " kmem driver " DRV_VERSION

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0)
#define	__devinit
#define	__devexit
#define	__devexit_p
#endif

static unsigned long long kmem_position = 0LL;

static int kmem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long page, pos;

	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT))
		return -EINVAL;

	pos = offset;

	while (size > 0) {
		page = vmalloc_to_pfn((void *)pos);
		if (remap_pfn_range(vma, start, page, PAGE_SIZE, PAGE_SHARED)) {
			return -EAGAIN;
		}
		start += PAGE_SIZE;
		pos += PAGE_SIZE;
		if (size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}
	
	return 0;
}

static int kmem_open(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);
	kmem_position = 0LL;

	return 0;
}

static ssize_t kmem_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *ppos)
{
	int copy_len;
#ifdef DEBUG
	printk("%s\n", __func__);
#endif

	copy_len = count;

	if ( copy_to_user( buf, (unsigned char *)kmem_position, copy_len ) ) {
		printk( KERN_INFO "copy_to_user failed\n" );
		return -EFAULT;
	}

	kmem_position += copy_len;

	return copy_len;
}

static ssize_t kmem_write(struct file *filp, const char __user *buf,
			    size_t count, loff_t *ppos)

{
	int copy_len;
#ifdef DEBUG
	printk("%s\n", __func__);
#endif

	copy_len = count;

	if ( copy_from_user( (unsigned char *)kmem_position, buf, copy_len ) ) {
		printk( KERN_INFO "copy_from_user failed\n" );
		return -EFAULT;
	}

	kmem_position += copy_len;

	return copy_len;
}

static int kmem_release(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);

	return 0;
}


static long kmem_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	printk("%s\n", __func__);
	return  -ENOTTY;
}

static loff_t kmem_llseek(struct file *flip, loff_t offset, int whence)
{
	if (whence == SEEK_SET)
		kmem_position = offset;
	else if (whence == SEEK_CUR)
		kmem_position += offset;
	else if (whence == SEEK_END)
		kmem_position = ~0LL + offset;

	return kmem_position;
}


static struct file_operations kmem_fops = {
	.owner		= THIS_MODULE,
	.read		= kmem_read,
	.write		= kmem_write,
	.compat_ioctl	= kmem_ioctl,
	.llseek		= kmem_llseek,
	.mmap		= kmem_mmap,
	.open		= kmem_open,
	.release	= kmem_release,
};

static struct miscdevice kmem_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DRV_NAME,
	.fops = &kmem_fops,
};

static int __init kmem_init(void)
{
	int ret;

#ifdef MODULE
	pr_info(kmem_DRIVER_NAME "\n");
#endif
	printk("%s\n", __func__);

	/* register character device */
	ret = misc_register(&kmem_dev);
	if (ret) {
		printk("fail to misc_register (MISC_DYNAMIC_MINOR)\n");
		goto error;
	}

	return 0;

error:

	return ret;
}

static void __exit kmem_cleanup(void)
{
	misc_deregister(&kmem_dev);

	printk("%s\n", __func__);
}

MODULE_LICENSE("GPL");
module_init(kmem_init);
module_exit(kmem_cleanup);
