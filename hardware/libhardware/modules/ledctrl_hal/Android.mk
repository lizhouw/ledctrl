LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ledctrl_hal.default

# HAL module implementation stored in
# hw/<LEDCTRL_MODULE_ID>.default.so
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_C_INCLUDES := hardware/libhardware
LOCAL_SRC_FILES := ledctrl_hal.c
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

