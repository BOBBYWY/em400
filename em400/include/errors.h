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

#ifndef ERRORS_H
#define ERRORS_H

enum em400_error {
	E_UNKNOWN = -1,
	E_OK = 0,
	E_MEM_NO_OS_MEM,
	E_MEM_BAD_SEGMENT_COUNT,
	E_MEM_CANNOT_ALLOCATE,
	E_MEM_BLOCK_TOO_SMALL,
	E_FILE_OPEN,
	E_FILE_OPERATION,
	E_TIMER_SIGNAL,
	E_TIMER_CREATE,
	E_TIMER_SET,
	E_ALLOC,
	E_DEBUGER_SIG_RESIZE
};

struct _em400_errordesc;

char * e400_gerror(int e);

#endif

// vim: tabstop=4