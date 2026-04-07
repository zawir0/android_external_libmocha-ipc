/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
 *
 * Implemented as per the Mocha AP-CP protocol analysis done by Dominik Marszk
 *
 * libmocha-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libmocha-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libmocha-ipc.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __DEVICE_JET_NETTEXT_H__
#define __DEVICE_JET_NETTEXT_H__

/**
 * All the TAPI Nettext (sms) context structures specific to Jet will be defined here.
 *
 * The enum below merges the original Jet-specific names with the Wave-style
 * names used by the shared mocha-ipc/tapi_nettext.c source.  Where a Jet name
 * already existed it is kept as an alias alongside the Wave-style name.
 */
enum TAPI_NETTEXT_TYPE
{
	TAPI_NETTEXT_SEND			= 0x37,
	TAPI_NETTEXT_SET_MEM_AVAIL		= 0x3A,
	TAPI_NETTEXT_SETMEMAVAIL		= 0x3A, /* Jet original name */
	TAPI_NETTEXT_SET_PREFERRED_MEM		= 0x3B,
	TAPI_NETTEXT_SETPREFERREDMEM		= 0x3B, /* Jet original name */
	TAPI_NETTEXT_SET_BURST			= 0x3D,
	TAPI_NETTEXT_SETNETBURST		= 0x3D, /* Jet original name */
	TAPI_NETTEXT_SET_CB_SETTING		= 0x3E,
	TAPI_NETTEXT_SETCBSETTING		= 0x3E, /* Jet original name */
	TAPI_NETTEXT_SEND_CALLBACK		= 0x40,
	TAPI_NETTEXT_INCOMING			= 0x42,
};

#endif
