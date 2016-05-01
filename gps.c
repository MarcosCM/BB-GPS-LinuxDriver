// Function and variable declaration macros
// These macros are useful if the driver is loaded at boot time (i.e. it is a built-in driver)
#include <linux/init.h>
#include <linux/kern_levels.h>	// printk macros
#include <linux/module.h>	// Needed by all modules
#include <linux/moduleparam.h>	// Module params macros

#define DRIVER_LICENSE	"GPL" // Open-source code under GPL license
#define DRIVER_AUTHOR	"Marcos Canales Mayo <marketes94@gmail.com>"
#define DRIVER_DESCRIPTION	"Fastrax IT500 GPS Linux Driver"
#define DRIVER_SUPPORTED_DEVICES	"gps" // /dev/gps

static char* uart_dev __initdata = "/dev/ttyO4";
module_param(uart_dev, charp, 0);
MODULE_PARM_DESC(uart_dev, "Path to the GPS UART device");

static int __init gps_init(void)
{
	printk(KERN_INFO "Hello world! uart_dev is %s\n", uart_dev);

	// 0 value means init_module succeeded. Otherwise means failed
	return 0;
}

static void __exit gps_exit(void)
{
	printk(KERN_INFO "Bye world!.\n");
}

module_init(gps_init);
module_exit(gps_exit);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_SUPPORTED_DEVICE(DRIVER_SUPPORTED_DEVICES);