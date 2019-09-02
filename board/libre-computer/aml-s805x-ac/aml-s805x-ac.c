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

int misc_init_r(void)
{
	u8 mac_addr[EFUSE_MAC_SIZE];
	ssize_t len;
	
	meson_eth_init(PHY_INTERFACE_MODE_RMII,
		       MESON_USE_INTERNAL_RMII_PHY);
	
	meson_set_boot_source();

	char chip_id[12];
	char mac_addr_rand[17];
	
	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
		len = meson_sm_read_efuse(EFUSE_MAC_OFFSET,
					  mac_addr, EFUSE_MAC_SIZE);
		if (len == EFUSE_MAC_SIZE && is_valid_ethaddr(mac_addr)){
			eth_env_set_enetaddr("ethaddr", mac_addr);
			printf("Net:   eth0 efuse MAC address: %s\n", mac_addr_rand);
		} else {
			meson_sm_get_chip_id(chip_id);
			sprintf(mac_addr_rand,"18:66:c7:%02x:%02x:%02x",chip_id[9],chip_id[10],chip_id[11]);
			env_set("ethaddr", mac_addr_rand);
			printf("Net:   eth0 chip_id MAC address: %s\n", mac_addr_rand);
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
};

int splash_screen_prepare(void)
{
        return splash_source_load(meson_splash_locations,
                                  ARRAY_SIZE(meson_splash_locations));
}
#endif

int meson_board_late_init(void)
{
	
	bmp_display((ulong)libre_computer_bmp_gz,BMP_ALIGN_CENTER, BMP_ALIGN_CENTER);	
	
#ifdef CONFIG_SPLASH_SCREEN
        int x = 0, y = 0, ret;
        ret = splash_screen_prepare();
        if (ret == 0){
                splash_get_pos(&x, &y);
        	char *env_splashimage_value = env_get("splashimage");
                bmp_display(simple_strtoul(env_splashimage_value, 0, 16), x, y);
        }
#endif

        return 0;
}

#if defined(CONFIG_LAST_STAGE_INIT)
int last_stage_init(void)
{
	return 0;
	
}
#endif

#if defined(CONFIG_OF_LIBFDT)
#if defined(CONFIG_OF_BOARD_SETUP)
int meson_ft_board_setup(void *blob, bd_t *bd)
{
	int node_offset;
	char * env_get_variable;
	char * local_var;
	const char * okay = "okay";
	char subsystem[4], subsystem_dev[2], subsystem_dev_subdev[4];

	local_var = get_local_var("devtype");
	if (local_var != NULL){
		sprintf((char *) subsystem, "%s", local_var);
		//printf("devtype: %s\n", (char *)subsystem);
		local_var = get_local_var("devnum");
		if (local_var != NULL){
			sprintf((char *) subsystem_dev, "%s", local_var);
			//printf("devnum: %s\n", (char *)subsystem_dev);
			local_var = get_local_var("distro_bootpart");
			if (local_var != NULL){
				sprintf((char *) subsystem_dev_subdev, "%s:%s", subsystem_dev, local_var);
				//printf("subsystem_dev_subdev %s\n", (char *) subsystem_dev_subdev);
			}

			char * do_load_ini_argv[5] = { NULL, subsystem, subsystem_dev_subdev, __stringify(CONFIG_SYS_LOAD_ADDR), "boot.ini" };
			char * do_ini_argv[2] = { NULL, "" };
			if (do_load(NULL, 0, 5, do_load_ini_argv,FS_TYPE_ANY) == 0)
				if (do_ini(NULL, 0, 2, do_ini_argv) == 0)
					printf("boot.ini: loaded\n");
		}
	}

	env_get_variable = env_get("cvbs");
	if (env_get_variable != NULL && strcmp(env_get_variable,"1") == 0){
		node_offset = fdt_path_offset(blob, "/cvbs-connector");
		if (node_offset < 0){
			printf("cvbs: unable to find dt node\n");
		} else {
			if (fdt_setprop(blob, node_offset, "status", okay, strlen(okay) + 1) < 0)
				printf("cvbs: failed to enable output\n");
			else
				printf("cvbs: enabled\n");
		}
	} else {
		printf("cvbs: disabled\n");
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
	
	return 0;
}
#endif
#if defined(CONFIG_OF_SYSTEM_SETUP)
int ft_system_setup(void *blob, bd_t *bd){
	return 0;
}
#endif
#endif

