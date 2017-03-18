/**
 * A simple Linux Kernel module, which sends a heartbeat like signal to a GPIO
 * PIN using a Kernel thread when loaded.
 *
 * The module has a parameter gpioPIN to change the GPIO PIN from the default.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>       // GPIO
#include <linux/kthread.h>    // threads
#include <linux/delay.h>      // msleep()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Simon Eisenmann");
MODULE_DESCRIPTION("A simple Linux GPIO heartbeat driver");
MODULE_VERSION("0.1");

static unsigned int gpioPIN = 77;
module_param(gpioPIN, uint, S_IRUGO);
MODULE_PARM_DESC(gpioPIN, " GPIO PIN number (default=77)");

static struct task_struct *task;
static bool gpioState = false;

static int heartbeat(void *arg)
{
	int count = 0;
	int timeout = 0;
	printk(KERN_INFO "GPIO Heartbeat: thread started\n");

	while (!kthread_should_stop()) {
		set_current_state(TASK_RUNNING);
		count++;
		gpioState = !gpioState;
		gpio_set_value(gpioPIN, gpioState);
		if (count % 4 == 0) {
			timeout = 1250;
			count = 0;
		} else {
			timeout = 120;
		}
		set_current_state(TASK_INTERRUPTIBLE);
		msleep(timeout);
	}

	printk(KERN_INFO "GPIO Heartbeat: thread complete\n");
	return 0;
}

static int __init gpioHeartbeat_init(void)
{
	int result = 0;
	printk(KERN_INFO "GPIO Heartbeat: start\n");

	gpio_request(gpioPIN, "sysfs");
	gpio_direction_output(gpioPIN, false); // Turn off by default.
	gpio_export(gpioPIN, false); // Export, but prevent direction change.

	task = kthread_run(heartbeat, NULL, "GPIO_heartbeat_thread");
	if (IS_ERR(task)) {
		printk(KERN_ALERT "GPIO Heartbeat: failed to create task\n");
		return PTR_ERR(task);
	}

	return result;
}

static void __exit gpioHeartbeat_exit(void)
{
	kthread_stop(task);
	gpio_set_value(gpioPIN, 0);
	gpio_unexport(gpioPIN);
	gpio_free(gpioPIN);
	printk(KERN_INFO "GPIO Heartbeat: exit\n");
}

module_init(gpioHeartbeat_init);
module_exit(gpioHeartbeat_exit);
