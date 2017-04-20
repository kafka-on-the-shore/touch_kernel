#include <linux/module.h>

static int __init panic_init_module(void) {
	panic("panic happening, pandora");
	return 0;
}

module_init(panic_init_module);
MODULE_LICENSE("GPL");

