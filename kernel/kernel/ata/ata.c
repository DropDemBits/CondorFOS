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

#include <stdio.h>
#include <string.h>

#include <kernel/ata.h>
#include <kernel/irq.h>
#include <kernel/hal.h>
#include <kernel/addrs.h>
#include <io.h>
#include <serial.h>
#include <condor.h>

#define ATA_TYPE_INVALID 0
#define ATA_TYPE_ATA 0b100
#define ATA_TYPE_SATA 0b101
#define ATA_TYPE_ATAPI 0b110
#define ATA_TYPE_SATAPI 0b111

#define DEVICE_INFO_SHIFT 1

#define BIT_SUM2(x) ((x >> 1) + (x & 0x1))
#define BIT_SUM4(x) (BIT_SUM2(x>>1) + BIT_SUM2(x & 0x3))
#define BIT_SUM8(x) (BIT_SUM4(x>>2) + BIT_SUM4(x & 0x7))
#define BIT_SUM16(x) (BIT_SUM8(x>>4) + BIT_SUM8(x & 0xF))
#define BIT_SUM32(x) (BIT_SUM16(x>>8) + BIT_SUM16(x & 0xFF))

//Forward declarations
extern ata_interface_type_t initDevice(uword_t base_port, uword_t alt_port, bool is_slave, void* destination);
extern bool initDevice_ATAPI(uword_t base_port, void* destination);
extern void* getDeviceData(uword_t base, void* destination);
extern int detectDriveType(uword_t base);
extern uword_t switchDevice(uword_t base);
extern irqreturn_t ata_irq();
extern void waitForIRQ(uword_t device);

static uword_t controller_index = 0;
static ata_device_t **controllers = POISON_NULL;
static ubyte_t current_controller = 0;

/**==========================================================================**\
|**                              PUBLIC FUNCTIONS                            **|
\**==========================================================================**/

uword_t ide_init(uword_t base_port, uword_t alt_port, ubyte_t irq)
{
    if(base_port == 0 || alt_port == 0) return ATA_DEVICE_INVALID;

    ata_info_t* data = kmalloc(256*16);
    ata_interface_type_t result_master = ATA_TYPE_INVALID, result_slave = ATA_TYPE_INVALID;
    ata_device_t* device = kmalloc(sizeof(ata_device_t));
    uword_t device_id = ATA_DEVICE_INVALID;

    result_slave = initDevice(base_port, alt_port, true, data);
    result_master = initDevice(base_port, alt_port, false, data);

    if(result_slave != ATA_TYPE_INVALID) result_master = result_slave;
    if(result_master != ATA_TYPE_INVALID) {
        device->dev_id = device_id;
        device->interface_type = result_master;
        device->lba_support = data->lba_support;
        device->dma_support = data->dma_support;
        device->ata_version = (BIT_SUM16(data->major_version));
        device->udma_support = data->udma0_support+data->udma1_support+data->udma2_support+data->udma3_support+data->udma4_support+data->udma5_support+data->udma6_support;
        device->mdma_support = data->mdma0_support+data->mdma1_support+data->mdma2_support;
        if((device->interface_type & 0b010) == 0b000) {
            device->lba48_support = data->lba48_feature_set_support;
        }
        device->base_port = base_port;
        device->alt_port = alt_port;
        device->irq = irq;
        device->is_slave = false;
        device->in_use = false;

        // Set for next IRQ
        device->is_blocked = true;
        irq_addISR(irq, ata_irq);
        device_id = controller_index;
        controller_index += 2;
        if(controllers == POISON_NULL) controllers = NULL; 
        controllers = krealloc(controllers, (controller_index >> 1) * sizeof(ata_device_t*));
        controllers[device_id >> 1] = device;
    }
    else {
        kfree(data);
        kfree(device);
    }

    return device_id;
}

ata_device_t* ata_getDeviceInfo(uword_t device)
{
    if(device == ATA_DEVICE_INVALID || device >= controller_index) return POISON_NULL;
    return controllers[device >> DEVICE_INFO_SHIFT];
}

bool ata_sendCommand(uword_t device, ubyte_t command, ubyte_t subcommand, void* data, size_t data_length)
{
    uword_t base = switchDevice(device);
    if(device == ATA_DEVICE_INVALID
        || data_length == 0
        || base == 0xFFF)
    return false;

    // TODO: Make this a function
    (void) subcommand;
    (void) data;

    outb(base + ATA_COMMAND, command);
    waitForIRQ(device);
    return controllers[device >> DEVICE_INFO_SHIFT]->has_errored;
}

bool ata_readSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination)
{
    if( (lba + sector_count > 0x0FFFFFFF &&
        controllers[device >> DEVICE_INFO_SHIFT]->lba48_support == 0) ||
        sector_count == 0 ||
        controllers[device >> DEVICE_INFO_SHIFT]->in_use)
        return false;
    else if(controllers == POISON_NULL)
        return false;
    else if(device == ATA_DEVICE_INVALID ||
            controllers[device >> DEVICE_INFO_SHIFT]->interface_type == ATA_TYPE_INVALID)
            return false;
    else if(controllers[device >> DEVICE_INFO_SHIFT]->interface_type != ATA)
            return false;

    uword_t base = switchDevice(device);
    uword_t* buffer = (uword_t*) destination;
    if(base == 0) return false;

    // TODO: Turn into proper spinlock
    controllers[device >> DEVICE_INFO_SHIFT]->in_use = true;

    outb(base+ATA_SECT_COUNT, sector_count & 0xFF);
    if(controllers[device >> DEVICE_INFO_SHIFT]->lba48_support == 1) {
        outb(base+ATA_LBALO, (lba >> 24) & 0xFF);
        outb(base+ATA_LBAMI, (lba >> 32) & 0xFF);
        outb(base+ATA_LBAHI, (lba >> 40) & 0xFF);
    }
    outb(base+ATA_LBALO, (lba >> 0) & 0xFF);
    outb(base+ATA_LBAMI, (lba >> 8) & 0xFF);
    outb(base+ATA_LBAHI, (lba >> 16) & 0xFF);
    if(controllers[device >> DEVICE_INFO_SHIFT]->lba48_support == 1) {
        outb(base+ATA_DEVICE_SEL, ((device & 1) << 5) | 0x40);
        outb(base+ATA_COMMAND, ATA_READ_SECTORS_EXT);
    }
    else {
        outb(base+ATA_DEVICE_SEL, ((lba >> 24) & 0xF) | ((device & 1)<<5) | 0x40);
        outb(base+ATA_COMMAND, ATA_READ_SECTORS);
    }
    waitForIRQ(device);

    for(uword_t i = 0; inb(controllers[device >> DEVICE_INFO_SHIFT]->alt_port) & 0x08; i++) {
        buffer[i] = inw(base);
    }

    controllers[device >> DEVICE_INFO_SHIFT]->in_use = false;

    if(controllers[device >> DEVICE_INFO_SHIFT]->has_errored) {
        printf("%d\n", controllers[device >> DEVICE_INFO_SHIFT]->error_info);
        return false;
    }

    return true;
}

bool ata_writeSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* source)
{
    if((lba + sector_count > 0x0FFFFFFF &&
        controllers[device >> DEVICE_INFO_SHIFT]->lba48_support == 0) ||
        sector_count == 0 ||
        controllers[device >> DEVICE_INFO_SHIFT]->in_use)
        return false;
    else if(controllers == POISON_NULL)
        return false;
    else if(device == ATA_DEVICE_INVALID ||
            controllers[device >> DEVICE_INFO_SHIFT]->interface_type == ATA_TYPE_INVALID)
            return false;
    else if(controllers[device >> DEVICE_INFO_SHIFT]->interface_type != ATA)
            return false;

    uword_t base = switchDevice(device);
    uword_t* buffer = (uword_t*) source;
    if(base == 0) return false;

    // TODO: Turn into proper spinlock
    controllers[device >> DEVICE_INFO_SHIFT]->in_use = true;

    outb(base+ATA_SECT_COUNT, sector_count & 0xFF);
    if(controllers[device >> DEVICE_INFO_SHIFT]->lba48_support == 1) {
        outb(base+ATA_LBALO, (lba >> 24) & 0xFF);
        outb(base+ATA_LBAMI, (lba >> 32) & 0xFF);
        outb(base+ATA_LBAHI, (lba >> 40) & 0xFF);
    }
    outb(base+ATA_LBALO, (lba >> 0) & 0xFF);
    outb(base+ATA_LBAMI, (lba >> 8) & 0xFF);
    outb(base+ATA_LBAHI, (lba >> 16) & 0xFF);
    if(controllers[device >> DEVICE_INFO_SHIFT]->lba48_support == 1) {
        outb(base+ATA_DEVICE_SEL, ((device & 1) << 5) | 0x40);
        outb(base+ATA_COMMAND, ATA_WRITE_SECTORS_EXT);
    }
    else {
        outb(base+ATA_DEVICE_SEL, ((lba >> 24) & 0xF) | ((device & 1)<<5) | 0x40);
        outb(base+ATA_COMMAND, ATA_WRITE_SECTORS);
    }

    for(uword_t i = 0; (inb(base+ATA_STATUS) & 0x8); i++) {
        outw(base, buffer[i]);
    }
    if(controllers[device >> DEVICE_INFO_SHIFT]->lba48_support == 1) outb(base+ATA_COMMAND, ATA_FLUSH_CACHE_EXT);
    else outb(base+ATA_COMMAND, ATA_FLUSH_CACHE);

    controllers[device >> DEVICE_INFO_SHIFT]->in_use = false;

    if(controllers[device >> DEVICE_INFO_SHIFT]->has_errored) {
        printf("%d\n", controllers[device >> DEVICE_INFO_SHIFT]->error_info);
        return false;
    }

    return true;
}

bool atapi_sendCommand(uword_t device, ubyte_t *command, ubyte_t command_length, void* data)
{
    if(controllers == POISON_NULL)
        return false;
    uword_t base = switchDevice(device);
    uword_t alt_status = controllers[device >> DEVICE_INFO_SHIFT]->alt_port;

    if(controllers[device >> DEVICE_INFO_SHIFT]->interface_type != ATAPI || 
        controllers[device >> DEVICE_INFO_SHIFT]->in_use) return false;
    else if(base == 0xFFFF) return false;

    controllers[device >> DEVICE_INFO_SHIFT]->in_use = true;

    outb(base+ATA_FEATURES, 0x00);

    //Begin Transaction
    outb(base+ATA_BYTELO, 0x00);
    outb(base+ATA_BYTEHI, 0x01);
    outb(base+ATA_COMMAND, ATA_PACKET);

    while(inb(alt_status) & 0x80 && !(inb(alt_status) & 0x08)) asm("pause");

    outw(base+ATA_DATA, command[0] | command[1] << 8);
    outw(base+ATA_DATA, command[2] | command[3] << 8);
    outw(base+ATA_DATA, command[4] | command[5] << 8);
    if(command_length < 6) goto wait;
    outw(base+ATA_DATA, command[6] | command[7] << 8);
    outw(base+ATA_DATA, command[8] | command[9] << 8);
    outw(base+ATA_DATA, command[10] | command[11] << 8);
    switch(controllers[device >> DEVICE_INFO_SHIFT]->max_command_length) {
        case 0: break;
        case 1:
            size8:
            if(command_length < 16) break;
            outw(base+ATA_DATA, command[12] | command[13] << 8);
            outw(base+ATA_DATA, command[14] | command[15] << 8);
            if(command_length < 32) break;
        case 2:
            if(command_length < 32) goto size8;
            outw(base+ATA_DATA, command[16] | command[17] << 8);
            outw(base+ATA_DATA, command[18] | command[19] << 8);
            outw(base+ATA_DATA, command[20] | command[21] << 8);
            outw(base+ATA_DATA, command[22] | command[23] << 8);
            outw(base+ATA_DATA, command[24] | command[25] << 8);
            outw(base+ATA_DATA, command[26] | command[27] << 8);
            outw(base+ATA_DATA, command[28] | command[29] << 8);
            outw(base+ATA_DATA, command[30] | command[31] << 8);
            break;
    }

    wait:
    waitForIRQ(device);

    uword_t word_count = (inb(base+ATA_BYTELO) | (inb(base+ATA_BYTEHI) << 8)) >> 1;
    uword_t index = 0;

    //Make sure CHK/ERR bit is really what it is
    tryAgain:
    inb(alt_status);
    inb(alt_status);
    inb(alt_status);
    inb(alt_status);

    ubyte_t status = 0;

    for(uword_t words = 0; words < word_count && !((status = inb(alt_status)) & 0x1) && (inb(alt_status) & 0x08); words++, index++) {
        if(atapi_getCommandType(command[0]) == ATA_COMMAND_TYPE_READ) ((uword_t*) data)[index] = inw(base+ATA_DATA);
        else if(atapi_getCommandType(command[0]) == ATA_COMMAND_TYPE_WRITE) outw(base+ATA_DATA, ((uword_t*) data)[index]);
    }

    if(status & 0x01) return false;

    waitForIRQ(device);

    if(inb(alt_status) & 0x08) {
        waitForIRQ(device);
        goto tryAgain;
    }

    controllers[device >> DEVICE_INFO_SHIFT]->in_use = false;
    return true;
}

bool atapi_readSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination)
{
    if(sector_count == 0 || lba > 0x0FFFFFFF || controllers == POISON_NULL)
        return false;
    else if(controllers[device >> DEVICE_INFO_SHIFT]->interface_type != ATAPI)
        return false;
    
    ubyte_t command[] = {ATAPI_READ_12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    command[9] = sector_count;
    command[2] = (lba >> 18) & 0xFF;
    command[3] = (lba >> 10) & 0xFF;
    command[4] = (lba >> 8) & 0xFF;
    command[5] = (lba >> 0) & 0xFF;

    atapi_sendCommand(device, command, LENGTH_OF(command), destination);

    return true;
}

bool atapi_writeSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination)
{
    if(sector_count == 0 || lba > 0x0FFFFFFF || controllers == POISON_NULL)
        return false;
    else if(controllers[device >> DEVICE_INFO_SHIFT]->interface_type == ATA_TYPE_INVALID
        || controllers[device >> DEVICE_INFO_SHIFT]->interface_type != ATAPI)
        return false;

    ubyte_t command[] = {ATAPI_WRITE_12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    command[9] = sector_count;
    command[2] = (lba >> 18) & 0xFF;
    command[3] = (lba >> 10) & 0xFF;
    command[4] = (lba >> 8) & 0xFF;
    command[5] = (lba >> 0) & 0xFF;

    atapi_sendCommand(device, command, LENGTH_OF(command), destination);

    return destination;
}

/**==========================================================================**\
|**                             PRIVATE FUNCTIONS                            **|
\**==========================================================================**/

ata_interface_type_t initDevice(uword_t base_port, uword_t alt_port, bool is_slave, void* destination)
{
    ata_interface_type_t devType = ATA_TYPE_INVALID;
    
    if(inb(base_port+ATA_STATUS) == 0xFF) return ATA_TYPE_INVALID;
    outb(base_port+ATA_DEVICE_SEL, (is_slave) << 4);
    inb(base_port+ATA_STATUS);
	inb(base_port+ATA_STATUS);
	inb(base_port+ATA_STATUS);
	inb(base_port+ATA_STATUS);

    // Delay by 400ns
	inw(base_port+ATA_STATUS);
	inw(base_port+ATA_STATUS);
	inw(base_port+ATA_STATUS);
	inw(base_port+ATA_STATUS);

    //Clear nIEN, HOB and SRST
    outb(alt_port, 0x00);

    // Send Identify
	outw(base_port+ATA_SECT_COUNT, 0x00);
	outw(base_port+ATA_LBALO, 0x00);
	outw(base_port+ATA_LBAMI, 0x00);
	outw(base_port+ATA_LBAHI, 0x00);
	outb(base_port+ATA_COMMAND, ATA_IDENTIFY_DEVICE);
	if(inb(base_port+ATA_STATUS) == 0x00) return ATA_TYPE_INVALID;
    devType = detectDriveType(base_port);

	while(inb(base_port+ATA_STATUS) & 0x80) {
        if(devType == ATA_TYPE_ATAPI && initDevice_ATAPI(base_port, destination)) return devType;
        else if(devType > ATA_TYPE_ATAPI) return ATA_TYPE_INVALID;
        devType = detectDriveType(base_port);
    }
    while(!(inb(base_port+ATA_STATUS) & (0x08 | 0x01)));

	if((inb(base_port+ATA_STATUS) & 1) && initDevice_ATAPI(base_port, destination)) return devType;
	getDeviceData(base_port, destination);

    return devType;
}

bool initDevice_ATAPI(uword_t base_port, void* destination)
{
    //Assume that device has been selected
	outb(base_port+ATA_COMMAND, ATA_IDENTIFY_PACKET_DEVICE);

    // Wait for BSY to clear
    while(inb(base_port+ATA_STATUS) & 0x80) asm("pause");

    // Wait for either DRQ or ERR to set
    while(!(inb(base_port+ATA_STATUS) & (0x08 | 0x01))) asm("pause");

    // Check if exit cause was an error
	if(inb(base_port+ATA_STATUS) & 0x01) return false;
    getDeviceData(base_port, destination);

    return true;
}

int detectDriveType(uword_t base_port)
{

    int cylMid = inb(base_port+ATA_LBAMI);
	int cylHigh = inb(base_port+ATA_LBAHI);

	if(cylMid == 0x14 && cylHigh == 0xEB) return ATA_TYPE_ATAPI;
	if(cylMid == 0x69 && cylHigh == 0x96) return ATA_TYPE_SATAPI;
	if(cylMid == 0x3c && cylHigh == 0xc3) return ATA_TYPE_SATA;
	if(cylMid == 0x00 && cylHigh == 0x00) return ATA_TYPE_ATA;
	return ATA_TYPE_INVALID;
}

uword_t switchDevice(uword_t device)
{
    if(device >= controller_index) return 0xFFFF;
    uword_t base = controllers[device >> DEVICE_INFO_SHIFT]->base_port;
    outb(base+ATA_DEVICE_SEL, (device & 0x1) << 4);
    inb(base+ATA_STATUS);
	inb(base+ATA_STATUS);
	inb(base+ATA_STATUS);
	inb(base+ATA_STATUS);
    current_controller = device;
    return base;
}

void* getDeviceData(uword_t base, void* destination)
{
    uword_t* buffer = (uword_t*) destination;

	for(uword_t index = 0; index < 256; index++)
		buffer[index] = inw(base);

    return buffer;
}

irqreturn_t ata_irq() {
    // ACK Interrupt
    ubyte_t status = inb(controllers[current_controller >> 1]->base_port+ATA_STATUS);
    if(status & 0x01) {
        controllers[current_controller >> 1]->has_errored = true;
        controllers[current_controller >> 1]->error_info = inb(controllers[current_controller >> 1]->base_port+ATA_ERROR);
    }
    else {
        controllers[current_controller >> 1]->has_errored = false;
        controllers[current_controller >> 1]->error_info = 0x00;
    }
    // Unblock current device
    controllers[current_controller >> 1]->is_blocked = false;

    return HANDLED;
}

void waitForIRQ(uword_t device)
{
    if(device >= controller_index) return;
    
    volatile uword_t timeout = 0;
    volatile ubyte_t alt_status = 0;
    //TODO: Make 101% Cross Arch Compatible
    hal_enableInterrupts();
    while(controllers[device >> DEVICE_INFO_SHIFT]->is_blocked && timeout < 256 && (alt_status & 0x1) == 0) {
        timeout++;
        asm("hlt");
        alt_status = inb(controllers[device >> DEVICE_INFO_SHIFT]->alt_port);
    }
    hal_restoreInterrupts();

    // Set for next IRQ
    controllers[device >> DEVICE_INFO_SHIFT]->is_blocked = true;
}
