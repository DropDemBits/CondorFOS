#include <stdio.h>
#include <string.h>

#include <kernel/ata.h>
#include <kernel/atapio.h>
#include <io.h>
#include <serial.h>
#include <condor.h>

#define ATA_SECT_COUNT 2
#define ATA_LBALO 3
#define ATA_LBAMI 4
#define ATA_LBAHI 5
#define ATA_DEVICE_SEL 6
#define ATA_CMD_REG_STATUS 7

#define ATA_TYPE_INVALID 0
#define ATA_TYPE_ATA 1
#define ATA_TYPE_ATAPI 2
#define ATA_TYPE_SATA 3
#define ATA_TYPE_SATAPI 4

static uword_t deviceType[8];
static ubyte_t currentDevice;

static int detectDriveType(int device)
{
	int cylMid = inb(device+ATA_LBAMI);
	int cylHigh = inb(device+ATA_LBAHI);
	
	if(cylMid == 0x14 && cylHigh == 0xEB) return ATA_TYPE_ATAPI;
	if(cylMid == 0x69 && cylHigh == 0x96) return ATA_TYPE_SATAPI;
	if(cylMid == 0x3c && cylHigh == 0xc3) return ATA_TYPE_SATA;
	if(cylMid == 0x00 && cylHigh == 0x00) return ATA_TYPE_ATA;
	return ATA_TYPE_INVALID;
}

static void* getDeviceData(uword_t base)
{
    static uword_t buffer[256];
	for(uword_t index = 0; index < 256; index++)
		buffer[index] = inw(base);
	return buffer;
}

static void* initDevice_ATAPI(uword_t base)
{
    //Assume that device has been selected
	outb(base+ATA_CMD_REG_STATUS, 0xA1);
    
    while(inb(base+ATA_CMD_REG_STATUS) & 0x80)
    {
        asm("pause");
    }
    while(!(inb(base+ATA_CMD_REG_STATUS) & 0x9)) asm("pause");
    
	if(inb(base+ATA_CMD_REG_STATUS) & 1) return NULL;
    
	return getDeviceData(base);
}

static uword_t switchDevice(int device)
{
    if(device == ATA_DEVICE_INVALID) return 0;
    
    uword_t base = 0;
    if((device & 0xe) == ATA_DEVICE_0) base = ATA_IO_BASE_0;
    if((device & 0xe) == ATA_DEVICE_2) base = ATA_IO_BASE_1;
    if((device & 0xe) == ATA_DEVICE_4) base = ATA_IO_BASE_2;
    if((device & 0xe) == ATA_DEVICE_6) base = ATA_IO_BASE_3;
    if(currentDevice == device) return base;
    outb(base, (device & 0x1) << 4);
    inb(base+ATA_CMD_REG_STATUS);
	inb(base+ATA_CMD_REG_STATUS);
	inb(base+ATA_CMD_REG_STATUS);
	inb(base+ATA_CMD_REG_STATUS);
    currentDevice = device;
    return base;
}

static void* initDevice(uword_t device)
{
    uword_t base = switchDevice(device);
    //ubyte_t slav = device & 0x1;
    
    if(inb(base+ATA_CMD_REG_STATUS) == 0xFF) return NULL;
    
	//outb(base+ATA_DEVICE_SEL, 0xA0 | slav << 4);
	inw(base+ATA_CMD_REG_STATUS);
	inw(base+ATA_CMD_REG_STATUS);
	inw(base+ATA_CMD_REG_STATUS);
	inw(base+ATA_CMD_REG_STATUS);
    
	outw(base+ATA_SECT_COUNT, 0x00);
	outw(base+ATA_LBALO, 0x00);
	outw(base+ATA_LBAMI, 0x00);
	outw(base+ATA_LBAHI, 0x00);
	outb(base+ATA_CMD_REG_STATUS, 0xEC);
	if(!inb(base+ATA_CMD_REG_STATUS)) return NULL;
    
    deviceType[device] = detectDriveType(base);
    
	while(inb(base+ATA_CMD_REG_STATUS) & 0x80)
    {
        ubyte_t drvType = detectDriveType(base);
        if(drvType == ATA_TYPE_ATAPI) return initDevice_ATAPI(base);
        else if(drvType > ATA_TYPE_ATAPI) return NULL;
        asm("pause");
    }
    while(!(inb(base+ATA_CMD_REG_STATUS) & 0x9)) asm("pause");
    
	if(inb(base+ATA_CMD_REG_STATUS) & 1) return initDevice_ATAPI(base);
    
	return getDeviceData(base);
}

static void setupDevice(uword_t device)
{
    ATAInfo* data = initDevice(device);
    if(data != NULL)
    {
        deviceType[device] |= data->lba_support << 4;
        deviceType[device] |= data->dma_support << 5;
        deviceType[device] |= data->major_version << 6;
    }
}

void ata_init()
{
    setupDevice(ATA_DEVICE_0);
    setupDevice(ATA_DEVICE_1);
    setupDevice(ATA_DEVICE_2);
    setupDevice(ATA_DEVICE_3);
    setupDevice(ATA_DEVICE_4);
    setupDevice(ATA_DEVICE_5);
    setupDevice(ATA_DEVICE_6);
    setupDevice(ATA_DEVICE_7);
}

void* ata_readSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination)
{
    uword_t base = switchDevice(device);
    uword_t* buffer = (uword_t*) destination;
    if(!base) return NULL;
    
    outb(base+ATA_SECT_COUNT, sector_count);
    outb(base+ATA_LBALO, (lba >> 0)&0xFF);
    outb(base+ATA_LBAMI, (lba >> 8)&0xFF);
    outb(base+ATA_LBAHI, (lba >> 16)&0xFF);
    outb(base+ATA_DEVICE_SEL, ((lba >> 24)&0xF) | ((device&1)<<5) | (1<<6));
    outb(base+ATA_CMD_REG_STATUS, 0x20);
    
    for(uword_t i = 0; (inb(base+ATA_CMD_REG_STATUS) & 0x8); i++)
    {
        buffer[i] = inw(base);
    }
    
    return buffer;
}

void ata_writeSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* source)
{
    uword_t base = switchDevice(device);
    uword_t* buffer = (uword_t*) source;
    if(!base) return;
    
    outb(base+ATA_SECT_COUNT, sector_count);
    outb(base+ATA_LBALO, (lba >> 0)&0xFF);
    outb(base+ATA_LBAMI, (lba >> 8)&0xFF);
    outb(base+ATA_LBAHI, (lba >> 16)&0xFF);
    outb(base+ATA_DEVICE_SEL, ((lba >> 24)&0xF) | ((device&1)<<5) | (1<<6));
    outb(base+ATA_CMD_REG_STATUS, 0x30);
    
    for(uword_t i = 0; (inb(base+ATA_CMD_REG_STATUS) & 0x8); i++)
    {
        outw(base, buffer[i]);
    }
}
