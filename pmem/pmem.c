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

#include "get_pte.c"

//#define	DEBUG

#ifndef	DRV_NAME
#define	DRV_NAME	"pmem"
#endif
#ifndef	DRV_IDX
#define	DRV_IDX		(0)
#endif
#define	DRV_VERSION	"0.3.0"
#define	pmem_DRIVER_NAME	DRV_NAME " driver " DRV_VERSION

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0)
#define	__devinit
#define	__devexit
#define	__devexit_p
#endif

unsigned char paged_buf[4096]__attribute__((aligned(4096)));
int paging_level;


#ifdef __aarch64__
int get_paging_level( void )
{
	int rc;

	rc = 4;
	printk( KERN_INFO "MMU %d Level paging\n", rc );

	return ( rc );
}

pte_t *get_pte(unsigned long vaddr)
{
	return get_pte4( vaddr );
}
#endif


#ifdef __x86_64__
pte_t *get_pte(unsigned long vaddr)
{
	if (paging_level==4)
		return get_pte4( vaddr );
	else
		return get_pte5( vaddr );
}

int get_paging_level( void )
{
	u64 cpu_cr3, cpu_cr4;
	int rc;

	get_cr34( &cpu_cr3, &cpu_cr4 );
	if ( cpu_cr4 & X86_CR4_LA57)
		rc = 5;
	else
		rc = 4;

	printk( KERN_INFO "CR3=%x CR4=%x (MMU %d Level paging)\n", cpu_cr3, cpu_cr4, rc );

	return ( rc );
}
#endif


static int pmem_mmap(struct file *filp, struct vm_area_struct *vma)
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

static int pmem_open(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);

	return 0;
}

static ssize_t pmem_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *ppos)
{
	unsigned long copy_len, left_len;
	pte_t *pte = 0;
	unsigned long pte_save;
#ifdef DEBUG
	printk("%s\n", __func__);
#endif

	left_len = 0x1000 - (filp->f_pos & 0xfff);
	if (count <= left_len)
		copy_len = count;
	else
		copy_len = left_len;
	if ( (pte = get_pte((unsigned long long)paged_buf)) ) {
		pte_save = pte->pte;
		pte->pte = (filp->f_pos & ~0xfff) | (pte_save & 0xfff);
	}

	if ( copy_to_user( buf, (unsigned char *)paged_buf+(filp->f_pos & 0xfff), copy_len ) ) {
		printk( KERN_INFO "copy_to_user failed\n" );
		return -EFAULT;
	}

	if (pte) {
		pte->pte = pte_save;
	}

	return copy_len;
}

static ssize_t pmem_write(struct file *filp, const char __user *buf,
			    size_t count, loff_t *ppos)

{
	unsigned long copy_len, left_len;
	pte_t *pte = 0;
	unsigned long pte_save;
#ifdef DEBUG
	printk("%s\n", __func__);
#endif

	left_len = 0x1000 - (filp->f_pos & 0xfff);
	if (count <= left_len)
		copy_len = count;
	else
		copy_len = left_len;


	if ( (pte = get_pte((unsigned long long)paged_buf)) ) {
		pte_save = pte->pte;
		pte->pte = (filp->f_pos & ~0xfff) | (pte_save & 0xfff);
#ifdef __aarch64__
		pte->pte |= (PTE_RDONLY | PTE_USER);
#endif
#ifdef __x86_64__
		pte->pte |= (_PAGE_RW);
#endif
	}

	if ( copy_from_user( (unsigned char *)paged_buf+(filp->f_pos & 0xfff), buf, copy_len ) ) {
		printk( KERN_INFO "copy_from_user failed\n" );
		return -EFAULT;
	}

	if (pte) {
		pte->pte = pte_save;
	}

	return copy_len;
}

static int pmem_release(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);

	return 0;
}


static long pmem_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	unsigned long long *ptr, ret;
	printk("%s\n", __func__);
	if (cmd == 1) {
		ptr = (unsigned long long *)arg;
printk( "VA=%llx\n", *ptr);
		ret = (unsigned long long)get_pte(*ptr);
printk( "PA=%llx\n", ret);
		*ptr = ret;
		return 0;
	}

	return  -ENOTTY;
}

static loff_t pmem_llseek(struct file *flip, loff_t offset, int whence)
{
	if (whence == SEEK_SET)
		flip->f_pos = offset;
	else if (whence == SEEK_CUR)
		flip->f_pos += offset;
	else if (whence == SEEK_END)
		flip->f_pos = ~0LL + offset;

	return flip->f_pos;
}


static struct file_operations pmem_fops = {
	.owner		= THIS_MODULE,
	.read		= pmem_read,
	.write		= pmem_write,
	.compat_ioctl	= pmem_ioctl,
	.llseek		= pmem_llseek,
	.mmap		= pmem_mmap,
	.open		= pmem_open,
	.release	= pmem_release,
};

static struct miscdevice pmem_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DRV_NAME,
	.fops = &pmem_fops,
};

static int __init pmem_init(void)
{
	int ret;

	printk( KERN_INFO "Physical MEMory access driver %s: Copyright (c) 2014-2025 Takeshi Matsuya\n", DRV_VERSION );
	paging_level = get_paging_level();


#ifdef MODULE
	pr_info(pmem_DRIVER_NAME "\n");
#endif
	printk("%s\n", __func__);

	/* register character device */
	ret = misc_register(&pmem_dev);
	if (ret) {
		printk("fail to misc_register (MISC_DYNAMIC_MINOR)\n");
		goto error;
	}

	return 0;

error:

	return ret;
}

static void __exit pmem_cleanup(void)
{
	misc_deregister(&pmem_dev);

	printk("%s\n", __func__);
}

MODULE_LICENSE("GPL");
module_init(pmem_init);
module_exit(pmem_cleanup);

MODULE_DESCRIPTION("Physical Memory Access driver");
MODULE_AUTHOR("<macchan@sfc.wide.ad.jp>");
MODULE_LICENSE("GPL");
