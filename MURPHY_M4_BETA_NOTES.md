# Murphy M4 touch-UI beta experiment

This local-only branch combines CrossPoint's `feat-touch-ui` beta with the
Murphy M4 hardware support. It is intentionally separate from the upstream PR
branches and the standalone `crosspoint-murphy-m4` repository.

## Source revisions

- CrossPoint beta: `55390670` (`feat-touch-ui`)
- FreeInk beta base: `477ac31`
- Local M4 FreeInk integration: `ad93c5b`
- Local CrossPoint integration: `34ac30b0`

The CrossPoint-level M4 delta deliberately excludes the original custom
`FrontlightController` and `FrontlightActivity`. The beta's `HalFrontlight` and
`FrontlightPanelActivity` are used unchanged.

## Build

```sh
pio run -e murphy_m4
```

The application image is written to:

```text
.pio/build/murphy_m4/firmware.bin
```

## Safe test flash

Use the same application-only procedure as the validated M4 port. Keep the
verified private 16 MiB factory backup outside the repository.

```sh
esptool --chip esp32s3 --port /dev/cu.usbmodem101 --baud 921600 \
  write-flash 0x10000 .pio/build/murphy_m4/firmware.bin
```

Replace the example port with the connected device's port. This does not write
the touch-controller firmware and does not replace the ESP32 partition table,
bootloader, NVS, or factory recovery data.

## Frontlight comparison

- Swipe down from the top edge to open the beta drop-down panel.
- Drag the brightness and warmth sliders; their end zones adjust by one percent.
- The physical left/right actions adjust brightness by five percent while the
  panel is open. Confirm toggles the light and Back closes the panel.
- The beta also exposes reader night mode and an optional setting to restore the
  light's on/off state after wake.

The M4 port's custom vertical panel and top+middle global toggle chord are not
present in this experiment. This is intentional so the test compares the
upstream beta behavior rather than a hybrid of both frontlight interfaces.

## Validation status

- `pio run -e murphy_m4`: passed
- `pio check -e murphy_m4 --fail-on-defect low --fail-on-defect medium
  --fail-on-defect high`: passed with no defects
- physical-device validation: pending
