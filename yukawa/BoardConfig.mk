include device/amlogic/yukawa/BoardConfigCommon.mk

DEVICE_MANIFEST_FILE += device/amlogic/yukawa/yukawa/manifest.xml
TARGET_BOARD_INFO_FILE := device/amlogic/yukawa/sei610/board-info.txt

ifeq ($(TARGET_USE_AB_SLOT), true)
BOARD_USERDATAIMAGE_PARTITION_SIZE := 11846811648
else
BOARD_USERDATAIMAGE_PARTITION_SIZE := 13415481344
endif