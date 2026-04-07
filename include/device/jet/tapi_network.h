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

#ifndef __DEVICE_JET_NETWORK_H__
#define __DEVICE_JET_NETWORK_H__

/**
 * All the TAPI Network context structures specific to Jet will be defined here.
 *
 * The enum below merges the original Jet-specific names with the Wave-style
 * names that the shared mocha-ipc/tapi_network.c source references.  Where
 * a Jet-specific name already existed with the same opcode value it is kept
 * alongside the Wave-style alias so both are available.
 */
enum TAPI_NETWORK_TYPE
{
	TAPI_NETWORK_INIT			= 0x45,
	TAPI_NETWORK_STARTUP			= 0x46,
	TAPI_NETWORK_SHUTDOWN			= 0x47,
	TAPI_NETWORK_SET_OFFLINE_MODE		= 0x48,
	TAPI_NETWORK_SELECT			= 0x49,
	TAPI_NETWORK_RESELECT			= 0x4A,
	TAPI_NETWORK_SEARCH			= 0x4C,
	TAPI_NETWORK_SET_SELECTION_MODE		= 0x4D,
	TAPI_NETWORK_SET_NETWORK_ORDER		= 0x50,
	TAPI_NETWORK_SET_MODE			= 0x51,
	/* 0x52: set subscription/sub-mode */
	TAPI_NETWORK_SET_SUBSCRIPTION_MODE	= 0x52,
	TAPI_NETWORK_SETSUBMODE			= 0x52, /* Jet original name */
	TAPI_NETWORK_SELECT_CNF			= 0x53,
	/* 0x54: network select indication */
	TAPI_NETWORK_SELECT_IND			= 0x54,
	TAPI_NETWORK_SELECTNET			= 0x54, /* Jet original name */
	TAPI_NETWORK_SEARCH_CNF			= 0x55,
	TAPI_NETWORK_SEARCH_IND			= 0x56,
	/* 0x57: radio / signal info */
	TAPI_NETWORK_RADIO_INFO			= 0x57,
	TAPI_NETWORK_RADIOINFO			= 0x57, /* Jet original name */
	/* 0x58: common error */
	TAPI_NETWORK_COMMON_ERROR		= 0x58,
	TAPI_NETWORK_COMMONERROR		= 0x58, /* Jet original name */
	/* 0x59: cell info */
	TAPI_NETWORK_CELL_INFO			= 0x59,
	TAPI_NETWORK_CELLINFO			= 0x59, /* Jet original name */
	TAPI_NETWORK_HOME_ZONE_IND		= 0x5A,
	TAPI_NETWORK_NITZ_INFO_IND		= 0x5B,
};

#endif
