//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>
#include <stdlib.h>

#include "memory.h"
#include "drv/multix.h"
#include "drv/multix_cf.h"

// -----------------------------------------------------------------------
int mx_decode_cf_sc(int addr, struct cf_sc *cf)
{
	uint16_t data;

	// --- word 0 - header ---
	data = MEMB(0, addr);
	cf->pl_desc_count = (data & 0b1111111100000000) >> 8;
	cf->ll_desc_count = (data & 0b0000000011111111);

	// --- word 1 - return field ---
	cf->retf = mem_ptr(0, addr+1);

	if (cf->pl_desc_count <= 0) {
		// missing physical line description
		return MX_CF_ERR;
	}

	if (cf->ll_desc_count <= 0) {
		// missing logical line descroption
		return MX_CF_ERR;
	}

	cf->pl = calloc(cf->pl_desc_count, sizeof(struct cf_sc_pl));
	cf->ll = calloc(cf->ll_desc_count, sizeof(struct cf_sc_ll));

	if (!cf->pl || !cf->ll) {
		// cannot allocate memory, treated as communication error
		return MX_CF_COMM;
	}

	// --- physical lines, 1 word each ---
	for (int pln=0 ; pln<cf->pl_desc_count ; pln++) {
		data = MEMB(0, addr+2+pln);
		cf->pl[pln]->dir =		(data & 0b1110000000000000) >> 13;
		cf->pl[pln]->used =		(data & 0b0001000000000000) >> 12;
		cf->pl[pln]->dev_type =	(data & 0b0000111100000000) >> 8;
		cf->pl[pln]->count =	(data & 0b0000000000011111) + 1;
	}

	// --- logical lines, 4 words each ---
	for (int lln=0 ; lln<cf->ll_desc_count ; lln+=4) {
		data = MEMB(0, addr+2+cf->pl_desc_count+lln);
		cf->ll[lln]->proto	= (data & 0b1111111100000000) >> 8;
		cf->ll[lln]->pl_id	= (data & 0b0000000011111111);
		switch (cf->ll[lln]->proto) {
			case 6: // Winchester
				cf->ll[lln]->winch = calloc(1, sizeof(struct ll_winch));
				if (!cf->ll[lln]->winch) {
					return MX_CF_COMM;
				}
				data = MEMB(0, addr+2+cf->pl_desc_count+lln+1);
				cf->ll[lln]->winch->type =				(data && 0b1111111100000000) >> 8;
				cf->ll[lln]->winch->format_allowed =	(data && 0b0000000011111111);
				break;
			default: // unknown protocol
				return MX_CF_ERR;
		}
	}

	return MX_OK;
}

// vim: tabstop=4
