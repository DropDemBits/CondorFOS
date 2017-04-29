#include <stdio.h>
#include <string.h>

#include <kernel/ata.h>
#include <kernel/ata_portio.h>
#include <kernel/idt.h>
#include <io.h>
#include <serial.h>
#include <condor.h>

#define ATA_TYPE_INVALID 0
#define ATA_TYPE_ATA 1
#define ATA_TYPE_ATAPI 2
#define ATA_TYPE_SATA 3
#define ATA_TYPE_SATAPI 4

#define BIT_SUM2(x) ((x >> 1) + (x & 0x1))
#define BIT_SUM4(x) (BIT_SUM2(x>>1) + BIT_SUM2(x & 0x3))
#define BIT_SUM8(x) (BIT_SUM4(x>>2) + BIT_SUM4(x & 0x7))
#define BIT_SUM16(x) (BIT_SUM8(x>>4) + BIT_SUM8(x & 0xF))
#define BIT_SUM32(x) (BIT_SUM16(x>>8) + BIT_SUM16(x & 0xFF))

//Forward declarations
extern void setupDevice(uword_t device);
extern void* initDevice(uword_t device, void* destination);
extern void* initDevice_ATAPI(uword_t base, void* destination);
extern void* getDeviceData(uword_t base, void* destination);
extern int detectDriveType(int device);
extern uword_t switchDevice(int device);
extern void ata_irq();
extern void waitForIRQ();

typedef struct {
    uword_t interface_type : 3;
    uword_t lba_support : 1;
    uword_t dma_support : 1;
    uword_t ata_version : 4;
    uword_t mdma_support : 2;
    uword_t udma_support : 3;
    uword_t lba_48_support : 1;
    uword_t reserved : 1;
    ubyte_t max_command_length : 2;
} DeviceType;

static DeviceType deviceType[8];
static ubyte_t currentDevice;
static ubyte_t blocked;
static volatile ubyte_t shouldBlock = 0;

/**==========================================================================**\
|**                             PRIVATE FUNCTIONS                            **|
\**==========================================================================**/

void setupDevice(uword_t device)
{
    ATAInfo* data = kmalloc(256*16);
    uword_t* dataArray = (uword_t*)data;
    
    if(initDevice(device, data) != NULL)
    {
        deviceType[device].lba_support = data->lba_support << 4;
        deviceType[device].dma_support = data->dma_support << 5;
        deviceType[device].ata_version = (BIT_SUM16(dataArray[80] & 0x7FE)) << 6;
        deviceType[device].udma_support = data->udma0_support+data->udma1_support+data->udma2_support+data->udma3_support+data->udma4_support+data->udma5_support+data->udma6_support;
        deviceType[device].mdma_support = data->mdma0_support+data->mdma1_support+data->mdma2_support;
        deviceType[device].lba_48_support = (dataArray[83] >> 10) & 0x1;
    }
    
    kfree(data);
}

void* initDevice(uword_t device, void* destination)
{
    uword_t base = switchDevice(device);
    //ubyte_t slav = device & 0x1;
    
    if(inb(base+ATA_STATUS) == 0xFF) return NULL;
    
	//outb(base+ATA_DEVICE_SEL, 0xA0 | slav << 4);
	inw(base+ATA_STATUS);
	inw(base+ATA_STATUS);
	inw(base+ATA_STATUS);
	inw(base+ATA_STATUS);
    
    //Clear nIEN, HOB and SRST
    outb(base+ATA_DEVICE_CONTROL, 0x00);
	outw(base+ATA_SECT_COUNT, 0x00);
	outw(base+ATA_LBALO, 0x00);
	outw(base+ATA_LBAMI, 0x00);
	outw(base+ATA_LBAHI, 0x00);
	outb(base+ATA_COMMAND, 0xEC);
	if(!inb(base+ATA_STATUS)) return NULL;
    
    deviceType[device].interface_type = detectDriveType(base);
    
	while(inb(base+ATA_STATUS) & 0x80)
    {
        ubyte_t drvType = detectDriveType(base);
        if(drvType == ATA_TYPE_ATAPI) return initDevice_ATAPI(base, destination);
        else if(drvType > ATA_TYPE_ATAPI) return NULL;
        asm("pause");
    }
    while(!(inb(base+ATA_STATUS) & 0x9)) asm("pause");
    
	if(inb(base+ATA_STATUS) & 1) return initDevice_ATAPI(base, destination);
    
	return getDeviceData(base, destination);
}

void* initDevice_ATAPI(uword_t base, void* destination)
{
    //Assume that device has been selected
	outb(base+ATA_COMMAND, 0xA1);
    
    while(inb(base+ATA_STATUS) & 0x80)
    {
        asm("pause");
    }
    while(!(inb(base+ATA_STATUS) & 0x9)) asm("pause");
    
	if(inb(base+ATA_STATUS) & 1) return NULL;
    
	return getDeviceData(base, destination);
}

int detectDriveType(int device)
{
	int cylMid = inb(device+ATA_LBAMI);
	int cylHigh = inb(device+ATA_LBAHI);
	
	if(cylMid == 0x14 && cylHigh == 0xEB) return ATA_TYPE_ATAPI;
	if(cylMid == 0x69 && cylHigh == 0x96) return ATA_TYPE_SATAPI;
	if(cylMid == 0x3c && cylHigh == 0xc3) return ATA_TYPE_SATA;
	if(cylMid == 0x00 && cylHigh == 0x00) return ATA_TYPE_ATA;
	return ATA_TYPE_INVALID;
}

uword_t switchDevice(int device)
{
    if(device == ATA_DEVICE_INVALID) return 0;
    
    uword_t base = 0;
    if((device & 0xe) == ATA_DEVICE_0) base = ATA_IO_BASE_0;
    if((device & 0xe) == ATA_DEVICE_2) base = ATA_IO_BASE_1;
    if(currentDevice == device) return base;
    outb(base+ATA_DEVICE_SEL, (device & 0x1) << 4);
    inb(base+ATA_STATUS);
	inb(base+ATA_STATUS);
	inb(base+ATA_STATUS);
	inb(base+ATA_STATUS);
    currentDevice = device;
    return base;
}

void* getDeviceData(uword_t base, void* destination)
{
    uword_t* buffer = (uword_t*) destination;
    
	for(uword_t index = 0; index < 256; index++)
		buffer[index] = inw(base);
	
    return buffer;
}

void ata_irq(udword_t* unused) {
    shouldBlock = unused[0] ^ unused[0];
}

void* atapi_sendCommand(uword_t device, ubyte_t command[], ubyte_t command_length, void* ptr)
{
    if(deviceType[device].interface_type == ATA_TYPE_INVALID || deviceType[device].interface_type & 0x1 || blocked) return ptr;
    blocked = 1;
    uword_t base = switchDevice(device);
    if(!base) return ptr;
    uword_t *data = ptr;
    
    outb(base+ATA_FEATURES, 0x00);
    
    //Begin Transaction
    outb(base+ATA_BYTELO, 0x00);
    outb(base+ATA_BYTEHI, 0x01);
    outb(base+ATA_COMMAND, 0xA0);
    
    while(inb(base+ATA_STATUS_ALT) & 0x80 && !(inb(base+ATA_STATUS_ALT) & 0x08)) asm("pause");
    
    outw(base+ATA_DATA, command[0] | command[1] << 8);
    outw(base+ATA_DATA, command[2] | command[3] << 8);
    outw(base+ATA_DATA, command[4] | command[5] << 8);
    if(command_length < 6) goto wait;
    outw(base+ATA_DATA, command[6] | command[7] << 8);
    outw(base+ATA_DATA, command[8] | command[9] << 8);
    outw(base+ATA_DATA, command[10] | command[11] << 8);
    switch(deviceType[device].max_command_length) {
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
    waitForIRQ();
    
    uword_t word_count = (inb(base+ATA_BYTELO) | (inb(base+ATA_BYTEHI) << 8)) >> 1;
    uword_t index = 0;
    
    //Make sure CHK/ERR bit is really what it is
    tryAgain:
    inb(base+ATA_STATUS_ALT);
    inb(base+ATA_STATUS_ALT);
    inb(base+ATA_STATUS_ALT);
    inb(base+ATA_STATUS_ALT);
    
    ubyte_t status = 0;
    
    for(uword_t words = 0; words < word_count && !((status = inb(base+ATA_STATUS_ALT)) & 0x1) && (inb(base+ATA_STATUS_ALT) & 0x08); words++, index++) {
        if(atapi_getCommandType(command[0]) == ATA_COMMAND_TYPE_READ) data[index] = inw(base+ATA_DATA);
        else if(atapi_getCommandType(command[0]) == ATA_COMMAND_TYPE_WRITE) outw(base+ATA_DATA, data[index]);
    }
    
    if(status & 0x1) return ptr;
    
    waitForIRQ();
    
    if(inb(base+ATA_STATUS_ALT) & 0x08) {
        waitForIRQ();
        goto tryAgain;
    }
    
    blocked = 0;
    return ptr;
}

void waitForIRQ()
{
    volatile uword_t timeout = 0; 
    shouldBlock = 1;
    //TODO: Make 101% Cross Arch Compatible
    asm("pushf//sti");
    while(shouldBlock && timeout < 256) {
        timeout++;
        asm("hlt");
    }
    asm("popf");
}

void* atapi_readSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination)
{
    if((lba + sector_count > 0x0FFFFFFF && !deviceType[device].lba_48_support) || sector_count == 0) return destination;
    if(deviceType[device].interface_type == ATA_TYPE_INVALID || deviceType[device].interface_type & 0x1) return destination;
    if(lba > 0xFFFFFFFF) return destination;
    ubyte_t command[] = {ATAPI_READ_12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    command[9] = sector_count;
    command[2] = (lba >> 18) & 0xFF;
    command[3] = (lba >> 10) & 0xFF;
    command[4] = (lba >> 8) & 0xFF;
    command[5] = (lba >> 0) & 0xFF;
    
    atapi_sendCommand(device, command, LENGTH_OF(command), destination);
    
    return destination;
}

void* atapi_writeSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* destination)
{
    if((lba + sector_count > 0x0FFFFFFF && !deviceType[device].lba_48_support) || sector_count == 0) return destination;
    if(deviceType[device].interface_type == ATA_TYPE_INVALID || deviceType[device].interface_type & 0x1) return destination;
    if(lba > 0xFFFFFFFF) return destination;
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
|**                              PUBLIC FUNCTIONS                            **|
\**==========================================================================**/

void ata_init()
{
    //TODO: Include PCI ATA(PI) devices
    idt_addISR(IRQ14, (udword_t) ata_irq);
    idt_addISR(IRQ15, (udword_t) ata_irq);
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
    if((lba + sector_count > 0x0FFFFFFF && !deviceType[device].lba_48_support) || sector_count == 0) return destination;
    if(deviceType[device].interface_type == ATA_TYPE_INVALID) return destination;
    if((deviceType[device].interface_type & 0x1) == 0) return atapi_readSectors(device, lba, sector_count, destination);
    uword_t base = switchDevice(device);
    uword_t* buffer = (uword_t*) destination;
    if(!base) return NULL;
    
    outb(base+ATA_SECT_COUNT, sector_count);
    if(deviceType[device].lba_48_support) {
        outb(base+ATA_LBALO, (lba >> 24) & 0xFF);
        outb(base+ATA_LBAMI, (lba >> 32) & 0xFF);
        outb(base+ATA_LBAHI, (lba >> 40) & 0xFF);
    }
    outb(base+ATA_LBALO, (lba >> 0) & 0xFF);
    outb(base+ATA_LBAMI, (lba >> 8) & 0xFF);
    outb(base+ATA_LBAHI, (lba >> 16) & 0xFF);
    outb(base+ATA_DEVICE_SEL, (deviceType[device].lba_48_support ? 0 : (lba >> 24) & 0xF) | ((device & 1)<<5) | (1<<6));
    outb(base+ATA_COMMAND, 0x20 | (deviceType[device].lba_48_support ? 0 : 4));
    
    for(uword_t i = 0; (inb(base+ATA_STATUS) & 0x8); i++)
    {
        buffer[i] = inw(base);
    }
    
    return buffer;
}

void ata_writeSectors(uword_t device, uqword_t lba, ubyte_t sector_count, void* source)
{
    if((lba + sector_count > 0x0FFFFFFF && !deviceType[device].lba_48_support) || sector_count == 0) return;
    if(deviceType[device].interface_type == ATA_TYPE_INVALID) return;
    if((deviceType[device].interface_type & 0x1) == 0) atapi_writeSectors(device, lba, sector_count, source);
    uword_t base = switchDevice(device);
    uword_t* buffer = (uword_t*) source;
    if(!base) return;
    
    outb(base+ATA_SECT_COUNT, sector_count);
    outb(base+ATA_LBALO, (lba >> 0)&0xFF);
    outb(base+ATA_LBAMI, (lba >> 8)&0xFF);
    outb(base+ATA_LBAHI, (lba >> 16)&0xFF);
    outb(base+ATA_DEVICE_SEL, ((lba >> 24)&0xF) | ((device&1)<<5) | (1<<6));
    outb(base+ATA_COMMAND, 0x30);
    
    for(uword_t i = 0; (inb(base+ATA_STATUS) & 0x8); i++)
    {
        outw(base, buffer[i]);
    }
}
