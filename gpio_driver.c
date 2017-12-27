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

#define IMX_GPIO_NR(bank, nr)  (((bank) - 1) * 32 + (nr))
#define EIM_EB0 (IMX_GPIO_NR(2, 28))
#define EIM_EB1 (IMX_GPIO_NR(2, 29))

#define EIM_EB0_HIGH   (0x60000)
#define EIM_EB0_LOW    (0x60001)
#define EIM_EB1_HIGH   (0x60002)
#define EIM_EB1_LOW    (0x60003)

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
    unlocked_ioctl: gpio_ioctl,
};

static struct miscdevice gpio_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "gpio",
    .fops = &gpio_fops
};

int __init gpio_init_module(void)
{
    unsigned int *addr = NULL;
    addr = (unsigned int *)ioremap(0x020e010c, 8);
    *addr = 0x5;
    *(addr + 1) = 0x5;
    iounmap(addr);

	/* register a character device */
    misc_register(&gpio_misc);

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

    misc_deregister(&gpio_misc);
    printk("gpio test Driver Module Unloaded\n");
}


module_init(gpio_init_module);
module_exit(gpio_cleanup_module);

MODULE_DESCRIPTION("gpio test driver");
MODULE_LICENSE("GPL");
