# Changelog

All notable changes to the MiP Power Up library are documented in this file.

## [2.0.2] - 2026-08-13
### Added
- Added `mip.getBaudRate()` method to query the active UART link speed (115200 or 9600 baud) negotiated during `begin()`.
- Added automated enum and macro table generation (`Constants-and-Defines`) to `deploy-wiki.yml` GitHub Actions workflow.
- Updated `LICENSE.txt` to include dual-licensing details for Apache 2.0 (core library) and MIT License (`MiP_Debug`).

### Refactored & Changed
- Renamed `MiPDebug` to `MiP_Debug` for complete naming uniformity across all library subsystem classes (`MiP_Battery`, `MiP_WiFi`, etc.).
- Moved internal container `CircularQueue` into the `mip_detail` internal namespace to clean up the public API surface.
- Converted preprocessor macros and magic numbers across main and component classes to strongly typed `static constexpr` constants.
- Made all component subclass constructors `private` with `friend class MiP;` to prevent orphan subclass instantiation.
- Refactored `MiP_Debug` to dynamically pull the library version string from `MPU_D1_MINI_VERSION`.
- Redesigned debug macros (`MIP_DEBUG_*_PRINT`, `MIP_DEBUG_*_PRINTLN`) to prevent duplicate level prefix printing (`[INFO]`, `[ERROR]`).
- Wrapped static debug log strings across all `.cpp` files in `F()` macros to conserve dynamic RAM on ESP8266.

### Fixed
- Fixed verification mismatch bug in `MiP_ChestLED::write(flash)` where millisecond parameters were incorrectly compared against tick counts.
- Fixed framing corruption bug in `MiP_Serial::processAllResponseData()` on incomplete response reads by flushing the RX buffer.
- Fixed modem force-sleep wake sequence in `MiP_WiFi::disableAirplaneMode()`.
- Fixed ESP8266 Flash string concatenation error in `MiP_WiFi::connect()`.
- Fixed buffer reserve copy-paste typo in `MiP_Debug::begin()`.
- Replaced floating-point `pow(10, i)` in `MiP_Debug::formatNumber()` with fast integer multiplication.
- Fixed `TELNET_PORT` compilation error by declaring it `public`.
- Fixed `Odometer.ino` stale variable bug where distance was printed prior to re-querying after reset.
- Fixed `Frustration.ino` debug port assignment (`Serial` -> `Serial1`) and fixed infinite loop when tipped over.
- Fixed `TimeWiFi.ino` function prototyping order and added a 30-second NTP sync timeout guard.
- Fixed `ZeroEEPROM.ino` physical address formatting bug.
- Fixed string truncation bug in `SendDongleCode.ino` stack buffer allocation.

### Documentation & Examples
- Completed a comprehensive Doxygen documentation pass across all public classes, structs, enums, methods, and parameters.
- Updated all 20 example sketches to adhere strictly to personification guidelines (referring to MiP directly as a person).
- Added watchdog reset (`WDT`) yield guards (`delay()`) across polling loops in example sketches.
- Updated `SoftwareHardwareVersion.ino` to demonstrate `mip.getBaudRate()`.

## [2.0.1] - 2026-08-09
### Added
- `sendDongleCode()` now supports 3- and 4-byte IR codes (previously only reliable for 2-byte).
- New example: `SendDongleCode`.

### Fixed
- IR dongle transmit bit-length field now correctly reflects 16 / 24 / 32 bits instead of a fixed 0x10.
- Various minor robustness cleanups.

## [2.0.0] - 2026-08-01
### Breaking
- Replaced monolithic `MiP` API with subsystem objects
  (`mip.chestLED`, `mip.motion`, `mip.radar`, `mip.sound`, `mip.wifi`, ...).
- Wi‑Fi/OTA setup moved to `mip.wifi.begin()`.
- Library renamed to “MiP Power Up - D1 mini”.

### Added
- Modular source layout under `src/`.
- Explicit serial transport and OOB event dispatch (including radar).
- Richer keywords.txt / documentation pass.

### Fixed
- Various robustness and naming cleanups.

## [1.0.1] - 2026-06-14
### Added
- Added auto speed negotiation to switch between 9600 and 115200 baud depending on the MiP hardware revision.
- Added support for reporting various debug levels: none, error, warning, info.

### Fixed
- Failure to connect to early MiP boards employing 9600 baud serial speed.

## [0.0.1] - 2018-08-15
- Initial release of the MiP ESP8266 Library.
- Basic movement commands and chest LED control implementation.
