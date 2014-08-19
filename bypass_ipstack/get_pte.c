#include <linux/semaphore.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/version.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/rtnetlink.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#include <asm/pgtable_types.h>

static int bad_address(void *p)
{
	unsigned long dummy;
	return probe_kernel_address((unsigned long*)p, dummy);
}

pte_t *get_pte(unsigned long long vaddr)
{
	pgd_t *pgd = pgd_offset(current->mm, vaddr);
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte = 0;

	/* to lock the page */
	struct page *pg;
//	unsigned long long paddr;

	if (bad_address(pgd)) {
		printk(KERN_ALERT "[nskk] Alert: bad address of pgd %p\n", pgd);
		goto bad;
	}
	if (!pgd_present(*pgd)) {
		printk(KERN_ALERT "[nskk] Alert: pgd not present %lu\n", *pgd);
		goto out;
	}

	pud = pud_offset(pgd, vaddr);
	if (bad_address(pud)) {
		printk(KERN_ALERT "[nskk] Alert: bad address of pud %p\n", pud);
		goto bad;
	}
	if (!pud_present(*pud) || pud_large(*pud)) {
		printk(KERN_ALERT "[nskk] Alert: pud not present %lu\n", *pud);
		goto out;
	}

	pmd = pmd_offset(pud, vaddr);
	if (bad_address(pmd)) {
		printk(KERN_ALERT "[nskk] Alert: bad address of pmd %p\n", pmd);
		goto bad;
	}
	if (!pmd_present(*pmd) || pmd_large(*pmd)) {
		printk(KERN_ALERT "[nskk] Alert: pmd not present %lu\n", *pmd);
		goto out;
	}

	pte = pte_offset_kernel(pmd, vaddr);
	if (bad_address(pte)) {
		printk(KERN_ALERT "[nskk] Alert: bad address of pte %p\n", pte);
		goto bad;
	}
	if (!pte_present(*pte)) {
		printk(KERN_ALERT "[nskk] Alert: pte not present %lu\n", *pte);
		goto out;
	}

	pg = pte_page(*pte);
//	pte->pte |= _PAGE_RW; // | _PAGE_USER;
//	paddr = pte_val(*pte);

out:
	return pte;
bad:
	printk(KERN_ALERT "[nskk] Alert: Bad address\n");
	return 0;
}
