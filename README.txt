Copyright 2014-2025 Takeshi Matsuya


[Memory Tools]

*CAUTION: These tools can cause your system to fail if used incorrectly.

 <kmem>
   description: Kernel Memory Access Linux Driver (x86-64 only)

   how to build: 
     cd kmem
     ./mk
     ls -l /dev/kmem

   how to use
     sudo lv /proc/kallsyms 
     memory /dev/kmem 0x???????


 <pmem>
   description: Physical Memory Access Linux Driver (x86-64 only)

   how to build: 
     cd pmem
     ./mk
     ls -l /dev/pmem

   how to use
     lspci -v
     memory /dev/pmem 0xfc800000    # (Display and modify PCI BAR register)


 <memory>
   description: Memory Editor

   how to build: 
     cd memory
     sudo apt-get install libncurses5-dev
     make
     sudo make install

   how to use: 
     sudo memory /dev/mem 0x0000



[Other test programs]

 <bypass_ipstack>
 <crc32>
 <cuda_iplookup>
 <ip_fast_checksum>
 <ipsum>
 <membench>
 <mmap>
 <mtrr>
 <pci_enabler_module>
 <player575>
 <rdtsc>
