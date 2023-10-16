/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>   /* O_ACCMODE */
#include <linux/uaccess.h> /* copy_from/to_user */
#include <asm/uaccess.h>
MODULE_LICENSE("Dual BSD/GPL");
#define B_SIZE 4
/* Declaration of memory.c functions */
int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct inode *inode, struct file *filp);
ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
void memory_exit(void);
int memory_init(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations memory_fops = {
    read : memory_read,
    write : memory_write,
    open : memory_open,
    release : memory_release
};

/* Declaration of the init and exit functions */
module_init(memory_init);
module_exit(memory_exit);

/* Global variables of the driver */
/* Major number */
int memory_major = 60;
/* Buffer to store data */
char*memory_buffer;

void memory_exit(void)
{

    /* Freeing the major number */
    unregister_chrdev(memory_major, "memory");

    /* Freeing buffer memory */
    if (memory_buffer)
    {
        kfree(memory_buffer);
    }

    printk("<1>Removing memory module\n");
}

int memory_init(void)
{
    int result;
    /* Registering device */
    result = register_chrdev(memory_major, "memory", &memory_fops);
    if (result < 0)
    {
        printk(
            "<1>memory: cannot obtain major number %d\n", memory_major);
        return result;
    }

    /* Allocating memory for the buffer */
   
    memory_buffer = (char*)kmalloc(B_SIZE * sizeof(char), GFP_KERNEL);

    if (!memory_buffer)
    {
        result = -ENOMEM;
        goto fail;
    }

    /*Set buffer to 0 by default */
    memset(memory_buffer, 0, B_SIZE * sizeof(char));
    printk("<1>Inserting memory module\n");
    return 0;
fail:
    memory_exit();
    return result;
}

int memory_open(struct inode *inode, struct file *filp)
{
    /* Success */
    return 0;
}

int memory_release(struct inode *inode, struct file *filp)
{
    /* Success */
    return 0;
}

ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    char bytes_to_copy;
    
    if (*f_pos >= B_SIZE * sizeof(char))
        return 0;  

    // Determine how many bytes to copy
    
    bytes_to_copy = min(count, B_SIZE * sizeof(char) - *f_pos);

    
    if (copy_to_user(buf, memory_buffer + *f_pos / sizeof(char), bytes_to_copy) != 0)
        return -EFAULT;  

    // Update file position
    *f_pos += bytes_to_copy;

    return bytes_to_copy;
}

ssize_t memory_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    char bytes_to_copy;

    if (*f_pos >= B_SIZE * sizeof(char))
        return 0;  
    
    bytes_to_copy = min(count, B_SIZE * sizeof(char) - *f_pos);

    if (copy_from_user(memory_buffer + *f_pos / sizeof(char), buf, bytes_to_copy) != 0)
        return -EFAULT;  

    *f_pos += bytes_to_copy;

    return bytes_to_copy;
}
