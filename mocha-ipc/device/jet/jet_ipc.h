/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 *
 * Modified for Jet - KB <kbjetdroid@gmail.com>
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

#ifndef _JET_IPC_H_
#define _JET_IPC_H_

#include <radio.h>
#include <device/jet/jet_modem_ctl.h>

/*
 * Jet modem control device path.  The modemctl driver exposes a single
 * character device that handles both control IOCTLs (ON/OFF/RESET/
 * AMSSRUNREQ/PMIC) and IPC packet IOCTLs (SEND/RECV).
 *
 * Override at build time by passing -DJET_MODEMCTL_PATH=\"/dev/yourpath\"
 * to the compiler.
 */
#ifndef JET_MODEMCTL_PATH
#define JET_MODEMCTL_PATH		"/dev/modemctl"
#endif

/*
 * Device node used for IPC packet exchange (IOCTL_MODEM_SEND/RECV).
 * On Jet this is the same node as the control device.
 * Override with -DJET_MODEMPACKET_PATH=\"/dev/yourpath\" if needed.
 */
#ifndef JET_MODEMPACKET_PATH
#define JET_MODEMPACKET_PATH		JET_MODEMCTL_PATH
#endif

struct multiPacketHeader {
	uint32_t command;
	uint32_t packtLen;
	uint32_t packetType;
};

#endif

