#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include <utils/misc.h>
#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/ledctrl_hal.h>
#include <stdio.h>

#define LEDCTRL_LOG_TAG   "LEDCTRL_JNI"

namespace android
{
    struct ledctrl_hal_device_t*  ledctrl_dev = NULL;

    static void ledctrl_jni_set_status (JNIEnv* env, jobject clazz, jchar value)
    {
        if(!ledctrl_dev){
            ALOGE("[%s] ledctrl_jni_set_status: the device is not open", LEDCTRL_LOG_TAG);
            return;
        }

        ALOGI("[%s] ledctrl_jni_set_status: switch %s the LED USR_DEF_RED_LED", LEDCTRL_LOG_TAG, value ? "on" : "off");
        ledctrl_dev->set_status(ledctrl_dev, value);
    }

    static jchar ledctrl_jni_get_status (JNIEnv* env, jobject clazz)
    {
        jchar status;

        if(!ledctrl_dev){
            ALOGE("[%s] ledctrl_jni_get_status: the device is not open", LEDCTRL_LOG_TAG);
            return -1;
        }

        ledctrl_dev->get_status(ledctrl_dev, (char*)&status);
        ALOGI("[%s] ledctrl_jni_get_status: the led is %s", LEDCTRL_LOG_TAG, status ? "on" : "off");

        return status;
    }

    static inline int ledctrl_jni_open(const hw_module_t* module, struct ledctrl_hal_device_t** device)
    {
        return module->methods->open(module, LEDCTRL_MODULE_NAME, (struct hw_device_t**)device);
    }

    static void ledctrl_jni_init (JNIEnv* env, jclass clazz)
    {
        ledctrl_hal_module_t* module;

        ALOGI("[%s] Initializing......", LEDCTRL_LOG_TAG);
        if((0 == hw_get_module(LEDCTRL_MODULE_ID, (const struct hw_module_t**)&module)) &&
           (0 == ledctrl_jni_open(&(module->mod_common), &ledctrl_dev))){
            ALOGI("[%s] Open device successfully", LEDCTRL_LOG_TAG);
        }
        else{
            ALOGE("[%s] Open device unsuccessfully", LEDCTRL_LOG_TAG);
        }
    }

    static const JNINativeMethod method_table[] = {
        { "init_native",      "()V",     (void*)ledctrl_jni_init       },
        { "setstatus_native", "(I)V",    (void*)ledctrl_jni_set_status },
        { "getstatus_native", "()I",     (void*)ledctrl_jni_get_status },
    };

    int register_android_server_LedCtrlMgr(JNIEnv* env){
        return jniRegisterNativeMethods(env, "com/android/server/LedCtrlMgr", method_table, NELEM(method_table));
    }
};
