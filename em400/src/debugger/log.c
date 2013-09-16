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

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <pthread.h>

#include "cfg.h"
#include "errors.h"
#include "debugger/log.h"
#include "cpu/cpu.h"
#include "cpu/registers.h"

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

char *log_dname[] = {
	"REG",
	"MEM",
	"CPU",
	"OP",
	"INT",
	"IO",
	"MX",
	"PX",
	"CCHR",
	"CMEM",
	"TERM",
	"WNCH",
	"FLOP",
	"PNCH",
	"PNRD",
	"OS",
	NULL
};

char *log_reg_name[] = {
	"R0",
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"R6",
	"R7",
	"IC",
	"SR",
	"IR",
	"KB",
	"MOD",
	"MODc",
	"ZC17",
	"ALARM"
};

char *log_int_name[] = {
	"CPU power loss",
	"memory parity",
	"no memory",
	"2nd CPU (high)",
	"ext power loss",
	"timer (special)",
	"illegal opcode",
	"AWP div overflow",
	"AWP underflow",
	"AWP overflow",
	"AWP div/0",
	"special (timer)",
	"channel 0",
	"channel 1",
	"channel 2",
	"channel 3",
	"channel 4",
	"channel 5",
	"channel 6",
	"channel 7",
	"channel 8",
	"channel 9",
	"channel 10",
	"channel 11",
	"channel 12",
	"channel 13",
	"channel 14",
	"channel 15",
	"OPRQ",
	"2nd CPU (lower)",
	"software high",
	"software low"
};

FILE *log_f;
char *log_fname;

int log_enabled = 0;
int log_level[L_MAX] = { 0 };

// -----------------------------------------------------------------------
int log_init(const char *logf)
{
	eprint("Initializing logging\n");
	log_fname = strdup(logf);
	if (!log_fname) {
		return E_LOG_OPEN;
	}
	log_f = fopen(log_fname, "a");
	if (!log_f) {
		return E_LOG_OPEN;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void log_shutdown()
{
	eprint("Shutdown logging\n");
	if (log_f) {
		fclose(log_f);
	}
	if (log_fname) {
		free(log_fname);
	}
}

// -----------------------------------------------------------------------
int log_enable()
{
	if (!log_enabled) {
		log_f = fopen(log_fname, "a");
		if (!log_f) {
			return E_LOG_OPEN;
		} else {
			log_enabled = 1;
		}
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void log_disable()
{
	if (log_enabled) {
		log_enabled = 0;
		fclose(log_f);
	}
}

// -----------------------------------------------------------------------
int log_find_domain(char *name)
{
	if (!strcasecmp(name, "all")) {
		return -1;
	}

	char **d = log_dname;
	int i = 0;
	while (*(d+i)) {
		if (!strcasecmp(name, *(d+i))) {
			return i;
		}
		i++;
	}
	return -666;
}

// -----------------------------------------------------------------------
void log_setlevel(int domain, int level)
{
	if (domain == -1) {
		for (int i=0 ; i<L_MAX ; i++) {
			log_level[i] = level;
		}
	} else {
		log_level[domain] = level;
	}
}

// -----------------------------------------------------------------------
void log_log(int domain, int level, char *format, ...)
{
	if (log_level[domain] < level) {
		return;
	}

	char now[30+1];
	struct timeval ct;

	gettimeofday(&ct, NULL);
	strftime(now, 30, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));

	int q = (regs[R_SR] >> 5) & 1;
	int nb = regs[R_SR] & 0b1111;

	va_list vl;
	va_start(vl, format);

	pthread_mutex_lock(&log_mutex);
	fprintf(log_f, "%s %-4s @ %i:%i 0x%04x / ", now, log_dname[domain], q, nb, cycle_ic);
	vfprintf(log_f, format, vl);
	fprintf(log_f, "\n");
	pthread_mutex_unlock(&log_mutex);

	va_end(vl);
	fflush(log_f);
}

// vim: tabstop=4 shiftwidth=4 autoindent
