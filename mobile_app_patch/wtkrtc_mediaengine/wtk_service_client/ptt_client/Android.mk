LOCAL_PATH:= $(call my-dir)/../../../../src/pttclient
include $(call my-dir)/../config.mk

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_MODULE := libpttclient


#LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES = \
	ptt.c \
	pttclient.c \
	tbcp.c \
	util.c

_OS_TYPE := ANDROID

LOCAL_CFLAGS := \
	-D$(_OS_TYPE) \
	-D_POSIX_SOURCE -Wall

ifeq ($(FREEPP_ONLY_VOICE),true)

VE2_INCLUDES := \
	$(LOCAL_PATH)/.. \
	$(LOCAL_PATH)/../iaxclient
	
LOCAL_CFLAGS += -DHAVE_GETTIMEOFDAY -std=c99

else

VE2_INCLUDES := \
	$(LOCAL_PATH)/.. \
	$(LOCAL_PATH)/../iaxclient
	 
LOCAL_CFLAGS += -DHAVE_GETTIMEOFDAY -DUSE_VIDEO -std=c99 -DRTCP_STATISTICS

endif

LOCAL_C_INCLUDES += \
	$(VE2_INCLUDES)

LOCAL_STATIC_LIBRARIES := \

LOCAL_PRELINK_MODULE := false

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog

LOCAL_STATIC_LIBRARIES := \

ifeq ($(strip $(BOARD_USES_ALSA_AUDIO)),true)
LOCAL_SHARED_LIBRARIES += libasound
endif

include $(BUILD_STATIC_LIBRARY)

