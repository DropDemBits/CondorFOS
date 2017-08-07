/*
 * Copyright (C) 2017 DropDemBits <r3usrlnd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <condor.h>
#include <kernel/ata_portio.h>

#ifndef _ATA_H
#define _ATA_H

#define ATA_DEVICE_INVALID 0xFFFF

#define ATA_COMMAND_TYPE_READ 0
#define ATA_COMMAND_TYPE_WRITE 1
#define ATA_COMMAND_TYPE_MISC 2

typedef struct {
	//Feature bits
	uword_t command_size : 2;
    uword_t incomplete : 1;
	uword_t reserved0 : 2;
	uword_t drq_mode : 2;
	uword_t removable_media : 1;
	uword_t scsi_set : 5;
	uword_t reserved1 : 1;
	uword_t ata_device : 2;

	uword_t reserved2[8];

	char serial_num[20];
	uword_t reserved3[3];
	char firmware_rev[8];
	char model_num[40];

	uword_t reserved4[2];

	//Capabilities
	uword_t vendor_specifics0 : 8;
	uword_t dma_support : 1;
	uword_t lba_support : 1;
	uword_t iordy_disable : 1;
	uword_t iordy_support : 1;
	uword_t ata_srst_required : 1;
	uword_t overlap_op : 1;
	uword_t command_queue_support : 1;
	uword_t interleaved_dma_support : 1;

	uword_t reserved5;
	uword_t vendor_specifics1 : 8;
	uword_t pio_transfer_mode : 1;
	//Valid Field (Groups)
	uword_t fields_54_58_valid : 1;
	uword_t fields_64_70_valid : 1;
	uword_t field_88_valid : 1;
	uword_t reserved6 : 13;

	uword_t reserved7[8];

    //Can't be bothered to do packet only udma

	//MDMA
	uword_t mdma0_support : 1;
	uword_t mdma1_support : 1;
	uword_t mdma2_support : 1;
	uword_t reserved8 : 5;
	uword_t mdma0_select : 1;
	uword_t mdma1_select : 1;
	uword_t mdma2_select : 1;

	uword_t reserved9 : 5;

	uword_t apio_mode_support : 8;

	uword_t vendor_specifics2 : 8;
	uword_t min_mdma_cycle_time;
	uword_t rec_mdma_cycle_time;
	uword_t min_pio_cycle_time;
	uword_t min_pio_cycle_time_fc;
	uword_t reserved10[2];
	uword_t release_response_time;
	uword_t bsy_clear_time;

	uword_t reserved11[2];

	uword_t time_packet_recieve_to_bus_release;
	uword_t time_service_to_bsy_clear;
	uword_t reserved_packet[2];

	//Queue Depth
	uword_t max_queue_depth : 5;
	uword_t reserved12 : 11;

	uword_t reserved13[4];

	//ATA Version
	uword_t reserved14 : 1;
	uword_t major_version : 15;
	uword_t minor_version;

	//Command set support
	uword_t smart_support : 1;
	uword_t secure_mode_support : 1;
	uword_t removable_media_support : 1;
	uword_t powerman_support : 1;
	uword_t packet_support : 1;
	uword_t write_cache_support : 1;
	uword_t lookahead_support : 1;
	uword_t release_int_support : 1;
	uword_t service_int_support : 1;
	uword_t drv_reset_support : 1;
	uword_t host_protected_area_support : 1;
	uword_t obsolete0 : 1;
	uword_t write_buffer_support : 1;
	uword_t read_buffer_support : 1;
	uword_t nop_support : 1;
	uword_t obsolete1 : 1;

	uword_t download_microcode_support : 1;
	uword_t rw_dma_queued_support : 1;
	uword_t cfa_feature_set_support : 1;
	uword_t apm_feature_set_support : 1;
	uword_t rm_media_notify_support : 1;
	uword_t powerup_standby_feature_set_support : 1;
	uword_t set_features_to_spinup : 1;
	uword_t reserved16 : 1;
	uword_t set_max_extension_support : 1;
	uword_t automatic_am_feature_set_support : 1;
	uword_t lba48_feature_set_support : 1;
	uword_t device_config_overlay_feature_set_support : 1;
	uword_t flush_cache_command_support : 1;
	uword_t flush_cache_ext_command_support : 1;
	uword_t one0 : 1;
	uword_t zero0 : 1;

	//Command set/feature support extension
	uword_t reserved17 : 14;
	uword_t one1 : 1;
	uword_t zero1 : 1;

	//Command set/Feature enable
	uword_t smart_enabled : 1;
	uword_t secure_mode_enabled : 1;
	uword_t removable_media_enabled : 1;
	uword_t powerman_enabled : 1;
	uword_t packet_enabled : 1;
	uword_t write_cache_enabled : 1;
	uword_t lookahead_enabled : 1;
	uword_t release_int_enabled : 1;
	uword_t service_int_enabled : 1;
	uword_t drv_reset_enabled : 1;
	uword_t host_protected_area_enabled : 1;
	uword_t obsolete2 : 1;
	uword_t write_buffer_enabled : 1;
	uword_t read_buffer_enabled : 1;
	uword_t nop_enabled : 1;
	uword_t obsolete3 : 1;
	uword_t download_microcode_enabled : 1;
	uword_t reserved18 : 3;
	uword_t rm_media_notify_enabled : 1;
	uword_t reserved19 : 9;
	uword_t one4 : 1;
	uword_t zero4 : 1;

	//Command set/feature support enabled
	uword_t reserved20 : 14;
	uword_t one5 : 1;
	uword_t zero5 : 1;

	//UDMA
	uword_t udma0_support : 1;
	uword_t udma1_support : 1;
	uword_t udma2_support : 1;
	uword_t udma3_support : 1;
	uword_t udma4_support : 1;
	uword_t udma5_support : 1;
	uword_t udma6_support : 1;
	uword_t reserved21 : 1;
	uword_t udma0_selected : 1;
	uword_t udma1_selected : 1;
	uword_t udma2_selected : 1;
	uword_t udma3_selected : 1;
	uword_t udma4_selected : 1;
	uword_t udma5_selected : 1;
	uword_t udma6_selected : 1;
    uword_t reserved22 : 1;
	uword_t reserved23[37];

	uword_t rm_media_status_notify_support : 2;
	uword_t reserved24 : 14;

	//Security Status
	uword_t security_support : 1;
	uword_t security_enable : 1;
	uword_t security_locked : 1;
	uword_t security_frozen : 1;
	uword_t security_count_expire : 1;
	uword_t security_erase_support : 1;
	uword_t reserved25 : 2;
	uword_t security_level : 1;
	uword_t reserved26 : 7;

	uword_t vendor_specifics3[30];
	uword_t reserved27[95];
} ata_info_t;

/* Port Offsets */
#define ATA_DATA 0
#define ATA_FEATURES 1
#define ATA_SECT_COUNT 2
#define ATA_LBALO 3
#define ATA_LBAMI 4
#define ATA_LBAHI 5
#define ATA_DEVICE_SEL 6
#define ATA_COMMAND 7
#define ATA_STATUS_ALT 0x206

/* Alternate Names */
#define ATA_INT_REASON ATA_FEATURES
#define ATA_BYTELO ATA_LBAMI
#define ATA_BYTEHI ATA_LBAHI
#define ATA_STATUS ATA_COMMAND
#define ATA_DEVICE_CONTROL ATA_STATUS_ALT

/* 65535 ATA Controllers should be enough for anybody */
#define ATA_MAX_DEVICES 65535

typedef enum {
	NONE,
	ATA,
	ATAPI,
	SATA,
	SATAPI,
} ata_interface_type_t;

//ATA Commands
enum {
	ATA_IDENTIFY_DEVICE = 0xEC,
} ATA_COMMANDS;

//SCSI/ATAPI Commands
enum {
	ATAPI_IDENTIFY_PACKET_DEVICE = 0xA1,
	ATAPI_READ_6=0x08,
	ATAPI_READ_10=0x28,
	ATAPI_READ_12=0xA8,
	ATAPI_READ_16=0x88,
	ATAPI_WRITE_6=0x0A,
	ATAPI_WRITE_10=0x2A,
	ATAPI_WRITE_12=0xAA,
	ATAPI_WRITE_16=0x8A,
} ATAPI_COMMANDS;

inline ubyte_t atapi_getCommandType(ubyte_t command) {
    if((command & ATAPI_READ_6) == ATAPI_READ_6) return ATA_COMMAND_TYPE_READ;
    if((command & ATAPI_WRITE_6) == ATAPI_WRITE_6) return ATA_COMMAND_TYPE_WRITE;
    return ATA_COMMAND_TYPE_MISC;
}

/*
 * Initializes an IDE controller
 * @param base_port Base port of controller
 * @param alt_port Alternate status port of controller
 * @param irq IRQ used for controller
 */
uword_t ide_init(uword_t base_port, uword_t alt_port, ubyte_t irq);

void* ata_readSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination);

void ata_writeSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* source);

#endif
