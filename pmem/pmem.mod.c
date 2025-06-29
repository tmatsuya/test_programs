#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x14\x00\x00\x00\x35\xe4\x57\x2d"
	"pcpu_hot\0\0\0\0"
	"\x14\x00\x00\x00\x83\x9d\xd7\x72"
	"pgdir_shift\0"
	"\x18\x00\x00\x00\x4a\xab\x6c\xe1"
	"boot_cpu_data\0\0\0"
	"\x10\x00\x00\x00\x95\x68\x6d\xcf"
	"pv_ops\0\0"
	"\x14\x00\x00\x00\xe6\x10\xec\xd4"
	"BUG_func\0\0\0\0"
	"\x18\x00\x00\x00\x7b\xf7\x19\x1d"
	"physical_mask\0\0\0"
	"\x1c\x00\x00\x00\x5e\xd7\xd8\x7c"
	"page_offset_base\0\0\0\0"
	"\x2c\x00\x00\x00\x61\xe5\x48\xa6"
	"__ubsan_handle_shift_out_of_bounds\0\0"
	"\x1c\x00\x00\x00\x48\x9f\xdb\x88"
	"__check_object_size\0"
	"\x18\x00\x00\x00\xc2\x9c\xc4\x13"
	"_copy_from_user\0"
	"\x18\x00\x00\x00\xe1\xbe\x10\x6b"
	"_copy_to_user\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x18\x00\x00\x00\xf3\x36\x8c\x56"
	"misc_register\0\0\0"
	"\x18\x00\x00\x00\x36\xcf\x44\x37"
	"vmalloc_to_pfn\0\0"
	"\x18\x00\x00\x00\x6d\x19\xef\x0d"
	"remap_pfn_range\0"
	"\x18\x00\x00\x00\x2d\xc9\x9a\xef"
	"misc_deregister\0"
	"\x18\x00\x00\x00\x76\xf2\x0f\x5e"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "EF3BDCD73D8DCCA1618F587");
