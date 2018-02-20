#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/cdev.h>

static int init_char_device(void);
static void exit_char_device(void);

module_init(init_char_device);
module_exit(exit_char_device);


static int char_device_open(struct inode *inode, struct file  *file);
static int char_device_release(struct inode *inode, struct file *file);
static int char_device_read(struct file *file, char *buf, size_t lbuf,
    loff_t *ppos);
static int char_device_write(struct file *file,const char *buf,size_t lbuf,
  loff_t *ppos);
static loff_t char_device_lseek(struct file *file,loff_t offset,int orig);


/* file operations structure */
static struct file_operations char_device_file_ops;

/* Variable Declaration */

static int char_device_id;
#define CHAR_DEVICE_NAME "llk_device"
#define MAX_LENGTH 4000
static char char_device_buf[MAX_LENGTH];
struct cdev *llk_cdev;
dev_t devicenums;

/**
 * Driver initialization routine.It shows how to register a char device
 * Look it closely, important routine to understand a char driver.
 * */
static int init_char_device(void)
{
    int i,ret;
    /* Initialize the file operations structure object as we did earlier */

    char_device_file_ops.owner = THIS_MODULE;
    char_device_file_ops.read = char_device_read;
    char_device_file_ops.write = char_device_write;
    char_device_file_ops.open = char_device_open;
    char_device_file_ops.release = char_device_release;
    char_device_file_ops.llseek = char_device_lseek;

    /* Acquire the major and minor numbers for your driver module */

    /* We are passing 0 in the second argument and passing 1 in the */
    /* third argument. That means we want to request only one minor number for */  /* this major number and so that minor number would be 0 */

    ret=alloc_chrdev_region(&devicenums,0,1,"llk_device");
    char_device_id= MAJOR(mydev);//Get the major no
    llk_cdev= cdev_alloc(); // Get an allocated cdev structure
    /*Register the character Device*/

    veda_cdev->owner=THIS_MODULE;
    veda_cdev->ops= &char_device_file_ops;

     /* Add this to the kernel */

    ret=cdev_add(llk_cdev,devicenums,1);
    if( ret < 0 )
    {
        printk("Error registering device driver with MAJOR NO [%d]\n", char_device_id);
        return ret;
    }
    printk("Device successfully with MAJOR NO[%d]\n",char_device_id);
    /* Now clear the device buffer */

    char_device_buf = { 0 } ;
    return 0;
}

/**
* Module Cleanup. Simply returns back the major,minor number and the cdev   object resources back.
 * */
static void exit_char_device(void)
{
    printk("\n Module removed");
    unregister_chrdev_region(devicenums,1);
    cdev_del(llk_cdev);
}

#define SUCCESS 0

/* Implementing open/read/write/lseek/close functions as we wrote earlier*/
/**
 * This function is called when a user wants to use this device
 * and has called the open function.
 *
 * The function will keep a count of how many people
 * tried to open it and increments it each time
 * this function is called
 *
 * The function prints out two pieces of information
 * 1. Number of times open() was called on this device
 * 2. Number of processes accessing this device right now
 *
 * Return value
 *          Always returns SUCCESS
 * */
static int char_device_open(struct inode *inode, struct file  *file)
{
    static int counter = 0;
    counter++;
    printk("Number of times open() was called: %d\n", counter);
    printk("Process id of the current process: %d\n", current->pid );
    return SUCCESS;
}

/**
 * This function is called when the user program uses close() function
 *
 * The function decrements the number of processes currently
 * using this device. This should be done because if there are no
 * users of a driver for a long time, the kernel will unload
 * the driver from the memory.
 *
 * Return value
 *          Always returns SUCCESS
 * */

static int char_device_release(struct inode *inode, struct file *file)
{
    return SUCCESS;
}

/**
 * This function is called when the user calls read on this device
 * It reads from a 'file' some data into 'buf' which
 * is 'lbuf' long starting from 'ppos' (present position)
 * Parameters
 *          buf  = buffer
 *          ppos = present position
 *          lbuf = length of the buffer
 *          file = file to read
 * The function returns the number of bytes(characters) read.
 * */
static int char_device_read(struct file *file, char *buf,size_t lbuf,
    loff_t *ppos)
{
    int maxbytes; /* number of bytes from ppos to MAX_LENGTH */
    int bytes_to_do; /* number of bytes to read */
    int nbytes; /* number of bytes actually read */

    maxbytes = MAX_LENGTH - *ppos;

    if( maxbytes > lbuf ) bytes_to_do = lbuf;
    else bytes_to_do = maxbytes;

    if( bytes_to_do == 0 )
    {
        printk("Reached end of device\n");
                return -ENOSPC; /* Causes read() to return EOF */
    }

    nbytes = bytes_to_do -
                 copy_to_user( buf, /* to */
                                   char_device_buf + *ppos, /* from */
                                   bytes_to_do ); /* how many bytes */
    *ppos += nbytes;
    return nbytes;   
}

/**
 * This function is called when the user calls write on this device
 * It writes into 'file' the contents of 'buf' starting from 'ppos'
 * up to 'lbuf' bytes.
 * Understanding the parameters
 *          buf  = buffer
 *          file = file to write into
 *          lbuf = length of the buffer
 *          ppos = present position pointer
 * The function returs the number of characters(bytes) written
 * */

static int char_device_write(struct file *file,const char *buf,
   size_t lbuf,loff_t *ppos)
{
    int nbytes; /* Number of bytes written */
    int bytes_to_do; /* Number of bytes to write */
    int maxbytes; /* Maximum number of bytes that can be written */

    maxbytes = MAX_LENGTH - *ppos;

    if( maxbytes > lbuf ) bytes_to_do = lbuf;
    else bytes_to_do = maxbytes;

    if( bytes_to_do == 0 )
    {
        printk("Reached end of device\n");
                return -ENOSPC; /* Returns EOF at write() */
    }

    nbytes = bytes_to_do -
             copy_from_user( char_device_buf + *ppos, /* to */
                                         buf, /* from */
                                         bytes_to_do ); /* how many bytes */
    *ppos += nbytes;
    return nbytes;
}

/**
 * This function is called when lseek() is called on the device
 * The function should place the ppos pointer of 'file'
 * at an offset of 'offset' from 'orig'
 *
 * if orig = SEEK_SET
 *          ppos = offset
 *
 * if orig = SEEK_END
 *          ppos = MAX_LENGTH - offset
 *
 * if orig = SEEK_CUR
 *          ppos += offset
 *
 * returns the new position
 * */
static loff_t char_device_lseek(struct file *file,loff_t offset,
  int orig)
{
    loff_t new_pos=0;
    switch( orig )
    {
    case 0: /* SEEK_SET: */
        new_pos = offset;
        break;
    case 1: /* SEEK_CUR: */
        new_pos = file->f_pos + offset;
        break;
    case 2: /* SEEK_END: */
        new_pos = MAX_LENGTH - offset;
        break;
    }          
    if( new_pos > MAX_LENGTH ) new_pos = MAX_LENGTH;
    if( new_pos < 0 ) new_pos = 0;
    file->f_pos = new_pos;
    return new_pos;
}

MODULE_AUTHOR("LLK");
MODULE_DESCRIPTION("A Basic Character Device Driver");