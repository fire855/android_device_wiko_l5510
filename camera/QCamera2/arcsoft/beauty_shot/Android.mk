################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
#-----------------------------------------------------------
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE := libarcsoft_beauty_shot
LOCAL_PREBUILT_LIBS := libarcsoft_beauty_shot.so
#LOCAL_LDLIBS := -ldl

#-----------------------------------------------------------
include $(BUILD_MULTI_PREBUILT)
#include $(BUILD_SHARED_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))