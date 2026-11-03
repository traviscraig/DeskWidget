# DeskWidget

**A human-specified, AI-agent-developed desktop information display.**

DeskWidget is a purpose-built embedded dashboard for the Adafruit Qualia ESP32-S3
and a 480 × 480 RGB display. It combines a clock, calendar, local weather, indoor
environmental measurements, air-quality trends, and device diagnostics in a
single always-on display designed for a desk or workspace.

The project also serves as a practical example of **spec-driven development with
an AI coding agent**: a human defined the product goals, hardware constraints,
required behavior, and acceptance expectations, while an AI agent translated
those specifications into firmware, reviewed the implementation, diagnosed
hardware and software issues, and iteratively refined the result.

## Project Goals

DeskWidget was specified to provide useful information at a glance while still
behaving like a dependable appliance. The design priorities were:

- A clear, readable interface for a square 480 × 480 display
- Accurate time with RTC startup, NTP synchronization, and network fallback
- Indoor temperature, humidity, TVOC, and eCO2 measurements
- Current conditions, daily high/low temperatures, and weather-aware icons
- Simple on-device configuration using a rotary encoder and buttons
- Persistent settings that survive restarts and interrupted writes
- Graceful operation when Wi-Fi, sensors, storage, or external services fail
- A modular architecture that can be inspected, tested, and extended

## Features

- **Time and calendar** — local time, date, U.S. federal holiday labels, timezone
  handling, DS3231 RTC support, and NTP synchronization
- **Indoor environment** — AHT20 temperature and humidity measurements plus
  SGP30 TVOC and eCO2 readings with humidity compensation
- **Air-quality trend** — compares rolling TVOC averages and reports improving,
  steady, worsening, or collecting states on-screen and through the encoder LED
- **Network weather** — current temperature, daily high and low, condition code,
  sunrise, sunset, and day/night-aware weather graphics using Open-Meteo data
- **Automatic location** — derives location coordinates and timezone from the
  network connection, then stores the resulting configuration
- **On-device Wi-Fi setup** — scans for networks and supports password entry with
  the rotary encoder without requiring credentials to be compiled into firmware
- **Resilient storage** — redundant EEPROM slots, sequence numbers, CRC checks,
  and commit markers protect configuration and SGP30 baseline records
- **Diagnostics** — firmware version, uptime, heap and PSRAM availability, sensor
  failures, weather failures, input queue overflows, and timing deadline misses
- **Degraded operation** — stale, unavailable, initializing, and fault states are
  represented explicitly instead of silently displaying invalid data

## Spec-Driven AI-Agent Development

This was not developed from a single broad prompt asking an AI to “make a
widget.” The work was guided by concrete specifications and reviewed in small,
verifiable increments.

### 1. Human-owned specification

The human project owner supplied the intent and constraints, including:

- What information the product should display
- Which hardware and peripherals it must support
- How users should navigate and configure the device
- Timing, persistence, reliability, and failure-handling expectations
- Visual and interaction feedback for important states
- The criteria used to decide whether each change was complete

This kept product decisions with the human rather than delegating them to the
model.

### 2. Agent-created implementation plan

The AI coding agent inspected the existing repository and hardware environment,
decomposed the specification into services, identified dependencies, and planned
changes before editing code. Larger requirements were divided into focused tasks
such as display initialization, sensor scheduling, network operations, storage,
input handling, UI state, and rendering.

### 3. Incremental implementation

The agent implemented one bounded behavior at a time and integrated it through
well-defined interfaces. The resulting firmware is separated into:

| Component | Responsibility |
| --- | --- |
| `Application` | Startup, scheduling, service coordination, health, and rendering |
| `BoardServices` | Qualia display, PSRAM framebuffer, I2C, buttons, and backlight |
| `EnvironmentalService` | AHT20/SGP30 sampling, compensation, baselines, and trends |
| `NetworkWeatherService` | Wi-Fi, secure HTTP, IP location, weather, and retries |
| `TimeService` | RTC, system time, timezone, NTP, and fallback synchronization |
| `StorageService` | Versioned, CRC-protected configuration and baseline persistence |
| `InputService` | Encoder, buttons, debouncing, long presses, queueing, and LED state |
| `UiController` | Screen navigation, editable settings, Wi-Fi setup, and reset flow |
| `Renderer` | Main dashboard, settings screens, icons, status, and diagnostics |

### 4. Verification and refinement

After each implementation step, the agent checked the code against the stated
behavior, compiled the firmware, and used build output and device observations to
guide corrections. The human remained responsible for physical-hardware testing,
visual judgment, requirement changes, and final acceptance. Feedback from the
running device was converted into tighter specifications for the next iteration.

This workflow made the AI agent an implementation and review tool—not the product
owner. The specification remained the source of truth, and generated code still
had to compile, integrate with the real hardware, and satisfy observable behavior.

## Architecture

The main Arduino loop is intentionally small and non-blocking. Services maintain
their own schedules and publish typed snapshots with quality and revision data.
The application redraws only when a snapshot or UI state changes, while network
operations run in a separate FreeRTOS task so sensor and input timing remain
responsive.

Reliability is handled throughout the design rather than in one error handler:

- Sensor values carry explicit quality states and become stale after timeouts.
- Wi-Fi reconnects use bounded backoff instead of blocking the main loop.
- HTTPS requests validate server certificate chains.
- Weather responses are checked for missing and out-of-range values.
- Configuration writes use alternating records and are verified before commit.
- SGP30 baselines are restored only when they match the sensor and are recent.
- A PSRAM-backed canvas reduces visible tearing during full-screen updates.

## Hardware

The firmware targets the following core hardware:

- Adafruit Qualia ESP32-S3 for RGB-666 displays
- 480 × 480 RGB display
- AHT20 temperature and humidity sensor
- SGP30 TVOC and eCO2 sensor
- DS3231 real-time clock
- Adafruit seesaw rotary encoder with NeoPixel
- 4 KB I2C EEPROM

## Software Stack

- PlatformIO
- Arduino framework for ESP32-S3
- Arduino_GFX
- Adafruit AHTX0
- Adafruit SGP30
- Adafruit RTClib
- Adafruit seesaw
- ArduinoJson
- ESP32 Wi-Fi, secure HTTP, and FreeRTOS APIs

Dependency versions and the target board are pinned in `platformio.ini` for
repeatable builds.

## Building

1. Install [Visual Studio Code](https://code.visualstudio.com/) and the
   [PlatformIO extension](https://platformio.org/install/ide?install=vscode).
2. Clone this repository and open its root directory in Visual Studio Code.
3. Allow PlatformIO to install the pinned platform and library dependencies.
4. Build the `adafruit_qualia_s3_rgb666` environment:

   ```sh
   pio run -e adafruit_qualia_s3_rgb666
   ```

5. Connect the device and upload the firmware:

   ```sh
   pio run -e adafruit_qualia_s3_rgb666 -t upload
   ```

The current firmware version is defined by `DESK_WIDGET_VERSION` in
`platformio.ini`.

## Repository Layout

```text
include/        Service interfaces, shared types, and configuration constants
src/            Firmware implementation and Arduino entry point
scripts/        PlatformIO support scripts
test/           Project test area
platformio.ini  Board, framework, build flags, and pinned dependencies
```

## Development Philosophy

DeskWidget demonstrates that AI-assisted development works best when the human
provides precise intent and retains engineering judgment. Clear specifications,
small implementation steps, explicit interfaces, repeatable builds, and testing
on real hardware turn generated code into a maintainable project rather than a
one-off prototype.

## License

This project is available under the terms in [LICENSE](LICENSE).
