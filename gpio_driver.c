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

#include <linux/dmaengine.h>
#include <linux/device.h>

#include <linux/io.h>
#include <linux/delay.h>

#define IMX_GPIO_NR(bank, nr)  (((bank) - 1) * 32 + (nr))
#define EIM_EB0 (IMX_GPIO_NR(2, 28))
#define EIM_EB1 (IMX_GPIO_NR(2, 29))

#define EIM_EB0_HIGH   (0x60000)
#define EIM_EB0_LOW    (0x60001)
#define EIM_EB1_HIGH   (0x60002)
#define EIM_EB1_LOW    (0x60003)

static int gMajor; /* major number of device */
static struct class *gpio_class;

static long gpio_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case EIM_EB0_HIGH:
            gpio_set_value(EIM_EB0, 1);
            printk ("EIM_EB0_HIGH \n");
            break;

        case EIM_EB0_LOW:
            gpio_set_value(EIM_EB0, 0);
            printk ("EIM_EB0_LOW \n");
            break;

        case EIM_EB1_HIGH:
            gpio_set_value(EIM_EB1, 1);
            printk ("EIM_EB1_HIGH \n");
            break;

        case EIM_EB1_LOW:
            gpio_set_value(EIM_EB1, 0);
            printk ("EIM_EB1_LOW \n");
            break;

        default:
            break;
    }

    return 0;
}

struct file_operations gpio_fops = {
    compat_ioctl: gpio_ioctl,
};

int __init gpio_init_module(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	struct device *temp_class;
#else
	struct class_device *temp_class;
#endif
	int error;

	/* register a character device */
    error = register_chrdev(0, "gpio", &gpio_fops);
	if (error < 0) {
        printk("gpio test driver can't get major number\n");
		return error;
	}
	gMajor = error;
    printk("gpio test major number = %d\n",gMajor);

    gpio_class = class_create(THIS_MODULE, "gpio");
    if (IS_ERR(gpio_class)) {
        printk(KERN_ERR "Error creating gpio module class.\n");
        unregister_chrdev(gMajor, "gpio");
        return PTR_ERR(gpio_class);
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
    temp_class = device_create(gpio_class, NULL,
                   MKDEV(gMajor, 0), NULL, "gpio");
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
    temp_class = device_create(gpio_class, NULL,
                   MKDEV(gMajor, 0), "gpio");
#else
    temp_class = class_device_create(gpio_class, NULL,
					     MKDEV(gMajor, 0), NULL,
                         "gpio");
#endif
	if (IS_ERR(temp_class)) {
		printk(KERN_ERR "Error creating register test class device.\n");
        class_destroy(gpio_class);
        unregister_chrdev(gMajor, "gpio");
		return -1;
	}

    /* gpio */
    writel (0x00000005, (volatile void *)0x020e010c);
    writel (0x00000005, (volatile void *)0x020e0110);

    gpio_request (EIM_EB0, "GPIO2_IO28");
    gpio_request (EIM_EB1, "GPIO2_IO29");
    gpio_direction_output (EIM_EB0, 0);
    gpio_direction_output (EIM_EB1, 0);

    printk("gpio test Driver Module loaded\n");
	return 0;
}

static void gpio_cleanup_module(void)
{
    gpio_direction_output (EIM_EB0, 0);
    gpio_direction_output (EIM_EB1, 0);
    gpio_free (EIM_EB0);
    gpio_free (EIM_EB1);

    unregister_chrdev(gMajor, "gpio");
    device_destroy(gpio_class, MKDEV(gMajor, 0));
    class_destroy(gpio_class);

    printk("gpio test Driver Module Unloaded\n");
}


module_init(gpio_init_module);
module_exit(gpio_cleanup_module);

MODULE_DESCRIPTION("register test driver");
MODULE_LICENSE("GPL");
