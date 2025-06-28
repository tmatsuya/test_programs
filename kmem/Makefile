ifneq ($(KERNELRELEASE),)
obj-m		:= kmem.o
else
KDIR		:= /lib/modules/$(shell uname -r)/build/
PWD		:= $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	install -m 644 $(PWD)/*.ko /lib/modules/`uname -r`/kernel/drivers/misc
	if [ -d /etc/udev/rules.d -a ! -f /etc/udev/rules.d/99-kmem.rules ] ; then \
		install -m 644 99-kmem.rules /etc/udev/rules.d ; \
	fi
	depmod -a

autoload:
	if [ -d /etc/modules-load.d -a ! -f /etc/modules-load.d/kmem.conf ] ; then \
		install -m 644 kmem.conf /etc/modules-load.d ; \
	else \
		if [ -d /etc/sysconfig/modules -a ! -f /etc/sysconfig/modules/kmem.modules ] ; then \
			install -m 755 kmem.modules /etc/sysconfig/modules ; \
		fi \
	fi
endif
