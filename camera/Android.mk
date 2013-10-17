#
# Copyright (C) 2013 Paul Kocialkowski
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

ifneq ($(filter n5100 n5110,$(TARGET_DEVICE)),)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	exynos_camera.c \
	exynos_exif.c \
	exynos_jpeg.c \
	exynos_param.c \
	exynos_utils.c \
	exynos_v4l2.c \
	exynos_v4l2_output.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	hardware/samsung/exynos4/hal/include

LOCAL_SHARED_LIBRARIES := libutils libcutils liblog libcamera_client libhardware
LOCAL_PRELINK_MODULE := false

ifeq ($(TARGET_SOC),exynos4x12)
	LOCAL_SHARED_LIBRARIES += libhwjpeg
	LOCAL_CFLAGS += -DEXYNOS_JPEG_HW

	LOCAL_SRC_FILES += exynos_ion.c
	LOCAL_CFLAGS += -DEXYNOS_ION
endif

LOCAL_MODULE := camera.$(TARGET_BOOTLOADER_BOARD_NAME)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif
