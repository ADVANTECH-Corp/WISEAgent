LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# My macros define 
MY_C_FILE_LIST	:= $(wildcard $(LOCAL_PATH)/*.c)
MY_C_FILE_LIST	+= $(wildcard $(LOCAL_PATH)/Linux/*.c)
MY_EXCLUDE_LIST := Linux/common.c Linux/platform.c

# Module
LOCAL_MODULE 	:= Platform 
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../Include 
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../Library/Log
LOCAL_CFLAGS 	:= -fPIC
LOCAL_SRC_FILES	:= $(filter-out $(MY_EXCLUDE_LIST), $(MY_C_FILE_LIST:$(LOCAL_PATH)/%=%))

# Export and Build 
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH) 
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/Linux 
include $(BUILD_STATIC_LIBRARY)

# Debug 
#$(warning $(LOCAL_EXPORT_C_INCLUDES))
