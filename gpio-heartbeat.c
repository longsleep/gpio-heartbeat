/*
 * gpio-heartbeat - GPIO heartbeat sends a heartbeat like signal to GPIO pin
 *
 * Copyright (C) 2017 Simon Eisenmann <simon@longsleep.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>            // GPIO
#include <linux/kthread.h>         // threads
#include <linux/delay.h>           // msleep()
#include <linux/platform_device.h> // platform driver

static unsigned int gpioPIN = 77;
module_param(gpioPIN, uint, S_IRUGO);
MODULE_PARM_DESC(gpioPIN, " GPIO PIN number (default=77)");

static struct task_struct *task;
static bool gpioState = false;

#if defined(CONFIG_OF)
static struct of_device_id platform_device_match_table[] = {
	{ .compatible = "gpio-heartbeat" },
	{}
};
MODULE_DEVICE_TABLE(of, platform_device_match_table);
#endif

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

static int platform_device_probe(struct platform_device *pdev)
{
	return gpioHeartbeat_init();
}

static int platform_device_remove(struct platform_device *pdev)
{
	gpioHeartbeat_exit();
	return 0;
}

static struct platform_driver platform_device_platform_driver = {
	.probe = platform_device_probe,
	.remove = platform_device_remove,
	.driver = {
		.name = "gpio-heartbeat",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(platform_device_match_table),
	},
};

module_platform_driver(platform_device_platform_driver);

MODULE_DESCRIPTION("A simple Linux GPIO heartbeat driver");
MODULE_AUTHOR("Simon Eisenmann");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");
