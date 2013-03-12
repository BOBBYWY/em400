//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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
#include <pthread.h>
#include <stdio.h>

#include "cfg.h"
#include "utils.h"
#include "errors.h"
#include "io.h"
#include "memory.h"
#include "registers.h"

#include "drv/drivers.h"

#include "debugger/log.h"

struct chan_t io_chan[IO_MAX_CHAN];

// -----------------------------------------------------------------------
int io_chan_init(int c_num, char *chan_name)
{
	struct drv_t *c_driver = drv_get(DRV_CHAN, CHAN_IGNORE, chan_name);

	if (!c_driver) {
		return E_IO_CHAN_UNKNOWN;
	} 

	// driver sanity checks
	if ((!c_driver->f_init) || (!c_driver->f_shutdown) || (!c_driver->f_reset) || (!c_driver->f_cmd)) {
		return E_IO_DRV_CHAN_BAD;
	}

	// common channel initialization
	io_chan[c_num].type = c_driver->chan_type;
	io_chan[c_num].name = c_driver->name;
	io_chan[c_num].finish = 0;
	io_chan[c_num].f_shutdown = c_driver->f_shutdown;
	io_chan[c_num].f_reset = c_driver->f_reset;
	io_chan[c_num].f_cmd = c_driver->f_cmd;

	eprint("  Channel %i (%s):\n", c_num, io_chan[c_num].name);

	// initialize the channel
	return c_driver->f_init(io_chan+c_num, NULL);
}

// -----------------------------------------------------------------------
int io_unit_init(struct chan_t *chan, struct cfg_unit_t *unit_cfg)
{
	struct drv_t *u_driver = drv_get(DRV_UNIT, chan->type, unit_cfg->name);

	if (!u_driver) {
		return E_IO_UNIT_UNKNOWN;
	}

	// driver sanity checks
	if ((!u_driver->f_init) || (!u_driver->f_shutdown) || (!u_driver->f_reset) || (!u_driver->f_cmd)) {
		return E_IO_DRV_UNIT_BAD;
	}

	struct unit_t *unit = malloc(sizeof(struct unit_t));
	chan->unit[unit_cfg->num] = unit;

	unit->name = u_driver->name;

	eprint("   Unit %i (%s)\n", unit_cfg->num, unit->name);

	// initialize the unit
	return u_driver->f_init(&chan->unit[unit_cfg->num], unit_cfg->args);
}

// -----------------------------------------------------------------------
int io_init()
{
	int res;

	for (int chan=0 ; chan<IO_MAX_CHAN ; chan++) {
		char *chan_name = em400_cfg.chan[chan].name;
		if (!chan_name) continue;

		res = io_chan_init(chan, chan_name);
		if (res != E_OK) {
			return res;
		}

		struct cfg_unit_t *unit_cfg = em400_cfg.chan[chan].units;
		while (unit_cfg) {
			io_unit_init(&io_chan[chan], unit_cfg);
			if (res != E_OK) {
				return res;
			}
			unit_cfg = unit_cfg->next;
		}
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void io_shutdown()
{
	for (int i=0 ; i<IO_MAX_CHAN ; i++) {
		struct chan_t *ch = io_chan + i;
		for (int j=0 ; j<IO_MAX_UNIT ; j++) {
			struct unit_t *u = ch->unit[j];
			if (u) {
				if (u->f_shutdown) {
					u->f_shutdown(u);
				}
				free(u);
			}
		}
		if (ch->f_shutdown) {
			ch->f_shutdown(ch);
		}
	}
}

// -----------------------------------------------------------------------
int io_dispatch(int dir, uint16_t n, uint16_t *r)
{
	int is_mem = (n & 0b0000000000000001);
	int chan = (n & 0b0000000000011110) >> 1;
	int unit = (n & 0b0000000011100000) >> 5;
	int cmd = (n & 0b1111111100000000) >> 8;

	// software memory configuration
	if (is_mem) {
		LOG(D_IO, 1, "MEM command, dir = %s, module = %d, segment = %d, cmd = %d", dir ? "OUT" : "IN", chan, unit, cmd);
		if (dir == IO_OU) {
			int nb = *r & 0b0000000000001111;
			int ab = (*r & 0b1111000000000000) >> 12;
			// here, channel is memory module, unit is memory segment
			return mem_add_map(nb, ab, chan, unit);
		} else {
			// TODO: what to return?
			return IO_NO;
		}

	// channel/unit command
	} else {
		int res = io_chan[chan].f_cmd(&io_chan[chan], unit, dir, cmd, r);

#ifdef WITH_DEBUGGER
		char *cmdc = int2bin(cmd, 8);
		LOG(D_IO, 1, "I/O command, dir = %s, chan = %d, unit = %d, cmd = %s, result -> %i", dir ? "OUT" : "IN", chan, unit, cmdc, res);
		free(cmdc);
#endif
		return res;
	}
}

// vim: tabstop=4
