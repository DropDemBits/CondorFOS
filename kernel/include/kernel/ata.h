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

#include <stdbool.h>

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
#define ATA_INT_REASON ATA_SECT_COUNT
#define ATA_ERROR ATA_FEATURES
#define ATA_BYTELO ATA_LBAMI
#define ATA_BYTEHI ATA_LBAHI
#define ATA_STATUS ATA_COMMAND
#define ATA_DEVICE_CONTROL ATA_STATUS_ALT

/* 65535 ATA Controllers should be enough for anybody */
#define ATA_MAX_DEVICES 65535

typedef enum {
	NONE,
	ATA = 0b100,
	SATA = 0b101,
	ATAPI = 0b110,
	SATAPI = 0b111,
} ata_interface_type_t;

//ATA Commands (Created using ATA/ATAPI-6 & ACS-3)
enum {
	// General feature set
	ATA_IDENTIFY_DEVICE = 0xEC,
	ATA_EXECUTE_DEVICE_DIAGNOSTIC = 0x90,
	ATA_SET_FEATURES = 0xEF,
	ATA_DATA_SET_MANAGEMENT = 0x06,
	ATA_DOWNLOAD_MICROCODE = 0x92,
	ATA_DOWNLOAD_MICROCODE_DMA = 0x93,
	ATA_FLUSH_CACHE = 0xE7,
	ATA_NOP = 0x00,
	ATA_SET_DATE_TIME_EXT = 0x77,
	ATA_SET_MULTIPLE_MODE = 0xC6,
	ATA_READ_BUFFER = 0xE4,
	ATA_READ_BUFFER_DMA = 0xE9,
	ATA_READ_DMA = 0xC8,
	ATA_READ_MULTIPLE = 0xC4,
	ATA_READ_SECTORS = 0x20,
	ATA_READ_VERIFY_SECTORS = 0x40,
	ATA_WRITE_BUFFER = 0xE8,
	ATA_WRITE_BUFFER_DMA = 0xEB,
	ATA_WRITE_DMA = 0xCA,
	ATA_WRITE_MULTIPLE = 0xC5,
	ATA_WRITE_SECTORS = 0x30,
	ATA_WRITE_UNCORRECTABLE_EXT = 0x45,

	// Packet feature set
	ATA_DEVICE_RESET = 0x08,
	ATA_PACKET = 0xA0,
	ATA_IDENTIFY_PACKET_DEVICE = 0xA1,
	ATA_SEEK = 0x70, // Obsolete ATA7

	// Overlapped feature set (Obsolete ACS2)
	ATA_READ_DMA_QUEUED = 0xC7,
	ATA_READ_DMA_QUEUED_EXT = 0x26,
	ATA_WRITE_DMA_QUEUED = 0xCC,
	ATA_WRITE_DMA_QUEUED_EXT = 0x36,
	ATA_SERVICE = 0xA2,

	// 48bit Address feature set (Includes EXT suffixed commands)
	ATA_FLUSH_CACHE_EXT = 0xEA,
	ATA_READ_DMA_EXT = 0x25,
	ATA_READ_MULTIPLE_EXT = 0x29,
	ATA_READ_SECTORS_EXT = 0x24,
	ATA_READ_VERIFY_SECTORS_EXT = 0x42,
	ATA_WRITE_DMA_EXT = 0x35,
	ATA_WRITE_DMA_FUA_EXT = 0x3D,
	ATA_WRITE_MULTIPLE_EXT = 0x39,
	ATA_WRITE_MULTIPLE_FUA_EXT = 0xCE,
	ATA_WRITE_SECTORS_EXT = 0x34,

	// Accessible Max Address Configuation feature set
	ATA_ACCESSIBLE_MAX_ADDRESS_CONFIGURATION = 0x78,
	ATA_AMAC_GET_NATIVE_MAX_ADDRESS_EXT = 0x00,
	ATA_AMAC_SET_MAX_ADDRESS_EXT = 0x01,
	ATA_AMAC_FREEZE_ACCESSIBLE_MAX_ADDRESS_EXT = 0x02,

	// CFA feature set
	ATA_CFA_REQUEST_EXTENDED_ERROR = 0x03,
	ATA_CFA_WRITE_SECTORS_NO_ERASE = 0x38,
	ATA_CFA_WRITE_MULTIPLE_NO_ERASE = 0xCD,
	ATA_CFA_TRANSLATE_SECTOR = 0x87,
	ATA_CFA_ERASE_SECTORS = 0xC0,

	// General Purpose Logging (GPL) feature set
	ATA_READ_LOG_EXT = 0x2F,
	ATA_READ_LOG_DMA_EXT = 0x47,
	ATA_WRITE_LOG_EXT = 0x3F,
	ATA_WRITE_LOG_DMA_EXT = 0x57,

	// Native Command Queuing (NCQ) feature set
	ATA_READ_FPDMA_QUEUED = 0x60,
	ATA_WRITE_FPDMA_QUEUED = 0x61,
	ATA_NCQ_QUEUE_MANAGEMENT = 0x63,
	ATA_SEND_FPDMA_QUEUED = 0x64,
	ATA_RECEIVE_FPDMA_QUEUED = 0x65,

	// Power Management feature set
	ATA_CHECK_POWER_MODE = 0xE5,
	ATA_IDLE = 0xE3,
	ATA_IDLE_IMMEDIATE = 0xE1,
	ATA_SLEEP = 0xE6,
	ATA_STANDBY = 0xE2,
	ATA_STANDBY_IMMEDIATE = 0xE0,

	// Sanitize Device feature set
	ATA_SANATIZE_DEVICE = 0xB4,
	ATA_SANATIZE_STATUS_EXT = 0x00,
	ATA_CRYPTO_SCRAMBLE_EXT = 0x11,
	ATA_BLOCK_ERASE_EXT = 0x12,
	ATA_OVERWRITE_EXT = 0x14,
    ATA_SANATIZE_FREEZE_LOCK_EXT = 0x20,
    ATA_SANATIZE_ANTIFREEZE_LOCK_EXT = 0x20,

	// Security feature set
	ATA_SECURITY_SET_PASSWORD = 0xF1,
	ATA_SECURITY_UNLOCK = 0xF2,
	ATA_SECURITY_ERASE_PREPARE = 0xF3,
	ATA_SECURITY_ERASE_UNIT = 0xF4,
	ATA_SECURITY_FREEZE_LOCK = 0xF5,
	ATA_SECURITY_DISABLE_PASSWORD = 0xF6,

	// SMART feature set
	ATA_SMART = 0xB0,
    ATA_SMART_READ_DATA = 0xD0,
    ATA_SMART_ENABLE_DISABLE_AUTOSAVE = 0xD2,
    ATA_SMART_EXECUTE_OFF_LINE = 0xD4,
    ATA_SMART_READ_LOG_SECTOR = 0xD5,
    ATA_SMART_WRITE_LOG_SECTOR = 0xD6,
    ATA_SMART_ENABLE_OPERATIONS = 0xD8,
	ATA_SMART_DISABLE_OPERATIONS = 0xD9,
	ATA_SMART_RETURN_STATUS = 0xDA,

	// Sense Data Reporting feature set
	ATA_REQUEST_SENSE_DATA_EXT = 0x0B,

	// Streaming feature set
	ATA_CONFIGURE_STREAM = 0x51,
	ATA_READ_STREAM_PIO = 0x2B,
	ATA_READ_STREAM_DMA = 0x2A,
	ATA_WRITE_STREAM_PIO = 0x3B,
	ATA_WRITE_STREAM_DMA = 0x3A,

	ATA_READ_STREAM_EXT = 0x2B,
	ATA_READ_STREAM_DMA_EXT = 0x2A,
	ATA_WRITE_STREAM_EXT = 0x3B,
	ATA_WRITE_STREAM_DMA_EXT = 0x3A,

	// Trusted Computing feature set
	ATA_TRUSTED_NON_DATA = 0x5B,
	ATA_TRUSTED_SEND = 0x5E,
	ATA_TRUSTED_SEND_DMA = 0x5F,
	ATA_TRUSTED_RECEIVE = 0x5C,
	ATA_TRUSTED_RECEIVE_DMA = 0x5D,

	// Device Configuration Overlay feature set (Obsolete ACS3)
	ATA_DEVICE_CONFIGURATION = 0xB1,
    ATA_DEVICE_CONFIGURATION_RESTORE = 0xC0,
	ATA_DEVICE_CONFIGURATION_FREEZE_LOCK = 0xC1,
	ATA_DEVICE_CONFIGURATION_IDENTIFY = 0xC2,
    ATA_DEVICE_CONFIGURATION_SET = 0xC3,

	// Media Card Pass Through Command feature set
	ATA_CHECK_MEDIA_CARD_TYPE = 0xD1, // Obsolete ACS2
	//SD Card ATA Command Extension
	ATA_SD_ACE_D2 = 0xD2,
	ATA_SD_ACE_D3 = 0xD3,
	ATA_SD_ACE_D4 = 0xD4,
	//Smart Media ATA Command Extension
	ATA_SM_ACE_D2 = 0xD2,
	ATA_SM_ACE_D3 = 0xD3,
	ATA_SM_ACE_D4 = 0xD4,

	// Removeable Media Status Notification feature set (Obsolete ATA8)
	ATA_GET_MEDIA_STATUS = 0xDA,

	// Removeable Media feature set (Obsolete ATA8)
	ATA_MEDIA_EJECT = 0xED,
	ATA_MEDIA_LOCK = 0xDE,
	ATA_MEDIA_UNLOCK = 0xDF,

	// Host Protected Area feature set (Obsolete ACS3)
	ATA_SET_MAX_ADDRESS = 0xF9,
	ATA_SET_MAX_ADDRESS_EXT = 0x37,
	ATA_READ_NATIVE_MAX_ADDRESS = 0xF8,
	ATA_READ_NATIVE_MAX_ADDRESS_EXT = 0x27,

	// SET FEATURE Codes
	SET_FEATURE_ENABLE_8BIT_TRANSFERS = 0x01, // Retired ATA4, Reserved for CFA ATA5+
	SET_FEATURE_ENABLE_WRITE_CACHE = 0x02,
	SET_FEATURE_SET_TRANSFER_MODE = 0x03,
	SET_FEATURE_ENABLE_AUTOMATIC_DEFECT_REASSIGNMENT = 0x04, // Obsolete ATA4
	SET_FEATURE_ENABLE_APM = 0x05,
	SET_FEATURE_ENABLE_PUIS_FEATURE_SET = 0x06, // Introduced ATA5
	SET_FEATURE_PUIS_FEATURE_SET_DEVICE_SPIN_UP = 0x07,
	SET_FEATURE_ADDR_OFFSET_BOOT_AREA_METHOD_REPORT0 = 0x09, // Obsolete ACS3
	SET_FEATURE_ENABLE_CFA_POWER_MODE1 = 0x0A, // Reserved for CFA
	SET_FEATURE_ENABLE_SATA = 0x10, // Introduced ATA8 / ACS1
	SET_FEATURE_DISABLE_MEDIA_STATUS_NOTIFICATION = 0x31, // Obsolete ATA8
	SET_FEATURE_DISABLE_RETRY = 0x33, // Obsolete ATA5
	SET_FEATURE_ENABLE_FREE_FALL_CONTROL_FEATURE_SET = 0x41,
	SET_FEATURE_ENABLE_AAM_FEATURE_SET = 0x42, // Obsolete ACS2
	SET_FEATURE_SET_MAX_HOST_INTERFACE_SECTOR_TIMES = 0x43,
	SET_FEATURE_VENDOR_LENGTH_OF_EEC_OF_LONG_RW_COMMANDS = 0x44, // Obsolete ATA4
	SET_FEATURE_EPC = 0x4A, // Introduced ACS2
	SET_FEATURE_SET_CACHE_SEGMENTS_TO_COUNT = 0x54, // Obsolete ATA4
	SET_FEATURE_DISABLE_READ_LOOKAHEAD_FEATURE = 0x55,
	SET_FEATURE_ENABLE_RELEASE_INTERRUPT = 0x5D, // Obsolete ACS2
	SET_FEATURE_ENABLE_SERVICE_INTERRUPT = 0x5E, // Obsolete ACS2
	SET_FEATURE_DISABLE_REVERT_TO_POWER_ON_DEFAULTS = 0x66,
	SET_FEATURE_LONG_PHYSICAL_SECTOR_ALIGN_ERROR_REPORT_CTRL = 0x69, // Introduced ACS2
	SET_FEATURE_DISABLE_ECC = 0x77, // Obsolete ATA4

	SET_FEATURE_DISABLE_8BIT_TRANSFERS = 0x81, // Same status as enable
	SET_FEATURE_DISABLE_WRITE_CACHE = 0x82,
	SET_FEATURE_DISABLE_AUTOMATIC_DEFECT_REASSIGNMENT = 0x84, // Obsolete ATA4
	SET_FEATURE_DISABLE_APM = 0x85,
	SET_FEATURE_DISABLE_PUIS_FEATURE_SET = 0x86, // Introduced ATA5
	SET_FEATURE_ENABLE_ECC = 0x88, // Obsolete ATA6
	SET_FEATURE_ADDR_OFFSET_BOOT_AREA_METHOD_REPORT1 = 0x89,
	SET_FEATURE_DISABLE_CFA_POWER_MODE1 = 0x8A, // Reserved for CFA
	SET_FEATURE_DISABLE_SATA = 0x90, // Introduced ATA8 / ACS1
	SET_FEATURE_ENABLE_MEDIA_STATUS_NOTIFICATION = 0x95, // Obsolete ATA8 / ACS1
	SET_FEATURE_ENABLE_RETRIES = 0x99, // Obsolete ATA4
	SET_FEATURE_SET_DEVICE_MAX_AVERAGE_CURRENT = 0x9A, // Obsolete ATA4
	SET_FEATURE_ENABLE_READ_LOOKAHEAD_FEATURE = 0xAA,
	SET_FEATURE_SET_MAX_PREFETCH_TO_COUNT = 0xAB,
	SET_FEATURE_4BYTES_ECC_ON_LONG_RW_COMMANDS = 0xBB,
	SET_FEATURE_DISABLE_FREE_FALL_CONTROL_FEATURE_SET = 0xC1,
	SET_FEATURE_DISABLE_AAM_FEATURE_SET = 0xC2, // Obsolete ACS2
	SET_FEATURE_TOGGLE_SENSE_DATA_REPORTING_FEATURE_SET = 0xC3,
	SET_FEATURE_ENABLE_REVERT_TO_POWER_ON_DEFAULTS = 0xCC,
	SET_FEATURE_DISABLE_RELEASE_INTERRUPT = 0xDD, // Obsolete ACS2
	SET_FEATURE_DISABLE_SERVICE_INTERRUPT = 0xDE, // Obsolete ACS2

} ATA_COMMANDS;

//SCSI/ATAPI Commands
enum {
	// Packet Commands
	ATAPI_READ_6=0x08,
	ATAPI_READ_10=0x28,
	ATAPI_READ_12=0xA8,
	ATAPI_READ_16=0x88,
	ATAPI_WRITE_6=0x0A,
	ATAPI_WRITE_10=0x2A,
	ATAPI_WRITE_12=0xAA,
	ATAPI_WRITE_16=0x8A,
} ATAPI_COMMANDS;

typedef struct ata_device {
    // Configuration
    uword_t dev_id;
    ubyte_t irq;
    uword_t base_port;
    uword_t alt_port;
    bool is_slave;

    // Info
    ata_info_t* ata_info;
    ata_interface_type_t interface_type : 3;
    uword_t lba_support : 1;
    uword_t dma_support : 1;
    uword_t ata_version : 4;
    uword_t mdma_support : 2;
    uword_t udma_support : 3;
    uword_t lba48_support : 1;
    uword_t reserved : 1;
    ubyte_t max_command_length : 2;

    // Current State
    bool in_use;
    volatile bool is_blocked;
    volatile bool has_errored;
    ubyte_t error_info;
} ata_device_t;

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

ata_device_t* ata_getDeviceInfo(uword_t device);

bool ata_sendCommand(uword_t device, ubyte_t command, ubyte_t subcommand, void* data, size_t data_length);

bool ata_readSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination);

bool ata_writeSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* source);

bool atapi_sendCommand(uword_t device, ubyte_t *command, ubyte_t command_length, void* data);

bool atapi_readSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination);

bool atapi_writeSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination);

#endif
