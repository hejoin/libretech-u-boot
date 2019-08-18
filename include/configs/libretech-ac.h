/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for LibreTech AC
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
//#define CONFIG_MENU_SHOW
#define CONFIG_MENUKEY 0x1b
#define CONFIG_MENUPROMPT "Press ESC for menu.   "

#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_OFFSET	(-0x10000)

#ifndef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(ROMUSB, romusb, na)  \
	func(MMC, mmc, 0) \
	BOOT_TARGET_DEVICES_USB(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#endif

#define PARTS_DEFAULT \
	"name=esp,start=1M,size=255M,bootable,uuid=${uuid_gpt_esp};" \
	"name=system,size=-,uuid=${uuid_gpt_system};"

#define UUID_GPT_ESP "c12a7328-f81f-11d2-ba4b-00a0c93ec93b"
#define UUID_GPT_SYSTEM "b921b045-1df0-41c3-af44-4c6f280d3fae"

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
        "splashfile=boot.bmp\0" \
	"bootmenu_0=Boot=boot; echo \"Boot failed.\"; sleep 30; $menucmd\0" \
	"bootmenu_1=Boot USB=run bootcmd_usb0; echo \"USB Boot failed.\"; sleep 5; $menucmd -1\0" \
	"bootmenu_2=Boot PXE=run bootcmd_pxe; echo \"PXE Boot failed.\"; sleep 5; $menucmd -1\0" \
	"bootmenu_3=Boot DHCP=run bootcmd_dhcp; echo \"DHCP Boot failed.\"; sleep 5; $menucmd -1\0" \
	"bootmenu_4=Embedded OS=echo \"Not Implemented.\"; sleep 5; $menucmd -1\0" \
	"bootmenu_5=eMMC USB Drive Mode=mmc list; if mmc dev 0; then echo \"Press Control+C to end USB Drive mode.\"; ums 0 mmc 0:0; echo \"USB Drive mode ended.\"; else echo \"eMMC not detected.\"; fi; sleep 5; $menucmd -1\0" \
	"bootmenu_6=fastboot USB Mode=echo \"Press Control+C to end fastboot mode.\"; fastboot usb 0; echo \"fastboot mode ended.\"; sleep 5; $menucmd -1\0" \
	"bootmenu_7=Reboot=reset\0" \
	"bootmenu_delay=30\0" \
	"menucmd=bootmenu\0" \
	"uuid_gpt_esp=" UUID_GPT_ESP "\0" \
	"uuid_gpt_system=" UUID_GPT_SYSTEM "\0" \
	"partitions=" PARTS_DEFAULT "\0" \
	BOOTENV
#endif

#include <configs/meson64.h>

#endif /* __CONFIG_H */
