/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for LibreTech CC
 *
 * Copyright (C) 2017 Baylibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SPLASHIMAGE_GUARD
#define CONFIG_SPLASH_SOURCE
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (512*512*4)
#define CONFIG_SPLASHIMAGE_GUARD
#define CONFIG_SPLASH_SOURCE

#ifndef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(ROMUSB, romusb, na)  \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	BOOT_TARGET_DEVICES_USB(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#endif


#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
        "stdin=" STDIN_CFG "\0" \
        "stdout=" STDOUT_CFG "\0" \
        "stderr=" STDOUT_CFG "\0" \
        "fdt_addr_r=0x08008000\0" \
        "scriptaddr=0x08000000\0" \
        "kernel_addr_r=0x08080000\0" \
        "pxefile_addr_r=0x01080000\0" \
        "ramdisk_addr_r=0x13000000\0" \
        "fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
        "splashimage=0x12000000\0" \
        "splashpos=m,m\0" \
        "splashfile=u-boot.bmp\0" \
        BOOTENV
#endif

#include <configs/meson64.h>

#endif /* __CONFIG_H */
