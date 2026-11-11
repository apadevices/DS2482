# Changelog
All notable changes to the DS2482 library will be documented in this file.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).  
Versioning follows [Semantic Versioning](https://semver.org/).

---

## [1.1.1] — 2026-11-11

### Fixed
- **`readTemperature()` now checks the sensor data before using it.** The 9-byte scratchpad is verified with
  its CRC8 (byte 8) and the DS18B20 configuration byte (byte 4: bits 0-4 set, bit 7 clear). Before, the data
  was never checked: a bit error on a long cable, or a sensor lost during the read (all bytes 0xFF =
  −0.06 °C), returned a believable wrong temperature as success. Now such a read returns `false` and the
  temperature is left untouched — read again on your next cycle. An all-zero read (bus stuck low, CRC 0)
  is caught by the configuration byte check.
- `readScratchpad()` runs the same check and returns `false` on bad data (the bytes are still filled in).
- `printStatus()`: no "unused variable" warning when `DS2482_DIAGNOSTICS` is off.

### Notes
- 85.0 °C, the DS18B20 power-on value (a conversion not yet done), passes the checks — it is valid data.
  Reject it in your sketch if 85 °C cannot be real in your application (e.g. a pool).
- Found by APA-CONTROLLER (pool controller, sensors on up to 15 m of cable).
- Footprint: RAM +0, flash +74–86 B (examples, AVR). Built on AVR only (Mega, Uno).

---

## [1.1.0] — 2026-04-17

### Added
- `checkConversionStatus()` — non-blocking conversion ready poll, allows `loop()` to continue while DS18B20 converts
- Optional compile-time diagnostic system (`#define DS2482_DIAGNOSTICS 1`) — detailed I²C and 1-Wire trace output via Serial
- `keywords.txt` — Arduino IDE syntax highlighting for all public API symbols
- `library.properties` — Arduino Library Manager metadata

### Changed
- Passive pullup design formalised — DS2482 active/strong pullup intentionally disabled; external 4.7kΩ per channel is mandatory and documented
- Default I²C address documented as `0x18` in constructor and README
- README expanded with advanced usage, error handling patterns, and hardware notes

### Fixed
- Channel switching timing improved for reliable operation across all 8 channels

---

## [1.0.0] — 2025-10-01

### Added
- Initial release
- `DS2482` class — supports DS2482-800 I²C to 1-Wire bridge
- `begin()` — initialise I²C, reset DS2482, verify communication
- `startTemperatureConversion(channel)` — trigger DS18B20 conversion on selected channel (0–7)
- `readTemperature(channel, &temperature)` — read converted result in °C
- Single-sensor-per-channel design — no ROM code or sensor addressing required
- Up to 8 simultaneous DS18B20 sensors (one per channel)
- Error return values on all public methods — no silent failures
- Zero dynamic memory allocation — fixed RAM footprint after `begin()`
- No external dependencies — Wire library only
- MIT License

---

*Library by APADevices [@kecup] — [github.com/apadevices/DS2482](https://github.com/apadevices/DS2482)*
