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
#include <linux/kernel.h>	// container_of macro
#include <linux/kern_levels.h>	// printk macros
#include <linux/module.h>	// Needed by all modules
#include <linux/moduleparam.h>	// Module params macros
#include <linux/slab.h>	// kmalloc & kfrees

#define DRIVER_LICENSE	"GPL" // Open-source code under GPL license
#define DRIVER_AUTHOR	"Marcos Canales Mayo <marketes94@gmail.com>"
#define DRIVER_DESCRIPTION	"Fastrax IT500 GPS Linux Driver"
#define DRIVER_SUPPORTED_DEVICES	"gps" // /dev/gps
#define DRIVER_VERSION	"1.0"

#define DEVICE_NAME	"gps"	// /dev/gps
#define DEVICE_PATH	"/dev/gps"
#define NUMBER_OF_MINORS	1

// Device registration data
static struct class* gps_class;

// UART device data
//static struct file* uart_dev_filp = NULL;
static char* uart_dev = "/dev/ttyO4\0";
module_param(uart_dev, charp, 0);
MODULE_PARM_DESC(uart_dev, "Path to the GPS UART device");

// Driver variables
struct gps_cdev{
	struct cdev cdev; // Needed char device struct
};
static struct gps_cdev* gps_cdevp;
static dev_t gps_devn;

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
	struct gps_cdev* gps_cdev;

	gps_cdev = container_of(inodep->i_cdev, struct gps_cdev, cdev);
	filep->private_data = gps_cdev;

	printk(KERN_INFO "GPS device opened\n");
	return 0;
}

static int gps_release(struct inode* inodep, struct file* filep){
	printk(KERN_INFO "GPS device closed\n");
	return 0;
}

static ssize_t gps_read(struct file* filep, char __user *buf, size_t len, loff_t* offp){
	int res;
	char gps_buf[8]= "testasd\0";
	printk(KERN_INFO "Reading from GPS\n");
	//vfs_read(uart_dev_filp, gps_buf, len, offp);
	printk(KERN_INFO "gps_buf is %s; len is %d\n", gps_buf, len);
	res = copy_to_user(buf, gps_buf, len);
	printk(KERN_INFO "gps Could not copy %d bytes\n", res);
	return res;
}

static ssize_t gps_write(struct file* filep, const char __user *buf, size_t len, loff_t* offp){
	printk(KERN_INFO "Writing to GPS\n");
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

	/*result = mknod(DEVICE_PATH, S_IFCHR | 0666, gps_devn);
	if (result<0){
		printk(KERN_ALERT "Could not create fs node\n");
		goto UNDO_MAJOR_NUMBER;
	}*/

	// Register the class
	gps_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(gps_class)){
		printk(KERN_ALERT "Could not register the device class\n");
		goto UNDO_NOD;
	}

	// GFP_KERNEL = Normal and blockable allocation. This means that it will sleep until
	// the kernel can get the requested memory.
	gps_cdevp = kmalloc(sizeof(struct gps_cdev), GFP_KERNEL);
	// cdev struct with kobject
	cdev_init(&gps_cdevp->cdev, &fileops);
	gps_cdevp->cdev.owner = THIS_MODULE;
	// *cdev, dev_t, number_of_minors
	result = cdev_add(&gps_cdevp->cdev, gps_devn, NUMBER_OF_MINORS);
	if (result<0){
		printk(KERN_ALERT "Could not add cdev to the system\n");
		goto UNDO_CLASS;
	}

	// Open the UART device
	/*uart_dev_filp = filp_open(uart_dev, O_RDWR, 0);
	printk(KERN_INFO "GPS uart_dev_filp is %d\n", uart_dev_filp);
	if (uart_dev_filp == NULL || uart_dev_filp<0){
		printk(KERN_ALERT "Could not open the UART device\n");
		printk(KERN_ALERT "Error is %d", PTR_ERR(uart_dev_filp));
		goto UNDO_DEVICE;
	}*/

	// Success. Returning 0 means that init_module succeeded.
	printk(KERN_INFO "Enabled GPS module\n");
	return 0;

	// Error. Returning -1 means that init_module failed.
	UNDO_DEVICE:
		cdev_del(&gps_cdevp->cdev);
	UNDO_CLASS:
		class_unregister(gps_class);
		class_destroy(gps_class);
	UNDO_NOD:
		// Should remove /dev/gps?
	UNDO_MAJOR_NUMBER:
		unregister_chrdev_region(gps_devn, NUMBER_OF_MINORS);
	return -1;
}

static void __exit gps_exit(void)
{
	// Undo everything
	//filp_close(uart_dev_filp, NULL);
	cdev_del(&gps_cdevp->cdev);
	kfree(gps_cdevp);
	// Should destroy gps_class?
	class_unregister(gps_class);
	class_destroy(gps_class);
	// Should remove /dev/gps?
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