# CrossPoint for Murphy M4

Experimental, open-source CrossPoint firmware port for the Murphy M4 e-reader.
This repository is a standalone integration tree: it includes the matching
FreeInk SDK snapshot, so no Git submodule initialization is required.

The port has been tested on a retail Murphy M4 with working display, touch,
three side buttons, SD card, battery indicator, Wi-Fi, sleep/wake, and adjustable
cool/warm frontlight.

> [!WARNING]
> This is community firmware, not an official Murphy or CrossPoint release.
> Make and verify a complete 16 MiB backup of your own reader before flashing.
> Full backups can contain credentials and identifiers and must not be shared.

## Build

Requirements:

- Python 3.12 or another PlatformIO-supported Python version
- [PlatformIO/pioarduino](https://github.com/pioarduino/pioarduino)
- A recursive checkout is **not** required; FreeInk SDK is vendored here

```sh
pio run -e murphy_m4
```

The application image is generated at:

```text
.pio/build/murphy_m4/firmware.bin
```

## Install

Read the complete [build, backup, installation, and restoration guide](docs/murphy-m4/findings/crosspoint_port.md)
before continuing.

After making a verified full-flash backup and entering ESP32-S3 download mode,
install only the application partition:

```sh
esptool --chip esp32s3 --port /dev/cu.usbmodem101 --baud 921600 \
  write-flash 0x10000 .pio/build/murphy_m4/firmware.bin
```

Replace the example serial port with the one reported by your system.

## Controls added by this port

- Swipe down from the top edge to open the vertical frontlight panel.
- Drag the brightness or warmth bar to adjust it.
- Use the top/middle side buttons to increment/decrement the selected bar.
- Short-press the bottom button to switch between brightness and warmth.
- Press the top and middle side buttons together anywhere to toggle the light.

See [frontlight controls](docs/murphy-m4/findings/frontlight.md) and the
[complete port status](docs/murphy-m4/README.md).

## Touch-controller safety

The driver follows the factory firmware's runtime initialization and writes only
three volatile configuration registers. It contains no touch firmware image and
does not invoke a controller bootloader, erase, flash-write, or upgrade command.
Touch-controller firmware modification is explicitly outside this project's scope.

See [touch findings](docs/murphy-m4/findings/touch.md) for the confirmed wiring,
protocol, coordinate transform, and safety boundary.

## Provenance

This integration is based on:

- CrossPoint Reader `4e619035`, with the M4 integration commit developed as `c884fb9d`
- FreeInk SDK `a485dc4`, with the M4 hardware commit developed as `0803aeb`
- Murphy M4 research and recovery documentation developed as `8fdc6e8`

The original CrossPoint README is retained at
[docs/CROSSPOINT_UPSTREAM_README.md](docs/CROSSPOINT_UPSTREAM_README.md).

CrossPoint Reader and FreeInk SDK are MIT-licensed. Their existing copyright
and license notices are retained in this tree. The M4 port documentation and
original integration work are also provided under the repository's MIT license.

## Status

The final reviewed build and full factory-image restoration were both tested on
hardware. Broader testing across additional M4 units and hardware revisions is
still encouraged. Please report the device revision, exact build commit, and
reproduction steps with any issue.
