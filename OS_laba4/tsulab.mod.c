#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

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



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xdf4bee3d, "alloc_workqueue_noprof" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xc01aafd2, "get_random_u32" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0x2d88a3ab, "cancel_work_sync" },
	{ 0xbeb1d261, "destroy_workqueue" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x67628f51, "msleep" },
	{ 0xe9196a28, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd272d446,
	0xdf4bee3d,
	0x90a48d82,
	0xc01aafd2,
	0x49733ad6,
	0x2d88a3ab,
	0xbeb1d261,
	0xd272d446,
	0xe8213e80,
	0x67628f51,
	0xe9196a28,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__x86_return_thunk\0"
	"alloc_workqueue_noprof\0"
	"__ubsan_handle_out_of_bounds\0"
	"get_random_u32\0"
	"queue_work_on\0"
	"cancel_work_sync\0"
	"destroy_workqueue\0"
	"__fentry__\0"
	"_printk\0"
	"msleep\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "836E95533D191792E47BB85");
