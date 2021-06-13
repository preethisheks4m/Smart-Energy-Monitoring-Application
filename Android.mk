LOCAL_PATH := $(call my-dir)    

cmd-strip = $(TOOLCHAIN_PREFIX)strip --strip-all --strip-debug --strip-unneeded  -x $1

LOCAL_CPPFLAGS += -ffunction-sections -fdata-sections
LOCAL_CFLAGS += -ffunction-sections -fdata-sections 
LOCAL_LDFLAGS += -Wl,--gc-sections
LOCAL_CPPFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -fvisibility=hidden

include $(CLEAR_VARS)    
 
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog    

LOCAL_MODULE    := crypt    
 
LOCAL_SRC_FILES := crypt.c    
 
include $(BUILD_SHARED_LIBRARY)  