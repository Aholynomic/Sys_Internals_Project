#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
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

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xfece093d, "module_layout" },
	{ 0xe007de41, "kallsyms_lookup_name" },
	{ 0x61651be, "strcat" },
	{ 0xe914e41e, "strcpy" },
	{ 0xc358aaf8, "snprintf" },
	{ 0xc4f0da12, "ktime_get_with_offset" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xb94309bc, "filp_close" },
	{ 0xc5850110, "printk" },
	{ 0x2fb6c04b, "kernel_write" },
	{ 0x97255bdf, "strlen" },
	{ 0xac27ceef, "filp_open" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "5A65DD5941E37CDE5694946");
