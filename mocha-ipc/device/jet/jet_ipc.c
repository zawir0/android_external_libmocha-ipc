/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011 Paul Kocialkowski <contact@paulk.fr>
 *                    Joerie de Gram <j.de.gram@gmail.com>
 *                    Simon Busch <morphis@gravedo.de>
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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <radio.h>

#include "ipc_private.h"
#include "jet_ipc.h"

#define LOG_TAG "RIL-Mocha_JetIPC"
#include <utils/Log.h>

/*
 * jet_modem_bootstrap - Signal the Jet modem to start IPC communication.
 *
 * The Jet bootloader already loads amss.bin and starts the baseband before
 * Android boots, so firmware upload is not required here.  We only need to
 * open the modemctl device and send the AMSSRUNREQ ioctl so that the modem
 * begins responding on the IPC channel.
 *
 * IOCTL_MODEM_ON is issued first to ensure the modem power rail is active,
 * then IOCTL_MODEM_AMSSRUNREQ initiates the IPC handshake.
 */
int32_t jet_modem_bootstrap(struct ipc_client *client)
{
    int32_t modemctl_fd = -1;

    DEBUG_I("jet_modem_bootstrap: open %s\n", JET_MODEMCTL_PATH);

    modemctl_fd = open(JET_MODEMCTL_PATH, O_RDWR | O_NDELAY);

    if (modemctl_fd < 0) {
        DEBUG_E("jet_modem_bootstrap: failed to open %s: %s\n",
                JET_MODEMCTL_PATH, strerror(errno));
        return 1;
    }

    DEBUG_I("jet_modem_bootstrap: send IOCTL_MODEM_ON\n");
    ioctl(modemctl_fd, IOCTL_MODEM_ON);

    DEBUG_I("jet_modem_bootstrap: send IOCTL_MODEM_AMSSRUNREQ\n");
    ioctl(modemctl_fd, IOCTL_MODEM_AMSSRUNREQ);

    DEBUG_I("jet_modem_bootstrap: closing %s\n", JET_MODEMCTL_PATH);
    close(modemctl_fd);

    DEBUG_I("jet_modem_bootstrap: exit\n");
    return 0;
}

/*
 * jet_ipc_open - Open the Jet IPC packet device.
 *
 * Uses IOCTL_MODEM_SEND / IOCTL_MODEM_RECV for all packet I/O, so no
 * termios configuration is needed.
 */
int32_t jet_ipc_open(void *data, uint32_t size, void *io_data)
{
    int32_t fd = -1;

    if (io_data == NULL)
        return -1;

    fd = open(JET_MODEMPACKET_PATH, O_RDWR);

    DEBUG_I("IO filename=%s fd = 0x%x\n", JET_MODEMPACKET_PATH, fd);

    if (fd < 0) {
        DEBUG_E("jet_ipc_open: failed to open %s: %s\n",
                JET_MODEMPACKET_PATH, strerror(errno));
        return 1;
    }

    memcpy(io_data, &fd, sizeof(int32_t));

    return 0;
}

int32_t jet_ipc_close(void *data, uint32_t size, void *io_data)
{
    int32_t fd = -1;

    if (io_data == NULL)
        return -1;

    fd = *((int32_t *) io_data);

    if (fd >= 0)
        return close(fd);

    return 0;
}

int32_t jet_ipc_power_on(void *data)
{
    int32_t modemctl_fd = -1;

    DEBUG_I("jet_ipc_power_on: open %s\n", JET_MODEMCTL_PATH);

    modemctl_fd = open(JET_MODEMCTL_PATH, O_RDWR | O_NDELAY);

    if (modemctl_fd < 0) {
        DEBUG_E("jet_ipc_power_on: failed to open %s: %s\n",
                JET_MODEMCTL_PATH, strerror(errno));
        return 1;
    }

    DEBUG_I("jet_ipc_power_on: send IOCTL_MODEM_ON\n");
    ioctl(modemctl_fd, IOCTL_MODEM_ON);

    DEBUG_I("jet_ipc_power_on: closing %s\n", JET_MODEMCTL_PATH);
    close(modemctl_fd);

    return 0;
}

int32_t jet_ipc_power_off(void *data)
{
    int32_t modemctl_fd = -1;

    DEBUG_I("jet_ipc_power_off: open %s\n", JET_MODEMCTL_PATH);

    modemctl_fd = open(JET_MODEMCTL_PATH, O_RDWR | O_NDELAY);

    if (modemctl_fd < 0) {
        DEBUG_E("jet_ipc_power_off: failed to open %s: %s\n",
                JET_MODEMCTL_PATH, strerror(errno));
        return 1;
    }

    DEBUG_I("jet_ipc_power_off: send IOCTL_MODEM_OFF\n");
    ioctl(modemctl_fd, IOCTL_MODEM_OFF);

    DEBUG_I("jet_ipc_power_off: closing %s\n", JET_MODEMCTL_PATH);
    close(modemctl_fd);

    return 0;
}

/*
 * send_packet - Write a single modem_io frame via IOCTL_MODEM_SEND.
 *
 * The kernel modemctl driver takes a pointer to the modem_io struct and
 * copies magic/cmd/datasize/data into the DPRAM FIFO itself.
 */
static int32_t send_packet(struct ipc_client *client, struct modem_io *ipc_frame)
{
    return client->handlers->write((void *) ipc_frame, 0,
                                   client->handlers->write_data);
}

int32_t jet_ipc_send(struct ipc_client *client, struct modem_io *ipc_frame)
{
    int32_t left_data;
    struct modem_io multi_packet;
    struct multiPacketHeader *multiHeader;

    if (ipc_frame->datasize > MAX_SINGLE_FRAME_DATA) {
        DEBUG_I("packet to send is larger than 0x1000\n");

        multi_packet.magic = 0xCAFECAFE;
        multi_packet.cmd = FIFO_PKT_FIFO_INTERNAL;
        multi_packet.datasize = 0x0C;

        multiHeader = (struct multiPacketHeader *)
                      malloc(sizeof(struct multiPacketHeader));

        multiHeader->command = 0x02;
        multiHeader->packtLen = ipc_frame->datasize;
        multiHeader->packetType = ipc_frame->cmd;

        multi_packet.data = (uint8_t *) multiHeader;
        send_packet(client, &multi_packet);
        free(multiHeader);

        left_data = ipc_frame->datasize;
        multi_packet.data = ipc_frame->data;

        while (left_data > 0) {
            if (left_data > MAX_SINGLE_FRAME_DATA)
                multi_packet.datasize = MAX_SINGLE_FRAME_DATA;
            else
                multi_packet.datasize = left_data;

            send_packet(client, &multi_packet);

            multi_packet.data += MAX_SINGLE_FRAME_DATA;
            left_data -= MAX_SINGLE_FRAME_DATA;
        }
    } else {
        send_packet(client, ipc_frame);
    }

    return 0;
}

/*
 * jet_ipc_recv - Receive one IPC frame from the modem.
 *
 * The IOCTL_MODEM_RECV ioctl fills in the modem_io struct (magic, cmd,
 * datasize) and writes the payload into the pre-allocated data buffer.
 */
int32_t jet_ipc_recv(struct ipc_client *client, struct modem_io *ipc_frame)
{
    ipc_frame->data = (uint8_t *) malloc(SIZ_PACKET_BUFSIZE);
    return client->handlers->read((void *) ipc_frame, 0,
                                  client->handlers->read_data);
}

/*
 * jet_ipc_read - Low-level read via IOCTL_MODEM_RECV.
 *
 * 'data' points to a struct modem_io whose ->data field must already point
 * to a sufficiently large buffer (SIZ_PACKET_BUFSIZE bytes).
 */
int32_t jet_ipc_read(void *data, uint32_t size, void *io_data)
{
    int32_t fd = -1;
    int32_t rc;

    if (io_data == NULL)
        return -1;

    if (data == NULL)
        return -1;

    fd = *((int32_t *) io_data);

    if (fd < 0)
        return -1;

    rc = ioctl(fd, IOCTL_MODEM_RECV, data);

    if (rc < 0)
        return -1;

    return 0;
}

/*
 * jet_ipc_write - Low-level write via IOCTL_MODEM_SEND.
 *
 * 'data' points to a struct modem_io describing the frame to send.
 */
int32_t jet_ipc_write(void *data, uint32_t size, void *io_data)
{
    int32_t fd = -1;
    int32_t rc;

    if (io_data == NULL)
        return -1;

    fd = *((int32_t *) io_data);

    if (fd < 0)
        return -1;

    rc = ioctl(fd, IOCTL_MODEM_SEND, data);

    if (rc < 0)
        return -1;

    return 0;
}

/*
 * jet_modem_operations - Issue an arbitrary ioctl on the modemctl device.
 *
 * Opens JET_MODEMCTL_PATH, issues the requested ioctl with the supplied
 * data pointer, then closes the device.  This mirrors wave_modem_operations
 * and is used by upper layers (e.g. drv.c handleJetPmicRequest).
 */
int32_t jet_modem_operations(struct ipc_client *client, void *data,
                              uint32_t cmd)
{
    int32_t modemctl_fd = -1;
    int32_t ret;

    DEBUG_I("jet_modem_operations: open %s\n", JET_MODEMCTL_PATH);

    modemctl_fd = open(JET_MODEMCTL_PATH, O_RDWR | O_NDELAY);

    if (modemctl_fd < 0) {
        DEBUG_E("jet_modem_operations: failed to open %s: %s\n",
                JET_MODEMCTL_PATH, strerror(errno));
        return 1;
    }

    DEBUG_I("jet_modem_operations: ioctl cmd = 0x%x\n", cmd);
    ret = ioctl(modemctl_fd, cmd, data);

    DEBUG_I("jet_modem_operations: closing %s\n", JET_MODEMCTL_PATH);
    close(modemctl_fd);

    return ret;
}

void *jet_ipc_common_data_create(void)
{
    void *io_data;
    int io_data_len;

    io_data_len = sizeof(int);
    io_data = malloc(io_data_len);

    if (io_data == NULL)
        return NULL;

    memset(io_data, 0, io_data_len);

    return io_data;
}

int jet_ipc_common_data_destroy(void *io_data)
{
    if (io_data == NULL)
        return 0;

    free(io_data);

    return 0;
}

int jet_ipc_common_data_set_fd(void *io_data, int fd)
{
    int *common_data;

    if (io_data == NULL)
        return -1;

    common_data = (int *) io_data;
    *common_data = fd;

    return 0;
}

int jet_ipc_common_data_get_fd(void *io_data)
{
    int *common_data;

    if (io_data == NULL)
        return -1;

    common_data = (int *) io_data;

    return (int) *(common_data);
}

struct ipc_handlers jet_default_handlers = {
    .open = jet_ipc_open,
    .close = jet_ipc_close,
    .power_on = jet_ipc_power_on,
    .power_off = jet_ipc_power_off,
    .read = jet_ipc_read,
    .write = jet_ipc_write,
    .common_data = NULL,
    .common_data_create = jet_ipc_common_data_create,
    .common_data_destroy = jet_ipc_common_data_destroy,
    .common_data_set_fd = jet_ipc_common_data_set_fd,
    .common_data_get_fd = jet_ipc_common_data_get_fd,
};

struct ipc_ops jet_ops = {
    .send = jet_ipc_send,
    .recv = jet_ipc_recv,
    .bootstrap = jet_modem_bootstrap,
    .modem_operations = jet_modem_operations,
};

void jet_ipc_register(void)
{
    ipc_register_device_client_handlers(IPC_DEVICE_JET, &jet_ops,
                                        &jet_default_handlers);
}

