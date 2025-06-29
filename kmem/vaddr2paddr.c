static unsigned long vaddr2paddr_l4(unsigned long vaddr)
{
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long paddr = 0;
    unsigned long page_addr = 0;
    unsigned long page_offset = 0;

    pgd = pgd_offset(current->mm, vaddr);
    printk("pgd_val = 0x%lx\n", pgd_val(*pgd));
    printk("pgd_index = %lu\n", pgd_index(vaddr));
    if (pgd_none(*pgd)) {
        printk("not mapped in pgd\n");
        return -1;
    }

    pud = pud_offset((p4d_t *)pgd, vaddr);
    printk("pud_val = 0x%lx\n", pud_val(*pud));
    if (pud_none(*pud)) {
        printk("not mapped in pud\n");
        return -1;
    }

    pmd = pmd_offset(pud, vaddr);
    printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
    printk("pmd_index = %lu\n", pmd_index(vaddr));
    if (pmd_none(*pmd)) {
        printk("not mapped in pmd\n");
        return -1;
    }

    pte = pte_offset_kernel(pmd, vaddr);
    printk("pte_val = 0x%lx\n", pte_val(*pte));
    printk("pte_index = %lu\n", pte_index(vaddr));
    if (pte_none(*pte)) {
        printk("not mapped in pte\n");
        return -1;
    }

    /* Page frame physical address mechanism | offset */
    page_addr = pte_val(*pte) & PAGE_MASK;
    page_offset = vaddr & ~PAGE_MASK;
    paddr = page_addr | page_offset;
    printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
        printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);

    return paddr;
}


static unsigned long vaddr2paddr_l5(unsigned long vaddr)
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
	return -1;
    }
#ifdef DEBUG
    printk("\t pgd_val = 0x%lx\n", pgd_val(*pgd));
    printk("\t pgd_index = %lu\n", pgd_index(vaddr));
#endif

    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
      printk("\t not mapped in p4d\n");
      return -1;
    }

    pud = pud_offset(p4d, vaddr);
#ifdef DEBUG
    printk("\t pud_val = 0x%lx\n", pud_val(*pud));
#endif
    if (pud_none(*pud)) {
	printk("\t not mapped in pud\n");
	return -1;
    }

    pmd = pmd_offset(pud, vaddr);
#ifdef DEBUG
    printk("\t pmd_val = 0x%lx\n", pmd_val(*pmd));
    printk("\t pmd_index = %lu\n", pmd_index(vaddr));
#endif
    if (pmd_none(*pmd)) {
	printk("\t not mapped in pmd\n");
	return -1;
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
	  return -1;
	}
    }

    /* Page frame physical address mechanism | offset */
    page_addr = pte_val(*pte) & PAGE_MASK;
    page_offset = vaddr & ~PAGE_MASK;
    paddr = page_addr | page_offset;
#ifdef DEBUG
    printk("\t page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
    printk("\t vaddr = %lx, paddr = %lx\n", vaddr, paddr);
#endif

    return paddr;
}
