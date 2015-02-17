LOCAL_PATH := $(call my-dir)
    include $(CLEAR_VARS)
	LOCAL_MODULE_TAGS := eng
    LOCAL_MODULE := ledctrl_app
    LOCAL_SRC_FILES := $(call all-subdir-c-files)
    include $(BUILD_EXECUTABLE)

