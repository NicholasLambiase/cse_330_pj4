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



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x8f87b2a9, "class_destroy" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x122c3a7e, "_printk" },
	{ 0x1d01c156, "blkdev_put" },
	{ 0x6f28264, "cdev_add" },
	{ 0x2a9db912, "device_create" },
	{ 0x5833e735, "class_create" },
	{ 0xa000e84a, "blkdev_get_by_path" },
	{ 0x38b07c44, "param_ops_charp" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x8e8da62b, "device_destroy" },
	{ 0xa95eded2, "cdev_init" },
	{ 0x8e2f5771, "cdev_del" },
	{ 0x4c9adaff, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "754ACAB56C4398BBF087168");
