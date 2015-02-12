#ifndef __LEDCTRL_DEV_H__
#define __LEDCTRL_DEV_H__


#include <linux/cdev.h>
#include <linux/semaphore.h>

#define LEDCTRL_DEVICE_NODE_NAME   "ledctrl_dev"
#define LEDCTRL_DEVICE_FILE_NAME   "ledctrl_dev"
#define LEDCTRL_DEVICE_PROC_NAME   "ledctrl_dev"
#define LEDCTRL_DEVICE_CLASS_NAME  "ledctrl_dev"

struct ledctrl_hw_dev {
	struct  semaphore  sem;
	struct  cdev       dev;
};

#endif
