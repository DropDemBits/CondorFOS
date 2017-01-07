/*
 * Copyright (C) 2017 DropDemBits
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
 * File:   pmm.h
 * Author: DrompDemBits
 *
 * Created on January 5, 2017, 5:56 PM
 */

#include <addrs.h>
#include <condor.h>

#ifndef PMM_H
#define PMM_H

/**
 * Allocates page size physical memory
 * @return The address
 */
void * pmalloc();
void * pfree();

#endif /** PMM_ H */

