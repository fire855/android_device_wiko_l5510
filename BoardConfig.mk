#
# Copyright (C) 2014 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

-include device/cyanogen/msm8916-common/BoardConfigCommon.mk

DEVICE_PATH := device/wiko/l5510

TARGET_SPECIFIC_HEADER_PATH := $(DEVICE_PATH)/include
FORCE_32_BIT := true

# Kernel
TARGET_KERNEL_CONFIG := msm8916_l5510_defconfig

BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth

# Camera
#BOARD_CAMERA_SENSORS := \
#    ov5670_q5v41b \
#    ov8865_q8v18a
TARGET_USE_VENDOR_CAMERA_EXT := true
USE_DEVICE_SPECIFIC_CAMERA := true

# GPS
TARGET_GPS_HAL_PATH := $(DEVICE_PATH)/gps
TARGET_NO_RPC := true

# Lights
TARGET_PROVIDES_LIBLIGHT := true

# Partitions
BOARD_FLASH_BLOCK_SIZE := 131072
BOARD_BOOTIMAGE_PARTITION_SIZE := 0x02000000
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 0x02000000
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1702887424
BOARD_USERDATAIMAGE_PARTITION_SIZE := 12936655872
BOARD_CACHEIMAGE_PARTITION_SIZE := 274726912
BOARD_PERSISTIMAGE_PARTITION_SIZE := 33554432

# Video
TARGET_HAVE_SIGNED_VENUS_FW := true

BOARD_SEPOLICY_DIRS += \
    device/wiko/l5510/sepolicy

BOARD_SEPOLICY_UNION += \
    device.te \
    file.te \
    file_contexts \
    genfs_contexts \
    netd.te \
    ppp.te \
    vold.te

TARGET_SYSTEM_PROP := $(DEVICE_PATH)/system.prop

# inherit from the proprietary version
-include vendor/wiko/l5510/BoardConfigVendor.mk
