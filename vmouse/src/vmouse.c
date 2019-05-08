/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/input.h>

/*
 * Version Information
 */
#define DRIVER_VERSION "v0.1"

MODULE_AUTHOR("Florian Dollinger");
MODULE_DESCRIPTION("Mouse Emulation, designed to use in other modules");
MODULE_LICENSE("GPL");

struct instance_data {
    struct input_dev *idev;
};
static struct instance_data *mouse;


void vmouse_movement(int x, int value)
{
	input_report_rel(mouse->idev, (x == 1 ? REL_X : REL_Y), value);
	input_sync(mouse->idev);

}
EXPORT_SYMBOL(vmouse_movement);

int init_module(void)
{
    int retval;

	struct input_dev *input_dev;

    /* allocate and zero a new data structure for the new device */
    mouse = kmalloc(sizeof(struct instance_data), GFP_KERNEL);
    if (!mouse) return -ENOMEM; /* failure */
    memset(mouse, 0, sizeof(*mouse));


	printk(KERN_ERR "vmouse: hi!\n");

	input_dev = input_allocate_device();
	if (!input_dev) {
		printk(KERN_ERR "Not enough memory\n");
		retval = -ENOMEM;
	}

	mouse->idev = input_dev;

	input_dev->name = "Mouse Emulation";
	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	input_dev->keybit[0] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT) | BIT_MASK(BTN_MIDDLE);
	input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y) | BIT_MASK(REL_WHEEL);

	input_set_drvdata(input_dev, mouse);

	retval = input_register_device(input_dev);
	if (retval) {
		printk(KERN_ERR "Failed to register device\n");
		goto err_free_dev;
	}

	return 0;

err_free_dev:
	input_free_device(mouse->idev);
	kfree(mouse);
	return retval;

}

void cleanup_module(void)
{
	if(!mouse) return;

	input_unregister_device(mouse->idev);
	kfree(mouse);
}