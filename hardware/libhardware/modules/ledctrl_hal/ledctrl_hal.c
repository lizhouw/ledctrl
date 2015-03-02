#include <hardware/hardware.h>
#include <hardware/ledctrl_hal.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#define LEDCTRL_LOG_TAG       "LEDCTRL_HAL"
#define DEVICE_TAG            "LEDCTRL_HAL"
#define DEVICE_NAME           "/dev/ledctrl_dev"
#define MODULE_AUTHOR         "lizhou.wan@flukenetworks.com"

static int ledctrl_device_open   (const hw_module_t*, const char*, struct hw_device_t**);
static int ledctrl_device_close  (struct hw_device_t*);
static int ledctrl_set_status    (struct ledctrl_hal_device_t*, char);
static int ledctrl_get_status    (struct ledctrl_hal_device_t*, char*);

static struct hw_module_methods_t ledctrl_module_methods = {
    open: ledctrl_device_open,
};

struct ledctrl_hal_module_t HAL_MODULE_INFO_SYM = {
    mod_common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: LEDCTRL_MODULE_ID,
        name: LEDCTRL_MODULE_NAME,
        author: MODULE_AUTHOR,
        methods: &ledctrl_module_methods,
    }
};

static int ledctrl_device_open (const hw_module_t* module, const char* name, struct hw_device_t** device)
{
    struct ledctrl_hal_device_t* dev;

    dev = (struct ledctrl_hal_device_t*)malloc(sizeof(struct ledctrl_hal_device_t));
    if(!dev){
        ALOGE("[%s] Fail to allocate space", LEDCTRL_LOG_TAG);
        return -EFAULT;
    }

    memset(dev, 0, sizeof(struct ledctrl_hal_device_t));
    dev->dev_common.tag = HARDWARE_DEVICE_TAG;
    dev->dev_common.version = 1;
    dev->dev_common.module = (hw_module_t*)module;
    dev->dev_common.close = ledctrl_device_close;
    dev->set_status = ledctrl_set_status;
    dev->get_status = ledctrl_get_status;
    if((dev->fd = open(DEVICE_NAME, O_RDWR)) < 0){
        ALOGE("[%s] Fail to open device %s", LEDCTRL_LOG_TAG, DEVICE_NAME);
        free(dev);
        return -EFAULT;
    }

    *device = &(dev->dev_common);
    ALOGI("[%s] Succeed to open device %s", LEDCTRL_LOG_TAG, DEVICE_NAME);

    return 0;
}

static int ledctrl_device_close(struct hw_device_t* device)
{
    struct ledctrl_hal_device_t* ledctrl_dev = (struct ledctrl_hal_device_t*)device;

    if(ledctrl_dev){
        close(ledctrl_dev->fd);
        free(ledctrl_dev);
    }

    return 0;
}

static int ledctrl_set_status (struct ledctrl_hal_device_t* dev, char status)
{
    ALOGI("[%s] switch %s the led USR_DEF_RED_LED", LEDCTRL_LOG_TAG, status ? "on" : "off");

    write(dev->fd, &status, 1);
    return 0;
}

static int ledctrl_get_status (struct ledctrl_hal_device_t* dev, char* status)
{
    read(dev->fd, status, 1);

    ALOGI("[%s] the led USR_DEF_RED_LED is %s", LEDCTRL_LOG_TAG, *status ? "on" : "off");

    return 0;
}

