#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/dcache.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/cdev.h>
#include <linux/nospec.h>

#include <linux/bio.h>
#include <linux/blkdev.h>

#include "../ioctl-defines.h"

/* Device-related definitions */
static dev_t                dev = 0;
static struct class*        kmod_class;
static struct cdev          kmod_cdev;

/* Main file externs */
extern struct block_device*        bdevice;
extern struct bio*                 bdevice_bio;

/* Buffers for different operation requests */
struct block_rw_ops         rw_request;
struct block_rwoffset_ops   rwoffset_request;

/* Struct data variables */
unsigned int req_size = -1;
char* kern_buffer;

/* Sector Calculation variables */
const int SECTOR_BYTES = 4096;
int current_offset = 0;
int current_sector = 0;

/* Debuging Variables */
int iteration_count = 1;


static long kmod_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    switch (cmd)
    {
        case BREAD:

            if(copy_from_user((void*)&rw_request, (void*)arg, sizeof(struct block_rw_ops))){
                printk("Error: User didn't send right message.\n");
                return -1;
            }

            // Debugging to see what we received from the IOCTL
            printk("Command number %d: 'BREAD' called\tSize: %u\n", iteration_count, rw_request.size);
            iteration_count++;

            // Copy the size of the data we received
            req_size = rw_request.size;

            // Allocate Kernel Buffer with rw_request.size
            kern_buffer = (char*) vmalloc(req_size);

            current_offset = 0;

            for (int sector = 0; sector < (req_size / 512); sector++)
            {
                // Debug for sectors and offsets
                printk("READING Sector: %d\tKern_Buffer Offset: %d\n", current_sector, current_offset);

                // Make sure the bio is associated with the device
                bdevice_bio = bio_alloc(bdevice, 1, REQ_OP_READ, GFP_NOIO);

                // Aquire the BIO
                bio_get(bdevice_bio);
                
                // Set the new sector number
                bdevice_bio->bi_iter.bi_sector = current_sector;

                // Set the bio to the correct operation
                bdevice_bio->bi_opf = REQ_OP_READ;

                // Add kernel buffer page to the BIO with its correct offset
                bio_add_page(bdevice_bio, vmalloc_to_page((void*) kern_buffer), 512, current_offset);

                // Submit the request and wait for the operation to complete
                submit_bio_wait(bdevice_bio);
                
                // Return the BIO
                bio_put(bdevice_bio);

                // current_offset += 512;
                current_sector++;
            }

            // Copy the data that we now have stored in the kern_buffer back to the user
            copy_to_user(rw_request.data, kern_buffer, rw_request.size);

            // Debug formatting
            printk("\n");

            return 0;
        case BWRITE:

            if(copy_from_user((void*)&rw_request, (void*)arg, sizeof(struct block_rw_ops))){
                printk("Error: User didn't send right message.\n");
                return -1;
            }

            // Debugging to see what we received from the IOCTL
            printk("Command number %d: 'BWRITE' called\tSize: %u\n", iteration_count, rw_request.size);
            iteration_count++;

            // Copy the size of the data we received
            req_size = rw_request.size;

            // Put the data into kern_buffer
            kern_buffer = (char*) vmalloc(req_size);
            copy_from_user(kern_buffer, rw_request.data, req_size);

            current_offset = 0;

            for (int sector = 0; sector < (req_size / 512); sector++)
            {
                // Debug for sectors and offsets
                printk("WRITING Sector: %d\tOffset: %d\n", current_sector, current_offset);

                // Make sure the bio is associated with the device
                bdevice_bio = bio_alloc(bdevice, 1, REQ_OP_WRITE, GFP_NOIO);

                // Aquire the BIO
                bio_get(bdevice_bio);
                
                // Set the new sector number
                bdevice_bio->bi_iter.bi_sector = current_sector;

                // Set the bio to the correct operation
                bdevice_bio->bi_opf = REQ_OP_WRITE;

                // Add kernel buffer page to the BIO with its correct offset
                bio_add_page(bdevice_bio, vmalloc_to_page((void*) kern_buffer), 512, current_offset);

                // Submit the request and wait for the operation to complete
                submit_bio_wait(bdevice_bio);
                
                // Return the BIO
                bio_put(bdevice_bio);

                current_offset += 512;
                current_sector++;
            }
                
            return 0;
        case BREADOFFSET:
            
            printk("Command: 'BREADOFFSET' called\n");

            if(copy_from_user((void*) &rwoffset_request, (void*)arg, sizeof(struct block_rwoffset_ops))){
                printk("Error: User didn't send right message.\n");
                return -1;
            }

            return 0;
        case BWRITEOFFSET:

            printk("Command: 'BWRITEOFFSET' called\n");

            /* Get request from user */
            /* Allocate a kernel buffer to read/write user data */
            /* Perform the block operation */

            if(copy_from_user((void*) &rw_request, (void*)arg, sizeof(struct block_rw_ops))){
                printk("Error: User didn't send right message.\n");
                return -1;
            }

            return 0;
        default: 
            printk("Error: incorrect operation requested, returning.\n");
            return -1;
    }
    return 0;
}

static int kmod_open(struct inode* inode, struct file* file) {
    printk("Opened kmod. \n");
    return 0;
}

static int kmod_release(struct inode* inode, struct file* file) {
    printk("Closed kmod. \n");
    return 0;
}

static struct file_operations fops = 
{
    .owner          = THIS_MODULE,
    .open           = kmod_open,
    .release        = kmod_release,
    .unlocked_ioctl = kmod_ioctl,
};

/* Initialize the module for IOCTL commands */
bool kmod_ioctl_init(void) {

    /* Allocate a character device. */
    if (alloc_chrdev_region(&dev, 0, 1, "usbaccess") < 0) {
        printk("error: couldn't allocate \'usbaccess\' character device.\n");
        return false;
    }

    /* Initialize the chardev with my fops. */
    cdev_init(&kmod_cdev, &fops);
    if (cdev_add(&kmod_cdev, dev, 1) < 0) {
        printk("error: couldn't add kmod_cdev.\n");
        goto cdevfailed;
    }

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,2,16)
    if ((kmod_class = class_create(THIS_MODULE, "kmod_class")) == NULL) {
        printk("error: couldn't create kmod_class.\n");
        goto cdevfailed;
    }
#else
    if ((kmod_class = class_create("kmod_class")) == NULL) {
        printk("error: couldn't create kmod_class.\n");
        goto cdevfailed;
    }
#endif

    if ((device_create(kmod_class, NULL, dev, NULL, "kmod")) == NULL) {
        printk("error: couldn't create device.\n");
        goto classfailed;
    }

    printk("[*] IOCTL device initialization complete.\n");
    return true;

classfailed:
    class_destroy(kmod_class);
cdevfailed:
    unregister_chrdev_region(dev, 1);
    return false;
}

void kmod_ioctl_teardown(void) {
    /* Destroy the classes too (IOCTL-specific). */
    if (kmod_class) {
        device_destroy(kmod_class, dev);
        class_destroy(kmod_class);
    }
    cdev_del(&kmod_cdev);
    unregister_chrdev_region(dev,1);
    printk("[*] IOCTL device teardown complete.\n");
}
