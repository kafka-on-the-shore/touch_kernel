#include <linux/module.h>

static int __init panic_init_module(void)
{
	panic("panic happening, pandora");
	return 0;
}
static void __exit panic_exit_module(void)
{
    printk(KERN_ERR "Never touch here\n");
}

module_init(panic_init_module);
module_exit(panic_exit_module);
MODULE_LICENSE("GPL");

