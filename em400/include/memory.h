//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef MEMORY_H
#define MEMORY_H

#include <inttypes.h>

#define MEM_MAX_MODULES 16
#define MEM_MAX_SEGMENTS 8
#define MEM_SEGMENT_SIZE 4 * 1024
#define MEM_MAX_NB 16
#define MEM_MAX_AB 16

extern short int em400_mem_conf[MEM_MAX_MODULES];
extern uint16_t *em400_mem_segment[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
extern uint16_t *em400_mem_map[MEM_MAX_NB][MEM_MAX_AB];

int em400_mem_init();
void em400_mem_shutdown();
int em400_mem_add_user_map(unsigned short int nb, unsigned short int ab, unsigned short int mp, unsigned short int segment);
void em400_mem_remove_user_maps();

uint16_t * em400_mem_ptr(short unsigned int nb, uint16_t addr);
uint16_t em400_mem_read(short unsigned int nb, uint16_t addr);
void em400_mem_write(short unsigned int nb, uint16_t addr, uint16_t val);

void em400_mem_clear();
int em400_mem_load_image(const char* fname, unsigned short block);

#define MEM(a)			em400_mem_read(SR_Q*SR_NB, a)
#define MEMNB(a)		em400_mem_read(SR_NB, a)
#define MEMB(b, a)		em400_mem_read(b, a)
#define MEMw(a, x)		em400_mem_write(SR_Q*SR_NB, a, x)
#define MEMNBw(a, x)	em400_mem_write(SR_NB, a, x)
#define MEMBw(b, a, x)	em400_mem_write(b, a, x)

// -----------------------------------------------------------------------
// dword conversions
#define DWORD(x, y) (x<<16) | (y)
#define DWORDl(z) (z>>16) & 0xffff
#define DWORDr(z) z & 0xffff

#endif

// vim: tabstop=4
