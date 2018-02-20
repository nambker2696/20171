#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#define  DEVICE_NAME "simple_module"    // The device will appear at /dev/simple_module using this value
#define  CLASS_NAME  "sp_module"        // The device class -- this is a character device driver

MODULE_LICENSE("GPL");            // The license type -- this affects available functionality
MODULE_AUTHOR("Fan Marvel");    // The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for A Subject Programming System");  // The description -- see modinfo
MODULE_VERSION("4.0");            // A version number to inform users

static int    majorNumber;                  // Stores the device number -- determined automatically
static char   message[256] = {0};           // Memory for the string that is passed from userspace
static char   tmp[256];
static short  size_of_message;              // Used to remember the size of the string stored
static int    numberOpens = 0;              // Counts the number of times the device is opened
static struct class*  simple_moduleClass  = NULL; // The device-driver class struct pointer
static struct device* simple_moduleDevice = NULL; // The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static int     device_open(struct inode *, struct file *);
static int     device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = device_open,
   .read = device_read,
   .write = device_write,
   .release = device_release,
};

static int __init simple_module_init(void){
   printk(KERN_INFO "simple_module: Initializing the simple_module LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "simple_module failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "simple_module: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   simple_moduleClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(simple_moduleClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(simple_moduleClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "simple_module: device class registered correctly\n");

   // Register the device driver
   simple_moduleDevice = device_create(simple_moduleClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(simple_moduleDevice)){               // Clean up if there is an error
      class_destroy(simple_moduleClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(simple_moduleDevice);
   }
   printk(KERN_INFO "simple_module: device class created correctly\n"); // Made it! device was initialized
   return 0;
}


static void __exit simple_module_exit(void){
   device_destroy(simple_moduleClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(simple_moduleClass);                          // unregister the device class
   class_destroy(simple_moduleClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);                         // unregister the major number
   printk(KERN_INFO "simple_module: Goodbye from the LKM!\n");
}

static int device_open(struct inode *inodep, struct file *filep){
   tmp[0] = '\0';
   message[0] = '\0';
   numberOpens++;
   printk(KERN_INFO "simple_module: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

static ssize_t device_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   error_count = copy_to_user(buffer, message, size_of_message); // copy_to_user has the format ( * to, *from, size) and returns 0 on success

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "simple_module: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "simple_module: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   printk(KERN_INFO "message: %s\n", message);
   sprintf(tmp, "%s", message);
   sprintf(message, "%s%s", tmp, buffer);

   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "simple_module: Received %zu characters from the user\n", len);
   return len;
}


static int device_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "simple_module: Device successfully closed\n");
   return 0;
}


module_init(simple_module_init);
module_exit(simple_module_exit);
