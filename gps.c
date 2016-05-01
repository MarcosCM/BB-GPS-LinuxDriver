/**
 * @file   gps.c
 * @author Marcos Canales Mayo
 * @date   1 May 2016
 * @version 1.0
 */

#include <asm/uaccess.h>	// User and kernel space interaction
#include <linux/cdev.h>	// cdev functions
#include <linux/device.h>	// class functions
#include <linux/fs.h>	// File system, includes fcntl.h
// Function and variable declaration macros
// These macros are useful if the driver is loaded at boot time (i.e. it is a built-in driver)
#include <linux/init.h>	// init and exit macros
#include <linux/kern_levels.h>	// printk macros
#include <linux/module.h>	// Needed by all modules
#include <linux/moduleparam.h>	// Module params macros
#include <linux/slab.h>	// kmalloc & kfree

#define DRIVER_LICENSE	"GPL" // Open-source code under GPL license
#define DRIVER_AUTHOR	"Marcos Canales Mayo <marketes94@gmail.com>"
#define DRIVER_DESCRIPTION	"Fastrax IT500 GPS Linux Driver"
#define DRIVER_SUPPORTED_DEVICES	"gps" // /dev/gps
#define DRIVER_VERSION	"1.0"

#define DEVICE_NAME	"gps"
#define NUMBER_OF_MINORS	1

// Device registration data
static struct class* gps_class = NULL;

// UART device data
static struct file* uart_dev_filp = NULL;
static char* uart_dev __initdata = "/dev/ttyO4";
module_param(uart_dev, charp, 0);
MODULE_PARM_DESC(uart_dev, "Path to the GPS UART device");
static dev_t gps_devn = NULL;

// Driver variables
static char gps_buf[256];
struct gps_cdev{
	struct cdev cdev; // Needed char device struct
};
static struct gps_cdev* gps_devp;

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
	printk(KERN_INFO "GPS device opened");
	return 0;
}

static int gps_release(struct inode* inodep, struct file* filep){
	printk(KERN_INFO "GPS device closed");
	return 0;
}

static ssize_t gps_read(struct file* filep, char __user *buf, size_t len, loff_t* offp){
	vfs_read(uart_dev_filp, gps_buf, len, offp);
	return copy_to_user(buf, gps_buf, len);
}

static ssize_t gps_write(struct file* filep, const char __user *buf, size_t len, loff_t* offp){
	return 0;
}

static int __init gps_init(void)
{
	int result;

	// *dev_t, baseminor, number_of_minors, name
	result = alloc_chrdev_region(&gps_devn, 0, NUMBER_OF_MINORS, DEVICE_NAME);
	// Get major number for the device
	if (result<0){
    	printk(KERN_ALERT "Could not register a major number\n");
    	return result;
	}

	// Register the class
	gps_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(gps_class)){
		printk(KERN_ALERT "Could not register the device class\n");
		goto UNDO_MAJOR_NUMBER;
	}

	// GFP_KERNEL = Normal and blockable allocation. This means that it will sleep until
	// the kernel can get the requested memory.
	gps_devp = kmalloc(sizeof(struct gps_cdev), GFP_KERNEL);
	// cdev struct with kobject
	cdev_init(&gps_devp->cdev, &fileops);
	gps_devp->cdev.owner = THIS_MODULE;
	gps_devp->cdev.ops = &fileops;
	// *cdev, dev_t, number_of_minors
	result = cdev_add(&gps_devp->cdev, gps_devn, NUMBER_OF_MINORS);
	if (result<0){
		printk(KERN_ALERT "Could not add cdev to the system\n");
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
	UNDO_DEVICE:
		cdev_del(&gps_devp->cdev);
	UNDO_CLASS:
		class_unregister(gps_class);
		class_destroy(gps_class);
	UNDO_MAJOR_NUMBER:
		unregister_chrdev(MAJOR(gps_devn), DEVICE_NAME);
	return -1;
}

static void __exit gps_exit(void)
{
	// Undo everything
	filp_close(uart_dev_filp, NULL);
	cdev_del(&gps_devp->cdev);
	kfree(gps_devp);
	class_unregister(gps_class);
	class_destroy(gps_class);
	unregister_chrdev_region(gps_devn, NUMBER_OF_MINORS);
	printk(KERN_INFO "Disabled GPS module\n");
}

module_init(gps_init);
module_exit(gps_exit);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_VERSION(DRIVER_VERSION);
MODULE_SUPPORTED_DEVICE(DRIVER_SUPPORTED_DEVICES);