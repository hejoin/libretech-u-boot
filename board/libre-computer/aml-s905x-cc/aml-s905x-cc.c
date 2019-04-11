// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Da Xue
 * Author: Da Xue <da@libre.computer>
 */

#include <common.h>
#include <dm.h>
#include <environment.h>
#include <asm/io.h>
#include <asm/arch/gx.h>
#include <asm/arch/sm.h>
#include <asm/arch/eth.h>
#include <asm/arch/mem.h>
#include <fs.h>
#include <dm/uclass-internal.h>
#include <lcd.h>
#include <led.h>
#include <splash.h>
#include <libre-computer_logo.h>
#include <linux/libfdt_env.h>
#include <asm/arch/boot.h>

#define EFUSE_MAC_OFFSET	52
#define EFUSE_MAC_SIZE		6
#define	LC_LED_CPU		"librecomputer:blue:cpu"
#define	LC_LED_DISK		"librecomputer:green:disk"
#define	LC_EMMC_4X_MAX_FREQ	50000000
#define LC_EMMC_4X_VOLTAGE	3300000

int misc_init_r(void)
{
	u8 mac_addr[EFUSE_MAC_SIZE];
	ssize_t len;
	struct udevice *dev;
	
	led_get_by_label(LC_LED_CPU, &dev);
	led_set_state(dev, LEDST_OFF);
	led_get_by_label(LC_LED_DISK, &dev);
	led_set_state(dev, LEDST_ON);

	meson_eth_init(PHY_INTERFACE_MODE_RMII,
		       MESON_USE_INTERNAL_RMII_PHY);
	
	meson_set_boot_source();
	env_load();
	meson_set_boot_source();

	char * do_load_ini_argv[5] = { NULL, "mmc", env_get("bootdevice"), CONFIG_SYS_LOAD_ADDR, "u-boot.ini" };
	char * do_ini_argv[2] = { NULL, "" };
	if (do_load(NULL, 0, 5, do_load_ini_argv,FS_TYPE_ANY) == 0)
		do_ini(NULL, 0, 2, do_ini_argv);

	char chip_id[12];
	char mac_addr_rand[17];
	
	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
		len = meson_sm_read_efuse(EFUSE_MAC_OFFSET,
					  mac_addr, EFUSE_MAC_SIZE);
		if (len == EFUSE_MAC_SIZE && is_valid_ethaddr(mac_addr)){
			eth_env_set_enetaddr("ethaddr", mac_addr);
		} else {
			meson_sm_get_chip_id(chip_id);
			sprintf(mac_addr_rand,"18:66:C7:%02x:%02x:%02x",chip_id[9],chip_id[10],chip_id[11]);
			env_set("ethaddr", mac_addr_rand);
			printf("using random mac address: %s\n", mac_addr_rand);
		}
	}

	return 0;
}

#ifdef CONFIG_SPLASH_SCREEN
static struct splash_location meson_splash_locations[] = {
        {
                .name = "mmc_fs",
                .storage = SPLASH_STORAGE_MMC,
                .flags = SPLASH_STORAGE_FS,
                .devpart = "0:1",
        },
        {
                .name = "mmc_fs",
                .storage = SPLASH_STORAGE_MMC,
                .flags = SPLASH_STORAGE_FS,
                .devpart = "1:1",
        },
};

int splash_screen_prepare(void)
{
        return splash_source_load(meson_splash_locations,
                                  ARRAY_SIZE(meson_splash_locations));
}
#endif

int meson_board_late_init(void)
{
        int x = 0, y = 0, ret;
	struct udevice *dev;
	
	led_get_by_label(LC_LED_CPU, &dev);
	led_set_state(dev, LEDST_ON);
	led_get_by_label(LC_LED_DISK, &dev);
	led_set_state(dev, LEDST_OFF);

	bmp_display((ulong)libre_computer_bmp_gz,BMP_ALIGN_CENTER, BMP_ALIGN_CENTER);

        ret = splash_screen_prepare();
        if (ret == 0){
                splash_get_pos(&x, &y);
        	char *env_splashimage_value = env_get("splashimage");
                bmp_display(simple_strtoul(env_splashimage_value, 0, 16), x, y);
        }
        return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_SYSTEM_SETUP)
int ft_system_setup(void *blob, bd_t *bd)
{
	char * do_adc_range_argv[6] = { "range", "adc@8680", "1", "970", "1010", "100" };
	int node_offset;
	char * env_get_variable;
	struct udevice *dev;
	led_get_by_label(LC_LED_CPU, &dev);
	led_set_state(dev, LEDST_OFF);
	led_get_by_label(LC_LED_DISK, &dev);
	led_set_state(dev, LEDST_ON);

	if (do_adc_range(NULL,NULL,6,do_adc_range_argv) == 0){
		printf("sd_uhs: not supported\n");
	} else {
		env_get_variable = env_get("sd_uhs");
		if (env_get_variable != NULL && strcmp(env_get_variable,"1") == 0){
			node_offset = fdt_path_offset(blob, "/soc/apb/mmc@72000");
			if (node_offset < 0){
				printf("sd_uhs: unable to find dt node\n");
			} else {
				if (fdt_setprop(blob,node_offset,"sd-uhs-ddr50",NULL,0) == 0)
					printf("sd_uhs: added ddr50 support\n");
				else
					printf("sd_uhs: failed to add ddr50 support\n");
				if (fdt_setprop(blob,node_offset,"sd-uhs-sdr50",NULL,0) == 0)
					printf("sd_uhs: added sdr50 support\n");
				else
					printf("sd_uhs: failed to add sdr50 support\n");
			}
		} else {
			printf("sd_uhs: not enabled\n");
		}
	}

	env_get_variable = env_get("cvbs");
	if (env_get_variable != NULL && strcmp(env_get_variable,"0") == 0){
		node_offset = fdt_path_offset(blob, "/cvbs-connector");
		if (node_offset < 0){
			printf("cvbs: unable to find dt node\n");
		} else {
			if (fdt_del_node(blob, node_offset) == 0)
				printf("cvbs: disabled\n");
			else
				printf("cvbs: failed to disable output\n");
		}
	} else {
		printf("cvbs: enabled\n");
	}

	env_get_variable = env_get("efuse");
	if (env_get_variable != NULL && strcmp(env_get_variable,"rw") == 0){
		node_offset = fdt_path_offset(blob, "/efuse");
		if (node_offset < 0){
			printf("efuse: unable to find dt node\n");
		} else {
			if (fdt_delprop(blob, node_offset, "read-only") == 0)
				printf("efuse: enabled read-write\n");
			else
				printf("efuse: failed to enable read-write\n");
		}
	} else {
		printf("efuse: read-only\n");
	}
	
	env_get_variable = env_get("emmc");
	if (env_get_variable != NULL && strcmp(env_get_variable,"4.x") == 0){
		node_offset = fdt_path_offset(blob, "/soc/apb@d0000000/mmc@74000");
		if (node_offset < 0){
			printf("emmc: unable to find dt node\n");
		} else {
			if (fdt_delprop(blob, node_offset, "mmc-ddr-1_8v") != 0)
				printf("emmc: failed to remove ddr 1.8v support\n");
			if (fdt_delprop(blob, node_offset, "mmc-hs200-1_8v") != 0)
				printf("emmc: failed to remove hs200 1.8v support\n");
			if (fdt_setprop(blob, node_offset, "mmc-ddr-3_3v",NULL,0) != 0)
				printf("mmc: failed to add ddr 3.3v support\n");
			if (fdt_setprop_u32(blob, node_offset, "max-frequency", LC_EMMC_4X_MAX_FREQ) != 0)
				printf("mmc: failed to reduce maximum frequency to 50MHz\n");
		}
		node_offset = fdt_path_offset(blob, "/regulator-vddio_boot");
		if (node_offset < 0){
			printf("emmc: unable to find dt node for regulator\n");
		} else {
			if (fdt_setprop_u32(blob, node_offset, "regulator-min-microvolt", LC_EMMC_4X_VOLTAGE) != 0)
				printf("mmc: failed to lower regulator min voltage\n");
			if (fdt_setprop_u32(blob, node_offset, "regulator-max-microvolt", LC_EMMC_4X_VOLTAGE) != 0)
				printf("mmc: failed to lower regulator max voltage\n");
		}
		printf("emmc: 4.x mode\n");
	} else {
		printf("emmc: 5.x mode\n");
	}
	led_get_by_label(LC_LED_CPU, &dev);
	led_set_state(dev, LEDST_ON);
	return 0;
}
#endif
