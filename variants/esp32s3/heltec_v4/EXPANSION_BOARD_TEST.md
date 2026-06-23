# Heltec V4 expansion-board manual test

Use the same `heltec-v4-tft` firmware image for every test. Record the firmware hash and both detected hardware versions from
the boot log.

## Combination matrix

Test all four base/TFT combinations:

- V0.7 base + TP V0.4 TFT adapter.
- V0.7 base + TouchTFT V0.2 adapter.
- V2.02 base + TP V0.4 TFT adapter.
- V2.02 base + TouchTFT V0.2 adapter.

For every combination:

- Base and TFT adapter detection logs match the installed hardware.
- TFT image, backlight, touch input, GNSS, both sensor connectors, buttons, and audio work.
- V0.7 does not attempt to mount an SD card.
- V2.02 + TouchTFT V0.2 mounts MicroSD using `SCK16/MOSI15/MISO45/CS3`; MicroSD reports its capacity and completes a
  temporary-file write/read/delete cycle.
- V2.02 + TP V0.4 logs that SD is disabled because GPIO15 is shared by V2.02 SD MOSI and TP V0.4 TFT CS; TFT and touch still
  work.
- TFT touch uses the primary `Wire` bus: TP V0.4 uses `47/48`, TouchTFT V0.2 uses `17/18`. Base sensors use `Wire1` only
  when their pins differ from the primary bus.
- With complete offline map tiles on SD, opening the map uses `SdFatService` and does not log `HTTPClient`, `URLService`, or
  DNS activity.
- With missing SD tiles and WiFi disconnected, entering the map should not crash; if `HTTPClient::connect()` still appears in
  the backtrace, apply the Device UI WiFi-ready fallback gate as the next-stage fix.
- Repeatedly enter and leave the map page and confirm there is no Guru Meditation or task watchdog reset.

## No expansion board

- Repeat with each TFT adapter and confirm its independent GPIO21 detection.
- TFT, backlight, and touch remain operational.
- GNSS, SD, base I2C, alternate button, and audio remain disabled.
- USB serial and LoRa continue normally.

## Unknown GPIO combination

- The boot log includes the sampled GPIO bitmap and an `unknown` warning.
- The base uses the V0.7 profile while TFT detection continues to follow GPIO21.

## OLED environment

Use the same `heltec-v4` OLED firmware image for each base-plate test.

- V0.7 reports the correct base plate, keeps the onboard OLED on `Wire(17/18)`, and detects sensors on `Wire1(4/3)`.
- V2.02 reports the correct base plate and detects the onboard OLED and sensors on the shared `Wire(17/18)` bus.
- V2.02 powers GNSS through active-low GPIO42 and detects it on UART RX39/TX38 without driving GPIO42 as reset or GPIO40 as standby.
- With no base plate, the onboard OLED remains operational while GNSS, SD, base I2C, alternate button, and audio stay disabled.
- V2.02 mounts MicroSD on `SCK16/MOSI15/MISO45/CS3` and completes a temporary-file write/read/delete cycle.
- V0.7 and no-base boots do not attempt to mount an SD card.
- GPIO21 is used only for OLED reset; no TFT-adapter detection log is emitted.
