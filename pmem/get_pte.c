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


#ifdef __x86_64__
void get_cr34( u64 *cpu_cr3, u64 *cpu_cr4 )
{
	u64 cr3, cr4;
	__asm__ __volatile__ (
		"mov %%cr3, %%rax\n\t"
		"mov %%eax, %0\n\t"
		"mov %%cr4, %%rax\n\t"
		"mov %%eax, %1\n\t"
	: "=m" (cr3), "=m" (cr4)
	: /* no input */
	: "%rax"
	);
	*cpu_cr3 = cr3;
	*cpu_cr4 = cr4;
}
#endif


pte_t *get_pte(unsigned long long vaddr)
{
	pgd_t *pgd = pgd_offset(current->mm, vaddr);
	//unsigned long *pgd = pgd_offset(current->mm, vaddr);
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte = 0;

	/* to lock the page */
	struct page *pg;
	unsigned long long paddr;

	if (!pgd_present(*pgd)) {
		printk(KERN_ALERT "[nskk] Alert: pgd not present %lu\n", *pgd);
		goto out;
	}

	pud = pud_offset((p4d_t *)pgd, vaddr);
	if (!pud_present(*pud) || pud_large(*pud)) {
		printk(KERN_ALERT "[nskk] Alert: pud not present %lu\n", *pud);
		goto out;
	}

	pmd = pmd_offset(pud, vaddr);
	if (!pmd_present(*pmd) || pmd_large(*pmd)) {
		printk(KERN_ALERT "[nskk] Alert: pmd not present %lu\n", *pmd);
		goto out;
	}

	pte = pte_offset_kernel(pmd, vaddr);
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
