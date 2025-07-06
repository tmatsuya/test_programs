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
#define	DRV_NAME	"kmem"
#endif
#ifndef	DRV_IDX
#define	DRV_IDX		(0)
#endif
#define	DRV_VERSION	"0.3.0"
#define	kmem_DRIVER_NAME	DRV_NAME " driver " DRV_VERSION

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
	return ( 4 );
}

pte_t *get_pte(unsigned long vaddr)
{
	return (pte_t *)NULL;
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

	return 0;
}

static ssize_t kmem_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *ppos)
{
	unsigned long copy_len, left_len;
	unsigned long long addr, phy_addr;
	pte_t *pte = 0;
	unsigned long pte_save;
#ifdef DEBUG
	printk("%s\n", __func__);
#endif

	addr = (filp->f_pos & 0x0f00000000000000LL) ? (filp->f_pos | 0xf000000000000000LL) : filp->f_pos;

	left_len = 0x1000 - (filp->f_pos & 0xfff);
	if (count <= left_len)
		copy_len = count;
	else
		copy_len = left_len;

	if (addr < 0xffffffff80000000LL)
		phy_addr = pte_val(*get_pte( addr )) & PAGE_MASK;
	else if (addr < 0xffffffffc0000000LL)
		phy_addr = virt_to_phys( addr );
	else
		phy_addr = pte_val(*get_pte( addr )) & PAGE_MASK;

	if ( (pte = get_pte((unsigned long long)paged_buf)) ) {
		pte_save = pte->pte;
		pte->pte = (phy_addr & ~0xfffLL) | (pte_save & 0xfff);
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

static ssize_t kmem_write(struct file *filp, const char __user *buf,
			    size_t count, loff_t *ppos)

{
	unsigned long copy_len, left_len;
	unsigned long long addr, phy_addr;
	pte_t *pte = 0;
	unsigned long pte_save;
	unsigned char tbuf[4096];
#ifdef DEBUG
	printk("%s\n", __func__);
#endif

	addr = (filp->f_pos & 0x0f00000000000000LL) ? (filp->f_pos | 0xf000000000000000LL) : filp->f_pos;
	left_len = 0x1000 - (filp->f_pos & 0xfff);
	if (count <= left_len)
		copy_len = count;
	else
		copy_len = left_len;

	if (addr < 0xffffffff80000000LL)
		phy_addr = pte_val(*get_pte( addr )) & PAGE_MASK;
	else if (addr < 0xffffffffc0000000LL)
		phy_addr = virt_to_phys( addr );
	else
		phy_addr = pte_val(*get_pte( addr )) & PAGE_MASK;

	if ( (pte = get_pte((unsigned long long)paged_buf)) ) {
		pte_save = pte->pte;
		pte->pte = (phy_addr & ~0xfff) | (pte_save & 0xfff);
#ifdef __aarch64__
		pte->pte |= (PTE_RDONLY | PTE_USER);
#endif
#ifdef __x86_64__
		pte->pte |= (_PAGE_RW);
#endif
	}

	if ( copy_from_user( tbuf, buf, copy_len ) ) {
		printk( KERN_INFO "copy_from_user failed\n" );
		return -EFAULT;
	}
	// 実際にカーネル領域に書き込みを行う(ここは危険)
	memcpy( paged_buf+(filp->f_pos & 0xfff), tbuf, copy_len );

	if (pte) {
		pte->pte = pte_save;
	}


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
	unsigned long long *ptr, ret;
	printk("%s\n", __func__);

	return  -ENOTTY;
}

static loff_t kmem_llseek(struct file *flip, loff_t offset, int whence)
{
	if (whence == SEEK_SET)
		flip->f_pos = offset;
	else if (whence == SEEK_CUR)
		flip->f_pos += offset;
	else if (whence == SEEK_END)
		flip->f_pos = ~0LL + offset;

	flip->f_pos &= 0x0fffffffffffffffLL;

	return flip->f_pos;
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
	u64 cpu_cr3, cpu_cr4;

	printk( KERN_INFO "Kernel MEMory access driver %s: Copyright (c) 2014-2025 Takeshi Matsuya\n", DRV_VERSION );
	paging_level = get_paging_level();


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

MODULE_DESCRIPTION("Kernel Memory Access driver");
MODULE_AUTHOR("<macchan@sfc.wide.ad.jp>");
MODULE_LICENSE("GPL");
