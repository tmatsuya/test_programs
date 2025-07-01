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

#ifdef __aarch64__
void get_cr34( u64 *cpu_cr3, u64 *cpu_cr4 )
{
}


#endif

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


pte_t *get_pte4(unsigned long vaddr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long paddr = 0;
	unsigned long page_addr = 0;
	unsigned long page_offset = 0;

	pgd = pgd_offset(current->mm, vaddr);
#ifdef DEBUG
	printk("pgd_val = 0x%lx\n", pgd_val(*pgd));
	printk("pgd_index = %lu\n", pgd_index(vaddr));
#endif
	if (pgd_none(*pgd)) {
		printk("not mapped in pgd\n");
		return (pte_t *)NULL;
	}

	pud = pud_offset((p4d_t *)pgd, vaddr);
#ifdef DEBUG
	printk("pud_val = 0x%lx\n", pud_val(*pud));
#endif
	if (pud_none(*pud)) {
		printk("not mapped in pud\n");
		return (pte_t *)NULL;
	}

	pmd = pmd_offset(pud, vaddr);
#ifdef DEBUG
	printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
	printk("pmd_index = %lu\n", pmd_index(vaddr));
#endif
	if (pmd_none(*pmd)) {
		printk("not mapped in pmd\n");
		return (pte_t *)NULL;
	}

	pte = pte_offset_kernel(pmd, vaddr);
#ifdef DEBUG
	printk("pte_val = 0x%lx\n", pte_val(*pte));
	printk("pte_index = %lu\n", pte_index(vaddr));
#endif
	if (pte_none(*pte)) {
		printk("not mapped in pte\n");
		return (pte_t *)NULL;
	}

	/* Page frame physical address mechanism | offset */
	//page_addr = pte_val(*pte) & PAGE_MASK;
	//page_offset = vaddr & ~PAGE_MASK;
	//paddr = page_addr | page_offset;
#ifdef DEBUG
	//printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
	//printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);
#endif

	return pte;
}


pte_t *get_pte5(unsigned long vaddr)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long paddr = 0;
	unsigned long page_addr = 0;
	unsigned long page_offset = 0;

#ifdef DEBUG
	printk("vaddr %ld\n", vaddr);
#endif
	pgd = pgd_offset(current->mm, vaddr);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
	printk("\t not mapped in pgd\n");
		return (pte_t *)NULL;
	}
#ifdef DEBUG
	printk("\t pgd_val = 0x%lx\n", pgd_val(*pgd));
	printk("\t pgd_index = %lu\n", pgd_index(vaddr));
#endif

	p4d = p4d_offset(pgd, vaddr);
	if (p4d_none(*p4d) || p4d_bad(*p4d)) {
	  printk("\t not mapped in p4d\n");
	  return (pte_t *)NULL;
	}

	pud = pud_offset(p4d, vaddr);
#ifdef DEBUG
	printk("\t pud_val = 0x%lx\n", pud_val(*pud));
#endif
	if (pud_none(*pud)) {
	printk("\t not mapped in pud\n");
		return (pte_t *)NULL;
	}

	pmd = pmd_offset(pud, vaddr);
#ifdef DEBUG
	printk("\t pmd_val = 0x%lx\n", pmd_val(*pmd));
	printk("\t pmd_index = %lu\n", pmd_index(vaddr));
#endif
	if (pmd_none(*pmd)) {
	printk("\t not mapped in pmd\n");
		return (pte_t *)NULL;
	}

	pte = pte_offset_kernel(pmd, vaddr);
#ifdef DEBUG
	printk("\t pte_val = 0x%lx\n", pte_val(*pte));
	printk("\t pte_index = %lu\n", pte_index(vaddr));
#endif
	if (pte_none(*pte)) {
	printk("\t not mapped in pte\n");
	//return -1;
	//pte = pte_offset_map(pmd, vaddr);
	pte = pte_offset_kernel(pmd, vaddr);
#ifdef DEBUG
	printk("\t pte_val map = 0x%lx\n", pte_val(*pte));
	printk("\t pte_index map = %lu\n", pte_index(vaddr));
#endif
	if (pte_none(*pte)) {
	  printk("\t not mapped map in pte\n");
		  return (pte_t *)NULL;
	}
	}

	/* Page frame physical address mechanism | offset */
	//page_addr = pte_val(*pte) & PAGE_MASK;
	//page_offset = vaddr & ~PAGE_MASK;
	//paddr = page_addr | page_offset;
#ifdef DEBUG
	//printk("\t page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
	//printk("\t vaddr = %lx, paddr = %lx\n", vaddr, paddr);
#endif

	return pte;
}
#endif
