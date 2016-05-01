/**
 * @file   gps.c
 * @author Marcos Canales Mayo
 * @date   1 May 2016
 * @version 1.0
 */

#include <linux/device.h>	// Kernel driver
#include <linux/fs.h>	// File system, includes fcntl.h
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
#define DRIVER_VERSION	"1.0"

#define DEVICE_NAME	"gps"

// Device registration data
static int gps_major_number;
static struct class* gps_class = NULL;
static struct device* gps_device = NULL;

// UART device data
static struct file* uart_dev_filp = NULL;
static char* uart_dev __initdata = "/dev/ttyO4";
module_param(uart_dev, charp, 0);
MODULE_PARM_DESC(uart_dev, "Path to the GPS UART device");

// Driver operations
static int gps_open(struct inode*, struct file*);
static int gps_release(struct inode*, struct file*);
static ssize_t gps_read(struct file*, char*, size_t, loff_t*);
static ssize_t gps_write(struct file*, const char*, size_t, loff_t*);
static struct file_operations fileops =
{
   .open = gps_open,
   .release = gps_release,
   .read = gps_read,
   .write = gps_write
};

static int gps_open(struct inode* inodep, struct file* filep){
	return 0;
}

static int gps_release(struct inode* inodep, struct file* filep){
	return 0;
}

static ssize_t gps_read(struct file* filep, char* buf, size_t len, loff_t* offp){
	return 0;
}

static ssize_t gps_write(struct file* filep, const char* buf, size_t len, loff_t* offp){
	return 0;
}

static int __init gps_init(void)
{
	// Get major number for the device
	gps_major_number = register_chrdev(0, DEVICE_NAME, &fileops);
	if (gps_major_number<0){
    	printk(KERN_ALERT "Could not register a major number\n");
    	return -1;
	}

	// Register the class
	gps_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(gps_class)){
		printk(KERN_ALERT "Could not register the device class\n");
		goto UNDO_MAJOR_NUMBER;
	}

	// Register the device
	// MKDEV turns MAJOR and MINOR into a dev_t
	gps_device = device_create(gps_class, NULL, MKDEV(gps_major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(gps_device)){
		printk(KERN_ALERT "Could not create the device\n");
		goto UNDO_CLASS;
	}

	// Open the UART device
	uart_dev_filp = filp_open(uart_dev, O_RDWR, 0);
	if (uart_dev_filp == NULL){
		printk(KERN_ALERT "Could not open the UART device\n");
		goto UNDO_DEVICE;
	}

	// Success. Returning 0 means that init_module succeeded.
	printk(KERN_INFO "Enabled GPS module\n");
	return 0;

	// Error. Returning 1 means that init_module failed.
	UNDO:
	UNDO_DEVICE:
		device_destroy(gps_class, MKDEV(gps_major_number, 0));
	UNDO_CLASS:
		class_unregister(gps_class);
		class_destroy(gps_class);
	UNDO_MAJOR_NUMBER:
		unregister_chrdev(gps_major_number, DEVICE_NAME);
	return -1;
}

static void __exit gps_exit(void)
{
	// Undo everything
	filp_close(uart_dev_filp, NULL);
	device_destroy(gps_class, MKDEV(gps_major_number, 0));
	class_unregister(gps_class);
	class_destroy(gps_class);
	unregister_chrdev(gps_major_number, DEVICE_NAME);
	printk(KERN_INFO "Disabled GPS module\n");
}

module_init(gps_init);
module_exit(gps_exit);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_VERSION(DRIVER_VERSION);
MODULE_SUPPORTED_DEVICE(DRIVER_SUPPORTED_DEVICES);