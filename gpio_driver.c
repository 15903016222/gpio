#include <linux/module.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>

#include <linux/dmaengine.h>
#include <linux/device.h>

#include <linux/io.h>
#include <linux/delay.h>

#define DEVICE_NAME "gpio"
#define CLASS_NAME  "gpio_test"

static int gMajor; /* major number of device */
static struct class *gpio_class;

#define EIM_EB0_REG    0x020E010C
#define EIM_EB1_REG    0x020E0110

#define IMX_GPIO_NR(bank, nr)  (((bank) - 1) * 32 + (nr))
#define EIM_EB0 (IMX_GPIO_NR(2, 28))
#define EIM_EB1 (IMX_GPIO_NR(2, 29))

#define EIM_EB1_LOW    (0x6000)
#define EIM_EB1_HIGH   (0x6001)
#define EIM_EB0_LOW    (0x6002)
#define EIM_EB0_HIGH   (0x6003)

static long gpio_ioctl (struct file *filp,
                        unsigned int cmd,
                        unsigned long arg)
{
    unsigned int tmp, i;
    if (arg)
        i = copy_from_user(&tmp,(void *)arg,sizeof(tmp));

    switch (cmd) {
        case EIM_EB0_HIGH:
            gpio_set_value(EIM_EB0, 1);
            break;

        case EIM_EB0_LOW:
            gpio_set_value(EIM_EB0, 0);
            break;

        case EIM_EB1_HIGH:
            gpio_set_value(EIM_EB1, 1);
            break;

        case EIM_EB1_LOW:
            gpio_set_value(EIM_EB1, 0);
            break;

        default:
            break;
    }

    return 0;
}

struct file_operations gpio_fops = {
    unlocked_ioctl: gpio_ioctl,
};

int __init gpio_init_module(void)
{
    unsigned int *addr = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
    struct device *temp_class;
#else
    struct class_device *temp_class;
#endif
    int error;

    /* register a character device */
    error = register_chrdev(0, DEVICE_NAME, &gpio_fops);
    if (error < 0) {
        printk("gpio test driver can't get major number\n");
        return error;
    }
    gMajor = error;
    printk("gpio test major number = %d\n",gMajor);

    /*  */
    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpio_class)) {
        printk(KERN_ERR "Error creating register test module class.\n");
        unregister_chrdev(gMajor, DEVICE_NAME);
        return PTR_ERR(gpio_class);
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
    temp_class = device_create(gpio_class, NULL,
                   MKDEV(gMajor, 0), NULL, DEVICE_NAME);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
    temp_class = device_create(gpio_class, NULL,
                   MKDEV(gMajor, 0), DEVICE_NAME);
#else
    temp_class = class_device_create(gpio_class, NULL,
                         MKDEV(gMajor, 0), NULL,
                         DEVICE_NAME);
#endif
    if (IS_ERR(temp_class)) {
        printk(KERN_ERR "Error creating gpio test class device.\n");
        class_destroy(gpio_class);
        unregister_chrdev(gMajor, DEVICE_NAME);
        return -1;
    }

    addr = (unsigned int *)ioremap(EIM_EB0_REG, 8);
    *addr = 0x5;                                   // set EIM_EB0 gpio function
    *(addr + 1) = 0x5;                             // set EIM_EB1 gpio function
    iounmap(addr);

    gpio_request (EIM_EB0, "GPIO2_IO28");
    gpio_request (EIM_EB1, "GPIO2_IO29");
    gpio_direction_output (EIM_EB0, 0);
    gpio_direction_output (EIM_EB1, 0);

    printk("gpio test Driver Module loaded\n");
	return 0;
}

static void gpio_cleanup_module(void)
{
    gpio_set_value (EIM_EB0, 0);
    gpio_set_value (EIM_EB1, 0);
    gpio_free (EIM_EB0);
    gpio_free (EIM_EB1);

    unregister_chrdev(gMajor, DEVICE_NAME);
    device_destroy(gpio_class, MKDEV(gMajor, 0));
    class_destroy(gpio_class);

    printk("gpio test Driver Module Unloaded\n");
}

module_init(gpio_init_module);
module_exit(gpio_cleanup_module);

MODULE_DESCRIPTION("gpio test driver");
MODULE_LICENSE("GPL");
