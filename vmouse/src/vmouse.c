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
#include <linux/input.h>
#include <linux/timer.h>

MODULE_AUTHOR("Florian Dollinger");
MODULE_DESCRIPTION("Mouse Emulation, designed to use in other modules");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");


struct instance_data {
	struct input_dev* idev;
	struct timer_list timer;

	int report_rate_ms;

	int state_rel_x;
	int state_rel_y;
	int state_rel_wheel;

	int state_btn_left;
	int state_btn_right;
	int state_btn_middle;

	 // TODO:
	int registered_clients;
};
// TODO: currently global, not per instance
static struct instance_data *vmouse;


// TODO: implement
int register_client(void) { return -1; };
int unregister_client(void) { return -1; };

void vmouse_movement(int x, int value)
{
	if (x) {
		vmouse->state_rel_x = value;
	} else {
		vmouse->state_rel_y = value;
	}
}
EXPORT_SYMBOL(vmouse_movement);

void vmouse_wheel(int value)
{
	vmouse->state_rel_wheel = value;
}
EXPORT_SYMBOL(vmouse_wheel);

void vmouse_leftclick(int value)
{
	vmouse->state_btn_left = value;
}
EXPORT_SYMBOL(vmouse_leftclick);

void vmouse_rightclick(int value)
{
	vmouse->state_btn_right = value;
}
EXPORT_SYMBOL(vmouse_rightclick);

void vmouse_middleclick(int value)
{
	vmouse->state_btn_middle = value;
}
EXPORT_SYMBOL(vmouse_middleclick);


static void send_report(struct timer_list *t) {

	input_report_rel(vmouse->idev, REL_X, vmouse->state_rel_x);
	input_report_rel(vmouse->idev, REL_Y, vmouse->state_rel_y);
	input_report_rel(vmouse->idev, REL_WHEEL, vmouse->state_rel_wheel);

	input_report_key(vmouse->idev, BTN_LEFT, vmouse->state_btn_left);
	input_report_key(vmouse->idev, BTN_RIGHT, vmouse->state_btn_right);
	input_report_key(vmouse->idev, BTN_MIDDLE, vmouse->state_btn_middle);

	input_sync(vmouse->idev);

	/* restart timer */
	mod_timer(&vmouse->timer, jiffies + msecs_to_jiffies(vmouse->report_rate_ms));
}

int init_module(void)
{
	int retval;
	struct input_dev *input_dev;

	printk(KERN_ERR "vmouse: hi there!\n");

	/* allocate the input device */
	input_dev = input_allocate_device();
	if (!input_dev) {
		printk(KERN_ERR "Not enough memory\n");
		return -ENOMEM;
	}

	/* allocate and zero the vmouse struct and let the device
	 * subsystem handle the freeing of it
	 */
	vmouse = devm_kzalloc(&input_dev->dev, sizeof(*vmouse), GFP_KERNEL);
	if (!vmouse) {
		retval = -ENOMEM;
		goto err_free_dev;
	}
	vmouse->idev = input_dev;

	vmouse->report_rate_ms = 10;

	vmouse->idev->name = "Mouse Emulation";
	vmouse->idev->evbit[0]
		= BIT_MASK(EV_KEY)
		| BIT_MASK(EV_REL);
	vmouse->idev->keybit[BIT_WORD(BTN_MOUSE)]
		= BIT_MASK(BTN_LEFT)
		| BIT_MASK(BTN_RIGHT)
		| BIT_MASK(BTN_MIDDLE);
	vmouse->idev->relbit[0]
		= BIT_MASK(REL_X)
		| BIT_MASK(REL_Y)
		| BIT_MASK(REL_WHEEL);

	input_set_drvdata(vmouse->idev, vmouse);

	retval = input_register_device(vmouse->idev);
	if (retval) {
		printk(KERN_ERR "Failed to register device\n");
		goto err_free_dev;
	}

	/* setup and start timer */
	timer_setup(&vmouse->timer, send_report, 0);
	mod_timer(&vmouse->timer, jiffies + msecs_to_jiffies(vmouse->report_rate_ms));

	return 0; // s'all good

err_free_dev:
	input_free_device(vmouse->idev);
	return retval;
}

void cleanup_module(void)
{
	if(!vmouse)
		return;

	del_timer_sync(&vmouse->timer);

	/* has to be the last operation since vmouse gets removed by the device
	 * subsystem as soon as the input device is removed
	 */
	input_unregister_device(vmouse->idev);
}