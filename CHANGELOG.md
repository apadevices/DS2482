# Changelog
All notable changes to the DS2482 library will be documented in this file.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).  
Versioning follows [Semantic Versioning](https://semver.org/).

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
