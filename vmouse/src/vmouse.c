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


MODULE_AUTHOR("Florian Dollinger");
MODULE_DESCRIPTION("Mouse Emulation, designed to use in other modules");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");





struct instance_data {
    struct input_dev* idev;
	struct timer_list timer;

	int state_rel_x;
	int state_rel_y;
	int state_btn_left;
	int state_btn_right;
	int state_btn_middle;

	int registered_clients; // TODO:
};
static struct instance_data *mouse;
// TODO: currently global, not per instance


void vmouse_movement(int x, int value)
{
	if (x) {
		mouse->state_rel_x = value;
	} else {
		mouse->state_rel_y = value;
	}
}
EXPORT_SYMBOL(vmouse_movement);

void vmouse_leftclick(int value)
{
	mouse->state_btn_left = value;
}
EXPORT_SYMBOL(vmouse_leftclick);

void vmouse_rightclick(int value)
{
	mouse->state_btn_right = value;
}
EXPORT_SYMBOL(vmouse_rightclick);

void vmouse_middleclick(int value)
{
	mouse->state_btn_middle = value;
}
EXPORT_SYMBOL(vmouse_middleclick);


static void send_report(struct timer_list *t) {

	input_report_rel(mouse->idev, REL_X, mouse->state_rel_x);
	input_report_rel(mouse->idev, REL_Y, mouse->state_rel_y);
	input_report_key(mouse->idev, BTN_LEFT, mouse->state_btn_left);
	input_report_key(mouse->idev, BTN_RIGHT, mouse->state_btn_right);
	input_report_key(mouse->idev, BTN_MIDDLE, mouse->state_btn_middle);
	input_sync(mouse->idev);

	mod_timer(&mouse->timer, jiffies + msecs_to_jiffies(10));

}

int init_module(void)
{
    int retval;

	struct input_dev *input_dev;

    /* allocate and zero a new data structure for the new device */
    mouse = kmalloc(sizeof(struct instance_data), GFP_KERNEL);
    if (!mouse) return -ENOMEM; /* failure */
    memset(mouse, 0, sizeof(*mouse));


	printk(KERN_ERR "vmouse: hi there!\n");

	input_dev = input_allocate_device();
	if (!input_dev) {
		printk(KERN_ERR "Not enough memory\n");
		retval = -ENOMEM;
	}

	mouse->idev = input_dev;

	input_dev->name = "Mouse Emulation";
	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	input_dev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT) | BIT_MASK(BTN_MIDDLE);
	input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y) | BIT_MASK(REL_WHEEL);

	input_set_drvdata(input_dev, mouse);

	retval = input_register_device(input_dev);
	if (retval) {
		printk(KERN_ERR "Failed to register device\n");
		goto err_free_dev;
	}

	timer_setup(&mouse->timer, send_report, 0);
	mod_timer(&mouse->timer, jiffies + msecs_to_jiffies(10));


	return 0;

err_free_dev:
	input_free_device(mouse->idev);
	kfree(mouse);
	return retval;

}

void cleanup_module(void)
{
	del_timer_sync(&mouse->timer);

	if(!mouse) return;

	input_unregister_device(mouse->idev);
	kfree(mouse);
}