/******************************************************************************/
/*                                                                            */
/*                        File:   ledctrl_dev.c                               */
/*                                                                            */
/******************************************************************************/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include "ledctrl_dev.h"

#define USR_DEF_RED_LED_MASK        0x04     // GPIO1_2

static int ledctrl_major = 0;
static int ledctrl_minor = 0;
static int ledctrl_dir_offset  = 0;
static int ledctrl_data_offset = 0;
static phys_addr_t  phys_addr_ledctrl;

static struct class*                 ledctrl_class = NULL;
static struct ledctrl_hw_dev*        ledctrl_dev   = NULL;

static int     ledctrl_dev_open         (struct inode *, struct file *);
static int     ledctrl_dev_release      (struct inode *, struct file *);
static ssize_t ledctrl_dev_read         (struct file *,  char __user *,       size_t,   loff_t *);
static ssize_t ledctrl_dev_write        (struct file *,  const char __user *, size_t,   loff_t *);
// static int     ledctrl_dev_proc_open    (struct inode *, struct file *);
static ssize_t ledctrl_dev_proc_read    (struct file *,  char __user *,       size_t,   loff_t *);
static ssize_t ledctrl_dev_proc_write   (struct file *,  const char __user *, size_t,   loff_t *);
static int     ledctrl_dev_setup        (struct ledctrl_hw_dev*);
static void    ledctrl_dev_create_proc  (void);
static void    ledctrl_dev_remove_proc  (void);
static int     ledctrl_dev_show_status  (struct seq_file*,  void*);
static void    bigendian_2_littleendian (void*,          ssize_t);

static struct file_operations ledctrl_fops = {
    .owner   = THIS_MODULE,
    .open    = ledctrl_dev_open,
    .release = ledctrl_dev_release,
    .read    = ledctrl_dev_read,
    .write   = ledctrl_dev_write,
};

static struct file_operations ledctrl_proc_fops = {
    .owner   = THIS_MODULE,
    .open    = ledctrl_dev_open,
    .release = ledctrl_dev_release,
    .read    = ledctrl_dev_proc_read,
    .write   = ledctrl_dev_proc_write,
};

static  void    bigendian_2_littleendian (void*  val_buf, ssize_t  val_size)
{
    ssize_t  i = 0, j = val_size - 1;
    unsigned char  tmp_val;

    while(i < j){
        tmp_val = ((unsigned char*)val_buf)[i];
        ((unsigned char*)val_buf)[i] = ((unsigned char*)val_buf)[j];
        ((unsigned char*)val_buf)[j] = tmp_val;
        i++;
        j--;
    }
}

static int ledctrl_dev_open (struct inode* inode, struct file* filp)
{
    struct ledctrl_hw_dev* dev;

    dev = container_of(inode->i_cdev, struct ledctrl_hw_dev, dev);
    filp->private_data = dev;
	filp->fpos         = 0;

    return 0;
}

static int ledctrl_dev_release (struct inode* inode, struct file* filp)
{
    return 0;
}

static ssize_t ledctrl_dev_read (struct file* filp, char __user *buf, size_t  num,  loff_t*  off)
{
    ssize_t  err = 0;
    struct ledctrl_hw_dev* dev = filp->private_data;
    unsigned int reg_value;
    unsigned char led_status;
    volatile void __iomem * reg_data_addr = ioremap((phys_addr_ledctrl + ledctrl_data_offset), 4);

    if(down_interruptible(&(dev->sem))){
        return -ERESTARTSYS;
    }

    reg_value = readl(reg_data_addr);
    if(reg_value & USR_DEF_RED_LED_MASK){
        led_status = 1;
    }
    else{
        led_status = 0;
    }

    err = copy_to_user(buf, &led_status, 1);

    if(err > 0){
        err = -EFAULT;
    }
    else{
        err = 1;
    }

    up(&(dev->sem));

    return err;
}

static ssize_t ledctrl_dev_write (struct file* filp, const char __user *buf, size_t  num, loff_t*  off)
{
    ssize_t  err = 0;
    struct ledctrl_hw_dev* dev = filp->private_data;
    unsigned char led_status;
    volatile void __iomem * reg_data_addr = ioremap((phys_addr_ledctrl + ledctrl_data_offset), 4);

    if(down_interruptible(&(dev->sem))){
        return -ERESTARTSYS;
    }

    err = copy_from_user(&led_status, buf, 1);
    if(err > 0){
        err = -EFAULT;
        goto out;
    }
    else{
        err = 1;
    }

    if(led_status){
        writel(readl(reg_data_addr) | USR_DEF_RED_LED_MASK, reg_data_addr);
    }
    else{
        writel(readl(reg_data_addr) & ~USR_DEF_RED_LED_MASK, reg_data_addr);
    }

out:
    up(&(dev->sem));

    return err;
}

static int ledctrl_dev_show_status (struct seq_file* seq,  void* offset)
{
    int    err  = 0;
    char*  page = NULL;
    struct ledctrl_hw_dev* dev = ledctrl_dev;
    volatile void __iomem * reg_data_addr = ioremap((phys_addr_ledctrl + ledctrl_data_offset), 4);

    if(down_interruptible(&(dev->sem))){
        return -ERESTARTSYS;
    }

    page = (char*)__get_free_page(GFP_KERNEL);
    if(!page){
        err = -ENOMEM;
        goto nomem_err;
    }

    if(readl(reg_data_addr) | USR_DEF_RED_LED_MASK){
        err = snprintf(page, PAGE_SIZE, "1\n");
    }
    else{
        err = snprintf(page, PAGE_SIZE, "0\n");
    }
 
    if(err < 0){
         goto copy_err;
    }

    seq_printf(seq, page);

copy_err:
    free_page((unsigned long)page);
 
nomem_err:
    up(&(dev->sem));
    return 0;
}

#if 0
static int ledctrl_dev_proc_open (struct inode* inode, struct file* filp)
{
    return single_open(filp, ledctrl_dev_show_status, NULL);
}
#endif

static ssize_t ledctrl_dev_proc_read (struct file* filp, char __user *buf, size_t  num,  loff_t*  off)
{
    int    err  = 0;
    char*  page = NULL;
    volatile void __iomem * reg_data_addr = ioremap(GPIO1_DATA_REGISTER, 4);

    printk(KERN_ALERT "ledctrl_dev_proc_read_reg: (%d)\n", *off);

	if(1 == *off){
		return 0;
	}

    if(down_interruptible(&(ledctrl_dev->sem))){
        return -ERESTARTSYS;
    }

    page = (char*)__get_free_page(GFP_KERNEL);
    if(!page){
        err = -ENOMEM;
        goto nomem_err;
    }

    if(readl(reg_data_addr) | USR_DEF_RED_LED_MASK){
        err = snprintf(page, PAGE_SIZE, "1\n");
    }
    else{
        err = snprintf(page, PAGE_SIZE, "0\n");
    }
    
    if(err < 0){
        goto copy_err;
    }
    else{
        *off = err;
    }

    copy_to_user(buf, page, err);

copy_err:
    free_page((unsigned long)page);

nomem_err:
    up(&(ledctrl_dev->sem));

    return err;
}

static ssize_t ledctrl_dev_proc_write (struct file* filp, const char __user *buf, size_t  num, loff_t*  off)
{
    char* page = NULL;
    volatile void __iomem * reg_data_addr = ioremap((phys_addr_ledctrl + ledctrl_data_offset), 4);

    page = (char*)__get_free_page(GFP_KERNEL);
    if(!page){
        printk(KERN_ALERT "Fail to allocate page.\n");
        return -ENOMEM;
    }

    if(copy_from_user(page, buf, 1)){
        printk(KERN_ALERT "Fail to copy buff from user.\n");
        return -EFAULT;
        goto out;
    }

    if(down_interruptible(&(ledctrl_dev->sem))){
        return -ERESTARTSYS;
    }

    if('0' == page[0]){
        writel(readl(reg_data_addr) & ~USR_DEF_RED_LED_MASK, reg_data_addr);
    }
    else{
        writel(readl(reg_data_addr) | USR_DEF_RED_LED_MASK, reg_data_addr);
    }

    up(&(ledctrl_dev->sem));
out:
    free_page((unsigned long)page);
    return num;
}

static int ledctrl_dev_setup (struct ledctrl_hw_dev* ledctrl_dev)
{
    int   err;
    unsigned int reg_value;
    dev_t dev_no = MKDEV(ledctrl_major, ledctrl_minor);
    volatile void __iomem * reg_dir_addr = ioremap((phys_addr_ledctrl + ledctrl_dir_offset), 4);

    memset(ledctrl_dev, 0, sizeof(struct ledctrl_hw_dev));
    
    cdev_init(&(ledctrl_dev->dev), &ledctrl_fops);
    ledctrl_dev->dev.owner = THIS_MODULE;

    err = cdev_add(&(ledctrl_dev->dev), dev_no, 1);
    if(err){
        return err;
    }

    sema_init(&(ledctrl_dev->sem), 1);

    reg_value = readl(reg_dir_addr);
    writel(reg_value | USR_DEF_RED_LED_MASK, reg_dir_addr);

    return 0;
}

static void ledctrl_dev_create_proc (void)
{
    proc_create(LEDCTRL_DEVICE_PROC_NAME, 0, NULL, &ledctrl_proc_fops);
}

static void ledctrl_dev_remove_proc (void)
{
    remove_proc_entry(LEDCTRL_DEVICE_PROC_NAME, NULL);
}

static int __init ledctrl_dev_init (void)
{
    int   err    = -1;
    dev_t dev_no = 0;
    struct device_node * node_ledctrl;
    struct device*  tmp_dev = NULL;

    printk(KERN_ALERT "Initializing device ledctrl_dev");

    err = alloc_chrdev_region(&dev_no, 0, 1, LEDCTRL_DEVICE_NODE_NAME);
    if(err < 0){
        printk(KERN_ALERT "Fail to allocate char dev region.\n");
        goto fail;
    }

    ledctrl_major = MAJOR(dev_no);
    ledctrl_minor = MINOR(dev_no);

    ledctrl_dev = kmalloc(sizeof(struct ledctrl_hw_dev), GFP_KERNEL);
    if(!ledctrl_dev){
        err = -ENOMEM;
        printk(KERN_ALERT "Fail to allocate ledctrl device.\n");
        goto unregister;
    }

    err = ledctrl_dev_setup(ledctrl_dev);
    if(err){
        printk(KERN_ALERT "Fail to setup ledctrl dev: %d.\n", err);
        goto cleanup;
    }

    ledctrl_class = class_create (THIS_MODULE, LEDCTRL_DEVICE_CLASS_NAME);
    if(IS_ERR(ledctrl_class)){
        err = PTR_ERR(ledctrl_class);
        printk(KERN_ALERT "Fail to create ledctrl class.\n");
        goto destroy_cdev;
    }

    tmp_dev = device_create(ledctrl_class, NULL, dev_no, "%s", LEDCTRL_DEVICE_FILE_NAME);
    if(IS_ERR(tmp_dev)){
        err = PTR_ERR(tmp_dev);
        printk(KERN_ALERT "Fail to create ledctrl device.\n");
        goto destroy_class;
    }

    dev_set_drvdata(tmp_dev, ledctrl_dev);

    ledctrl_dev_create_proc();

    node_ledctrl = of_find_node_by_path("/soc/ledctrl-dev");
    if(NULL == node_ledctrl){
        printk(KERN_ALERT "ledctrl_dev_init: Fail to find node ledctrl GPIO1\n");
    }
    else{
        void *node_property;

        node_property       = of_get_property(node_ledctrl, "reg", NULL);
        bigendian_2_littleendian(node_property, sizeof(phys_addr_t));
        phys_addr_ledctrl   = *((phys_addr_t*)node_property);

        node_property       = of_get_property(node_ledctrl, "dir-offset", NULL);
        bigendian_2_littleendian(node_property, sizeof(int));
        ledctrl_dir_offset  = *((int*)node_property);

        node_property       = of_get_property(node_ledctrl, "data-offset", NULL);
        bigendian_2_littleendian(node_property, sizeof(int));
        ledctrl_data_offset = *((int*)node_property);
    }

    return 0;

destroy_class:
    class_destroy(ledctrl_class);

destroy_cdev:
    cdev_del(&(ledctrl_dev->dev));

cleanup:
    kfree(ledctrl_dev);

unregister:
    unregister_chrdev_region(dev_no, 1);

fail:
    return err;
}

static void __exit ledctrl_dev_exit (void)
{
    dev_t dev_no = MKDEV(ledctrl_major, ledctrl_minor);

    ledctrl_dev_remove_proc();

    if(ledctrl_dev){
        device_destroy(ledctrl_class, dev_no);
        class_destroy(ledctrl_class);
    }

    if(ledctrl_dev){
        cdev_del(&(ledctrl_dev->dev));
        kfree(ledctrl_dev);
    }

    unregister_chrdev_region(dev_no, 1);
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Flukenetworks driver to contrl LED usr_def_red_led on FreeScale IMX6 SABRESD");

module_init(ledctrl_dev_init);
module_exit(ledctrl_dev_exit);

