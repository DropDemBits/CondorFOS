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

/*
 * File:   ata_portio.h
 * Author: DropDemBits
 *
 * Macros for IO ports
 * Created on March 16, 2017, 3:28 PM
 */

#include <kernel/idt.h>

#ifndef _ATAPIO_H
#define _ATAPIO_H

#ifndef _ARCH_IDE_HAS_DEFAULTS_
#define _ARCH_IDE_HAS_DEFAULTS_ 1
#endif

// Fallback port numbers
#define ATA_IO_BASE_0 0x1F0
#define ATA_IO_BASE_1 0x170
#define ATA_IO_BASE_2 0x1E8
#define ATA_IO_BASE_3 0x168

#define ATA_IO_CTRL_STAT_0 0x3F6
#define ATA_IO_CTRL_STAT_1 0x376
#define ATA_IO_CTRL_STAT_2 (ATA_IO_BASE_2+0x206)
#define ATA_IO_CTRL_STAT_3 (ATA_IO_BASE_3+0x206)

// Fallback IRQ Numbers
#define ATA_IRQ_0 14
#define ATA_IRQ_1 15
#define ATA_IRQ_2 11
#define ATA_IRQ_3 10

#endif /* _ATAPIO_H */
