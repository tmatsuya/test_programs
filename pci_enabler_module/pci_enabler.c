#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/wait.h>		/* wait_queue_head_t */
#include <linux/sched.h>	/* wait_event_interruptible, wake_up_interruptible */
#include <linux/interrupt.h>
#include <linux/version.h>

#ifndef DRV_NAME
#define DRV_NAME	"pci_enabler"
#endif

#define	DRV_VERSION	"0.0.1"
#define	pci_enabler_DRIVER_NAME	DRV_NAME " PCI enabler driver " DRV_VERSION

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0)
#define	__devinit
#define	__devexit
#define	__devexit_p
#endif

static DEFINE_PCI_DEVICE_TABLE(pci_enabler_pci_tbl) = {
	{0x3776, 0x8011, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{0,}
};
MODULE_DEVICE_TABLE(pci, pci_enabler_pci_tbl);

static int __devinit pci_enabler_init_one (struct pci_dev *pdev,
				       const struct pci_device_id *ent)
{
        unsigned char *mmio0_ptr = 0L, *mmio1_ptr = 0L;
        unsigned long mmio0_start, mmio0_end, mmio0_flags, mmio0_len;
        unsigned long mmio1_start, mmio1_end, mmio1_flags, mmio1_len;
        static struct pci_dev *pcidev = NULL;
	int rc;
	static char name[16];
	static int board_idx = -1;

	mmio0_ptr = 0L;
	mmio1_ptr = 0L;

	rc = pci_enable_device (pdev);
	if (rc)
		goto err_out;

	rc = pci_request_regions (pdev, DRV_NAME);
	if (rc)
		goto err_out;

	++board_idx;

	printk( KERN_INFO "board_idx: %d\n", board_idx );

	pci_set_master (pdev);		/* set BUS Master Mode */

	mmio0_start = pci_resource_start (pdev, 0);
	mmio0_end   = pci_resource_end   (pdev, 0);
	mmio0_flags = pci_resource_flags (pdev, 0);
	mmio0_len   = pci_resource_len   (pdev, 0);

	printk( KERN_INFO "mmio0_start: %X\n", (unsigned int)mmio0_start );
	printk( KERN_INFO "mmio0_end  : %X\n", (unsigned int)mmio0_end   );
	printk( KERN_INFO "mmio0_flags: %X\n", (unsigned int)mmio0_flags );
	printk( KERN_INFO "mmio0_len  : %X\n", (unsigned int)mmio0_len   );

	mmio0_ptr = ioremap(mmio0_start, mmio0_len);
	if (!mmio0_ptr) {
		printk(KERN_ERR "cannot ioremap MMIO0 base\n");
		goto err_out;
	}

	mmio1_start = pci_resource_start (pdev, 2);
	mmio1_end   = pci_resource_end   (pdev, 2);
	mmio1_flags = pci_resource_flags (pdev, 2);
	mmio1_len   = pci_resource_len   (pdev, 2);

	printk( KERN_INFO "mmio1_start: %X\n", (unsigned int)mmio1_start );
	printk( KERN_INFO "mmio1_end  : %X\n", (unsigned int)mmio1_end   );
	printk( KERN_INFO "mmio1_flags: %X\n", (unsigned int)mmio1_flags );
	printk( KERN_INFO "mmio1_len  : %X\n", (unsigned int)mmio1_len   );

	mmio1_ptr = ioremap_wc(mmio1_start, mmio1_len);
	if (!mmio1_ptr) {
		printk(KERN_ERR "cannot ioremap MMIO1 base\n");
		goto err_out;
	}

	pcidev = pdev;

	return 0;
err_out:
	pci_release_regions (pdev);
	pci_disable_device (pdev);
	return -1;
}


static void __devexit pci_enabler_remove_one (struct pci_dev *pdev)
{
#ifdef macchan
	if (mmio0_ptr) {
		iounmap(mmio0_ptr);
		mmio0_ptr = 0L;
	}
	if (mmio1_ptr) {
		iounmap(mmio1_ptr);
		mmio1_ptr = 0L;
	}
#endif
	pci_release_regions (pdev);
	pci_disable_device (pdev);
	printk("%s\n", __func__);
}


static struct pci_driver pci_enabler_pci_driver = {
	.name		= DRV_NAME,
	.id_table	= pci_enabler_pci_tbl,
	.probe		= pci_enabler_init_one,
	.remove		= __devexit_p(pci_enabler_remove_one),
};


static int __init pci_enabler_init(void)
{

#ifdef MODULE
	pr_info(pci_enabler_DRIVER_NAME "\n");
#endif

	printk("%s\n", __func__);
	return pci_register_driver(&pci_enabler_pci_driver);
}

static void __exit pci_enabler_cleanup(void)
{
	printk("%s\n", __func__);
	pci_unregister_driver(&pci_enabler_pci_driver);
}

MODULE_LICENSE("GPL");
module_init(pci_enabler_init);
module_exit(pci_enabler_cleanup);

