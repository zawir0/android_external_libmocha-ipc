# Jet (GT-S8000) IPC Backend

This document describes the Jet-specific IPC backend in `libmocha-ipc` for the
Samsung GT-S8000 ("Jet") handset, including kernel device node requirements,
how to run `ipc-modemctrl`, expected log output, and how the battery gauge DRV
packet is handled.

---

## Background

The GT-S8000 uses a Qualcomm MSM6246 baseband (CP) communicating with the
Samsung S3C6410 application processor (AP) over a shared-memory (OneDRAM /
DPRAM) bus, controlled via a modemctl character device.

Unlike the Wave (GT-S8500) port the GT-S8000 bootloader **already loads
`amss.bin` and starts baseband execution** before Android boots (confirmed by
KB in 2011 forum post).  Userspace therefore does **not** need to upload
firmware; it only needs to signal the modem to begin IPC communication.

---

## Implementation: `mocha-ipc/device/jet/jet_ipc.c`

The Jet backend mirrors the Wave (`wave_ipc.c`) structure, adapted for the
Jet-specific kernel driver interface defined in
`include/device/jet/jet_modem_ctl.h`.

### Device paths

| Macro | Default value | Purpose |
|---|---|---|
| `JET_MODEMCTL_PATH` | `/dev/modemctl` | Modem control & IPC packet device |
| `JET_MODEMPACKET_PATH` | `JET_MODEMCTL_PATH` | IPC packet device (same node on Jet) |

Both macros can be overridden at build time:

```
LOCAL_CFLAGS += -DJET_MODEMCTL_PATH=\"/dev/your_modemctl\"
```

### Bootstrap sequence (`jet_modem_bootstrap`)

1. Open `JET_MODEMCTL_PATH` (`O_RDWR | O_NDELAY`).
2. Issue `IOCTL_MODEM_ON` – ensures the modem power rail is active.
3. Issue `IOCTL_MODEM_AMSSRUNREQ` – signals the modem to begin the IPC
   handshake (equivalent to the Wave `AMSSRUNREQ` step).
4. Close the device.

Because the bootloader has already started `amss.bin`, no firmware upload is
performed.

### IPC packet I/O

Packet send/receive uses the ioctl interface defined in `jet_modem_ctl.h`:

| Operation | IOCTL |
|---|---|
| Send packet | `IOCTL_MODEM_SEND` (`_IO('o', 0x23)`) |
| Receive packet | `IOCTL_MODEM_RECV` (`_IO('o', 0x24)`) |

Each ioctl takes a pointer to a `struct modem_io` (magic/cmd/datasize/data).
The kernel modemctl driver handles DPRAM FIFO framing internally.

### Bug fixes included

* **`common_data_set_fd`** – the original code in both Jet and Wave backends
  assigned the local pointer (`common_data = &fd`) instead of writing the
  value into the allocated buffer (`*common_data = fd`).  This prevented the
  open fd from ever being stored, causing `ipc_client_get_handlers_common_data_fd`
  to always return 0 and the read-loop's `select()` to block on fd 0 (stdin).

* **Use-after-free in `jet_ipc_send`** – the original code freed the
  `multiHeader` allocation before passing the pointer to `send_packet`.

---

## Required kernel device nodes

The kernel must expose the following device node for the Jet modemctl driver:

```
/dev/modemctl    (default; override with JET_MODEMCTL_PATH)
```

The driver must support at minimum the following ioctls:

| Constant | Value | Description |
|---|---|---|
| `IOCTL_MODEM_ON` | `_IO('o', 0x25)` | Power modem on |
| `IOCTL_MODEM_OFF` | `_IO('o', 0x22)` | Power modem off |
| `IOCTL_MODEM_AMSSRUNREQ` | `_IO('o', 0x26)` | Request AMSS run (IPC handshake) |
| `IOCTL_MODEM_SEND` | `_IO('o', 0x23)` | Send `modem_io` packet |
| `IOCTL_MODEM_RECV` | `_IO('o', 0x24)` | Receive `modem_io` packet |
| `IOCTL_MODEM_PMIC` | `_IO('o', 0x27)` | PMIC control (battery DRV) |
| `IOCTL_MODEM_RESET` | `_IO('o', 0x20)` | Reset modem |
| `IOCTL_MODEM_RAMDUMP` | `_IO('o', 0x19)` | Trigger RAM dump |

---

## Building

In the AOSP build system set `TARGET_DEVICE=jet` (or ensure the device
makefile does so).  `Android.mk` automatically:

* Compiles `mocha-ipc/device/jet/jet_ipc.c`.
* Passes `-DDEVICE_JET` so device-conditional code (e.g. `drv.c`) selects the
  Jet paths.

---

## Running `ipc-modemctrl` on Jet

The `ipc-modemctrl` binary (built from `tools/modemctrl.c`) is the primary
diagnostic tool.

### Bootstrap only

```sh
# Auto-detect device from /proc/cpuinfo
ipc-modemctrl --debug bootstrap

# Force Jet (useful when /proc/cpuinfo does not match "GT-S8000")
ipc-modemctrl --debug --device=jet bootstrap
```

### Bootstrap + IPC read loop

```sh
ipc-modemctrl --debug --device=jet start
```

This performs bootstrap, opens the packet device, and enters a `select()`
loop printing every received `modem_io` frame.

### Power control

```sh
ipc-modemctrl --device=jet power-on
ipc-modemctrl --device=jet power-off
```

---

## Expected dmesg / logcat output

A successful bootstrap on Jet should produce log lines similar to:

```
D/RIL-Mocha_JetIPC: jet_modem_bootstrap: open /dev/modemctl
D/RIL-Mocha_JetIPC: jet_modem_bootstrap: send IOCTL_MODEM_ON
D/RIL-Mocha_JetIPC: jet_modem_bootstrap: send IOCTL_MODEM_AMSSRUNREQ
D/RIL-Mocha_JetIPC: jet_modem_bootstrap: closing /dev/modemctl
D/RIL-Mocha_JetIPC: jet_modem_bootstrap: exit
D/RIL-Mocha_JetIPC: IO filename=/dev/modemctl fd = 0x5
```

The read loop will then block on `select()` until the modem sends its first
unsolicited packet (typically a `SYSTEM_INFO_REQ` DRV packet shortly after
the IPC handshake completes).

---

## Battery gauge DRV packet flow

The battery level is delivered by the modem as a DRV packet of type
`BATT_GAUGE_STATUS_RESP` (0x55), defined in `include/device/jet/drv.h`.

### Flow

```
Modem CP
  │
  │  FIFO_PKT_DRV frame, drvPacketType = BATT_GAUGE_STATUS_RESP (0x55)
  │  payload byte [1] = battery percentage (0–100)
  ▼
ipc_parse_drv()   [mocha-ipc/drv.c]
  │
  ▼
handleFuelGaugeStatus(uint8_t percentage)
  │
  │  opens /sys/class/power_supply/battery/capacity  (O_RDWR)
  │  writes percentage as ASCII decimal string
  ▼
Android BatteryService reads sysfs → reports real battery level
```

The same handler is invoked for `BATT_GAUGE_STATUS_CHANGE_IND` (0x55) which
is sent by the modem whenever the battery level changes significantly.

### Triggering a battery status request

To request an immediate battery reading, send a `BATT_GAUGE_STATUS_REQ`
(0x55) DRV packet to the modem:

```c
drv_send_packet(BATT_GAUGE_STATUS_REQ, NULL, 0);
```

This is typically called during wake-up (see `include/device/jet/drv.h`
comments).

### sysfs path

The battery capacity sysfs node is constructed from `power_dev_path` (defined
in `mocha-ipc/drv.c`):

```
/sys/devices/platform/i2c-gpio.6/i2c-6/6-0066/max8998-charger/power_supply/battery/capacity
```

This path targets the MAX8998 PMIC's battery power-supply class node on the
Jet hardware.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| `ipc-modemctrl` fails to open `/dev/modemctl` | Device node not present | Check kernel config; ensure modemctl driver is compiled and loaded |
| `ipc-modemctrl start` blocks in `select()` indefinitely | Bootstrap didn't complete (AMSSRUNREQ not acknowledged) | Check dmesg for modemctl driver errors; verify `amss.bin` is present on the DPRAM |
| Battery still shows hardcoded value | `handleFuelGaugeStatus` can't open sysfs path | Check `power_dev_path` in `drv.c`; ensure MAX8998 charger driver is loaded |
| `ipc_client_new()` returns NULL | `/proc/cpuinfo` Hardware string not matched | Use `--device=jet` to bypass auto-detection |
