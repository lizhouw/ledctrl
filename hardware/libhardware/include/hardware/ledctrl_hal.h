#ifndef ANDROID_LEDCTRL_HAL_INCLUDE_H
#define ANDROID_LEDCTRL_HAL_INCLUDE_H

#include <hardware/hardware.h>

#define LEDCTRL_MODULE_ID             "ledctrl_hal"
#define LEDCTRL_MODULE_NAME           "ledctrl_hal"

__BEGIN_DECLS

struct ledctrl_hal_module_t {
    struct hw_module_t mod_common;
};

struct ledctrl_hal_device_t {
    struct hw_device_t dev_common;
    int fd;
    int (*set_status) (struct ledctrl_hal_device_t* dev, char status);
    int (*get_status) (struct ledctrl_hal_device_t* dev, char* status);
};

__END_DECLS

#endif
