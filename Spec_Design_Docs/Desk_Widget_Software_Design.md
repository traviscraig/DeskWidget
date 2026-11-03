# 29. Software Design

## 29.1 Purpose and Design Basis

This section defines the software architecture and observable behavior of the Raccoon Works Desk Widget. It supports implementation by a mid-level embedded developer while permitting reasonable choices of classes, task structure, graphics backend, and open-source libraries.

The software operates on the Adafruit Qualia ESP32-S3 with a 3.4-inch 480 × 480 RGB TTL display, seesaw rotary encoder with push button, two Qualia user buttons, AHT20, SGP30, DS3231, and 24LC32 EEPROM. There is no touchscreen dependency.

The established requirements precede the design and remain authoritative. Requirement identifiers referenced here are retained regardless of document section renumbering. This section starts at 29 as requested; integration into the master document must resolve the existing Section 29 heading rather than leave duplicate numbers.

“Shall” describes behavior needed to satisfy the existing requirements. “Recommended” and the proposed numerical targets below describe starting design decisions, not additional approved requirements. The developer may revise them with documented rationale and verification evidence. Hardware-defined timing and protocol constraints remain mandatory.

## 29.2 Development Environment and Libraries

### 29.2.1 Firmware Platform

The recommended implementation is C++ using the Arduino framework for ESP32. Arduino IDE or PlatformIO may be used. ESP-IDF facilities may supplement the Arduino framework for RGB display support, task management, synchronization, timers, and diagnostics.

The repository shall record the board target, ESP32 core version, flash and PSRAM settings, partition scheme, compiler settings, library versions, and programming procedure. Establish a working Qualia/display example before building the application. Pin the validated dependency set rather than updating libraries automatically during a release build.

### 29.2.2 Preferred Library Selection

| Function | Preferred library or facility | Integration guidance |
|---|---|---|
| Drawing primitives, text, fonts | Adafruit_GFX | Preferred application drawing interface; requires a compatible RGB backend or adapter |
| Qualia RGB/ST7701S backend | Verified Arduino_GFX implementation or ESP-IDF RGB driver | Use the selected panel's initialization and timing profile |
| AHT20 | Adafruit_AHTX0 | Convert results into application-owned measurement records |
| SGP30 | Adafruit_SGP30 | Preserve acquisition cadence, humidity units, and baseline lifecycle |
| DS3231 | Adafruit RTClib | Store RTC time in UTC; perform local conversion elsewhere |
| Rotary encoder | Adafruit_seesaw | Normalize rotation direction and button events |
| Qualia board expander | Backend-provided support or compatible PCA9554 driver | Coordinate with display initialization; preserve other output bits |
| 24LC32 | Compatible Adafruit EEPROM support where verified, or another maintained EEPROM driver | Verify 16-bit memory addressing, 32-byte page boundaries, and write completion |
| Shared I²C | Wire; Adafruit BusIO where required | Serialize access and use bounded transactions |
| Wi-Fi/HTTPS | ESP32 framework Wi-Fi, secure client, and HTTP facilities | Keep blocking calls away from input and sensor deadlines |
| JSON decoding | ArduinoJson or equivalent | Bound input size and validate fields before publication |
| Network time | ESP32 SNTP/time facilities | Synchronize asynchronously |
| Optional internal configuration storage | ESP32 Preferences/NVS | Credentials may be stored here through the configuration abstraction |

Adafruit's Qualia Arduino demonstration uses Arduino_GFX and includes a profile for the TL034WVS05-B1477A panel. Arduino_GFX and Adafruit_GFX are separate libraries; similarity of their names does not establish source compatibility. Adafruit_GFX alone does not initialize or refresh this RGB panel. If Adafruit_GFX is adopted, provide an adapter with efficient pixel, rectangle, and bitmap operations. Alternatively, use a verified backend directly behind the application rendering interface and document the choice. [Reference: Adafruit Qualia Arduino Rainbow Demo.]

Open-source libraries are acceptable. Maintain a dependency/license list, retain required notices, and document substitutions. Application code shall not rely on sensor-library objects being accessible throughout the project.

## 29.3 Architecture and Module Responsibilities

### 29.3.1 Logical Modules

| Module | Responsibilities | Published output |
|---|---|---|
| Application coordinator | Startup sequencing, operation mode, service coordination | Application state and commands |
| Board services | Pin mapping, display initialization, expander access, bus ownership | Hardware status and bounded operations |
| Input service | Encoder polling, button debounce, event generation | Normalized input events |
| UI controller | Screen navigation, focus, editing, validation messages | Screen model and user commands |
| Renderer | Layout, formatting, icons, dirty regions, presentation | Visible display updates |
| Environmental service | AHT20 acquisition, SGP30 acquisition/compensation, conditioning | Measurement snapshots |
| Time service | RTC validation, UTC clock, NTP correction, local conversion | Time snapshot and quality |
| Calendar service | Holiday rules and observed-date calculation | Holiday labels for local date |
| Network service | Wi-Fi scanning, connection, reconnection | Connection state and scan results |
| Weather service | Requests, decoding, normalization, cache, stale status | Weather snapshot |
| Configuration service | Validation, active/draft settings, migration, persistence | Configuration revision and save result |
| Diagnostics service | Fault records, counters, timing, reset information | Health snapshot and bounded logs |

Modules may be classes, namespaces, components, or task-owned services. These are logical boundaries, not a prescribed file count.

### 29.3.2 Ownership and Information Flow

Each subsystem owns its mutable state. The UI consumes coherent snapshots and issues commands; it does not perform sensor reads, network requests, or EEPROM writes from drawing functions. Service completion produces an event or revised snapshot. The renderer receives already interpreted state.

Only one execution context shall modify the display at a time. Shared configuration and service snapshots shall be copied, queued, or protected against concurrent access. Do not retain pointers into temporary JSON documents or request buffers.

The bus owner shall coordinate all I²C clients, including expander accesses performed by the display backend. Locking only application sensor calls is insufficient if the backend also accesses Wire independently.

## 29.4 Runtime Scheduling

### 29.4.1 Execution Model

A cooperative scheduler, FreeRTOS tasks, or a hybrid is acceptable. A recommended starting arrangement is an application/UI execution context, a bounded sensor/bus service, and a network worker for potentially blocking DNS, TLS, and HTTP operations.

The essential behavior is that weather requests, Wi-Fi connection attempts, and configuration writes shall not suspend local sensing or menu operation for their entire duration. Selecting FreeRTOS does not by itself guarantee this behavior; library delays, bus locks, priorities, and rendering costs must be measured.

Use monotonic time for intervals, retries, debounce, and data age. Use UTC/calendar time for displayed dates and stored timestamps. NTP adjustments must not shift periodic deadlines. Handle timer rollover safely. Do not run a burst of missed SGP30 acquisitions to pretend a delayed interval was sampled correctly.

### 29.4.2 Proposed Timing Targets

| Activity | Starting target | Behavior |
|---|---|---|
| Input polling | Every 10–20 ms | Read cumulative encoder movement and button state |
| Button debounce | 25–40 ms stable state | Emit one event per accepted transition |
| Long press, if enabled | 800 ms | Suppress duplicate short-press action |
| Input-to-visible response | 100 ms or less | Measure during network and storage activity |
| Clock presentation | Once per second | Repaint time region only when needed |
| AHT20 acquisition | Every 2 s | Bounded acquisition outside renderer |
| SGP30 IAQ acquisition | Once per second | Follow sensor operating guidance |
| Main-screen measurement redraw | On new data, up to 1 Hz | Acquisition and drawing are independent |
| Current weather | Every 10 min | Within the existing 10–15 min preference |
| Weather stale indication | After 30 min without success | Proposed threshold; preserve data age |
| NTP synchronization | On connection, then every 6 h | Interval is configurable in software |
| Health aggregation | Every 1 s | Track progress, faults, and deadline misses |

Record actual sensor-call duration and display-update duration. Prioritize SGP30 deadlines and input service over optional animation. If deadlines are repeatedly missed, revise scheduling or driver use and expose a diagnostic counter.

### 29.4.3 Event and Queue Policy

Suggested events include RotationDelta, Select, Back, Menu, LongPress, SensorUpdated, TimeUpdated, NetworkStateChanged, WeatherUpdated, ConfigurationSaved, and FaultChanged. Names are illustrative.

Use bounded queues. Coalesce rotation deltas and repeated data-refresh notifications when safe. Button releases, cancellation, and save results shall not be silently discarded. Record overflow and recover input state if overflow could leave a control apparently pressed. Callbacks should enqueue or update bounded state rather than render, write EEPROM, or make network requests.

## 29.5 Application Data and Interfaces

### 29.5.1 Data Records

| Record | Minimum information |
|---|---|
| Measurement | Value, unit, quality, last-success monotonic time, optional UTC timestamp |
| Environmental snapshot | Temperature °C, RH %, TVOC ppb, eCO2 ppm, independent validity, compensation state, conditioning state |
| Time snapshot | UTC instant, local date/time, zone/DST information, source, validity, last synchronization |
| Weather snapshot | Location identity, current temperature °C, daily high/low °C, condition, sunrise/sunset UTC, observation time, fetch time, status |
| Configuration | Schema version, revision, units, location, time zone, brightness preference, weather settings, network credential reference |
| Health record | Subsystem, severity, fault code, first/last occurrence, count, recovery state |

Use canonical Celsius and UTC internally; convert for display. Keep relative humidity bounded to 0–100% after validation. Unknown values need an explicit validity state, not a numeric sentinel that can appear as a real reading.

Recommended quality states are unavailable, initializing, valid, stale, and fault. SGP30 conditioning is additional metadata: acquisition success is distinct from a mature baseline. Publish snapshots atomically enough that temperature, unit, and quality agree.

### 29.5.2 Interface Contracts

Each service shall expose initialization, periodic servicing or equivalent task behavior, status retrieval, and documented commands. A command returns immediate rejection or an operation identifier followed by a completion result. Errors shall distinguish unavailable hardware, invalid argument, timeout, busy state, and persistence failure where relevant.

Configuration save receives a validated draft and expected active revision, then returns success with a new revision or a specific failure. Weather refresh receives location/provider configuration and its revision. Results from an obsolete configuration revision shall not replace current data. Rendering receives snapshots and shall not change their ownership.

Exact method signatures are left to implementation. Document whether operations block, their timeout, calling context, and ownership of supplied buffers.

## 29.6 Startup and Operating States

### 29.6.1 Initialization Sequence

1. Capture reset reason and start bounded diagnostics. Never wait indefinitely for a USB serial terminal.
2. Validate configured PSRAM availability and allocate essential resources with checked return values.
3. Initialize board services, I²C, expander, panel initialization, RGB refresh, and usable backlight state.
4. Display startup status as soon as the display is available.
5. Load and validate configuration records; use defaults or a supported migration as appropriate.
6. Initialize inputs and each peripheral independently, with timeouts and per-device results.
7. Validate RTC time and seed the UTC system clock only if trustworthy.
8. Initialize SGP30 and restore an eligible baseline through the sensor service.
9. Enter the main screen or first-run setup. Start sensor deadlines regardless of network availability.
10. Begin Wi-Fi connection asynchronously. After connection, request NTP and weather when their prerequisites are met.

Display setup may itself use I²C. Its initialization shall finish before normal concurrent bus clients begin.

### 29.6.2 Operating State Behavior

The coordinator tracks startup, normal operation, configuration, and degraded operation. Subsystems retain independent states; an unavailable weather service shall not force the environmental service into a failed state.

First-run setup offers network configuration, location, time zone, and units, with a way to continue offline. Missing configuration is expected on a new device. Corrupted storage is reported separately.

A display initialization or essential-memory failure may prevent the normal UI; record the fault on serial and use bounded recovery. Missing sensors, RTC, EEPROM, or Wi-Fi shall allow remaining functions to operate. Avoid repeated restart loops for a missing peripheral.

## 29.7 Display and Graphics Design

### 29.7.1 Backend Integration

The board-specific backend owns panel reset, initialization sequence, RGB pin mapping, signal timing, pixel clock, framebuffer allocation, and refresh configuration. Use the verified profile for the actual 3.4-inch panel, rather than assuming all 480 × 480 ST7701S displays share timings.

RGB refresh and application redraw are separate: hardware refresh continues while the application changes only selected regions. RGB565 is the preferred pixel format. One full-screen buffer requires 460,800 bytes; two require 921,600 bytes, excluding alignment and backend buffers.

Allocate large graphics buffers in suitable PSRAM where supported. Observe backend constraints on DMA-accessible memory, cache synchronization, and internal RAM. Prefer efficient block transfers; a custom Adafruit_GFX adapter that forwards every pixel through an expensive call must be profiled before acceptance.

### 29.7.2 Layout and Rendering

The main screen contains time/date and holiday identification; current weather and condition symbol; daily high/low and sunrise/sunset; indoor temperature/RH; TVOC/eCO2; and concise connection/fault indicators. The exact pixel layout, palette, and font choices remain open, provided all required information is legible at 480 × 480.

Use reusable regions or widgets. Repaint an entire value region when shorter text replaces longer text. Clip drawing to valid bounds. Changing units, screen, theme, or layout can invalidate the full page; routine clock ticks should not.

Render unavailable values as an explicit placeholder and label stale cached data. Do not rely on color alone for state. Weather icons map from normalized conditions and include an unknown fallback. eCO2 shall always retain its equivalent-measurement label. Avoid presenting unsupported air-quality classifications as calibrated health thresholds.

Assets should be compiled or otherwise stored efficiently, with bounded decoding. Additional full-screen canvases are optional and justified by measurements. If tearing is visible, use supported synchronization or buffering without sacrificing input responsiveness.

### 29.7.3 Brightness

Expose brightness through the backend only after verifying the assembled board's actual control capability. Do not assume arbitrary PWM dimming from the presence of a backlight driver. If only enable/disable is supported, report the capability honestly and retain brightness configuration as pending rather than simulate nonexistent levels. Keep an accessible recovery path from a dark display.

## 29.8 Input and Navigation

The seesaw service reads cumulative encoder position and computes movement since the previous sample. Normalize sign using a bench test. Reconcile the initial count after reconnecting so recovery does not create a large false rotation. Read the encoder button and the two board buttons through their appropriate devices.

Default behavior is clockwise next/increase, counterclockwise previous/decrease, encoder press select/confirm, Button 1 back/cancel, and Button 2 menu/home with context indicated onscreen. A held button shall not repeatedly activate actions unless repeat is deliberately defined for that control.

The UI maintains screen identity, focused item, scroll position, and any edit draft. Restore appropriate focus when returning from a child screen. Lists may wrap; numeric values shall clamp to their supported limits. Optional acceleration may help numeric adjustments but should be disabled for individual character selection.

Recommended screens are Main, Settings, Units, Location, Time Zone, Wi-Fi Networks, Credential Entry, Display, Diagnostics, and Reset Confirmation. No touch interaction or host computer is required for normal configuration.

## 29.9 Configuration Workflows

### 29.9.1 Draft, Validation, and Save

Editing creates a draft independent of active configuration. Back/Cancel discards uncommitted changes. Confirm validates the complete affected record and starts persistence; success updates active configuration and notifies affected services. A write failure shall not produce a misleading Saved message.

Brightness preview may take effect temporarily and revert on cancel. Network credentials may be tested before save; retain previous credentials until replacement is committed. Saving a valid network need not depend on Internet weather availability. Distinguish successful association, obtaining an address, and successful Internet transactions.

### 29.9.2 Weather Location and Time Zone

Store weather location as a human-readable label and validated latitude/longitude. Coordinate entry shall work offline. A provider-based place search is optional; ambiguous matches require user selection. Weather location and time zone are separate settings so selecting a remote weather location does not silently change the local clock.

Provide a selectable time-zone list with correct DST rules, or a validated equivalent configuration. A fixed UTC offset alone is inadequate for regions observing DST. Bound stored strings and validate coordinates to latitude −90…90 and longitude −180…180.

### 29.9.3 SSID and Password Entry

Scan asynchronously and show a bounded, deduplicated list with signal/security indications. Support hidden networks through manual SSID entry. Preserve SSID bytes correctly within the supported encoding and length; document any unsupported non-ASCII entry case rather than corrupting a name.

A character picker or onscreen keyboard shall be operated entirely by rotation and the three buttons. Support uppercase, lowercase, digits, printable password symbols, space, deletion, confirmation, and cancellation. Indicate insertion position and remaining capacity. Normally mask the password; an explicit temporary reveal may be provided.

For the initial implementation, support open and WPA/WPA2-Personal networks. Validate applicable credential formats, including 8–63-character passphrases and a 64-hex-digit PSK if supported by the selected API. Identify unsupported enterprise/captive-portal workflows. WPA3 support may be added after confirming the chosen framework and hardware behavior.

## 29.10 Environmental Acquisition

### 29.10.1 AHT20

Use Adafruit_AHTX0 through the environmental service. Validate communication success, finite temperature, and valid RH before publishing. Retain the last good value and its age after a transient failure. A proposed stale threshold is 10 seconds without a successful acquisition.

Repeated failures trigger bounded reinitialization and an independent fault indicator. Recovery automatically replaces stale values. Do not conceal enclosure heating with an arbitrary temperature offset; any correction must be supported by characterization and documented separately.

### 29.10.2 SGP30

Use Adafruit_SGP30 and schedule IAQ acquisition once per second as directed by sensor guidance. Continue acquisition while settings pages are open. Track sensor initialization, initial stabilization, baseline conditioning/restoration, normal operation, and communication fault separately.

The initial fixed outputs following sensor initialization shall not be interpreted as evidence of clean air. Mark startup values as initializing according to the manufacturer's documented interval. Repeated communication failure or sustained cadence loss requires a documented recovery policy, normally reinitialization and a fresh assessment of baseline eligibility.

### 29.10.3 Humidity Compensation

Convert valid AHT20 temperature and RH into absolute humidity using the manufacturer/library example method. The Adafruit Arduino example calculates g/m³ and supplies the compensation value in mg/m³. Validate and convert units explicitly before calling the sensor API; do not pass RH percentage directly as absolute humidity.

Update compensation when a fresh AHT20 measurement is available. If that input becomes stale, expose degraded compensation status and use a documented bounded fallback, such as temporarily retaining the last valid compensation then disabling it through the sensor's supported mechanism. AHT20 failure shall not automatically hide valid SGP30 acquisitions.

### 29.10.4 Baseline Lifecycle

Persist both baseline words, sensor identity, format version, trustworthy capture timestamp, and integrity information. Restore only records matching the sensor and accepted manufacturer age/validity rules. Unknown age due to invalid RTC time shall not be treated as a fresh baseline. Confirm the precise restore sequence and maximum age against the sensor documentation used for the release.

Adafruit guidance calls for approximately 12 hours of operation before the first baseline save when no valid baseline was restored, and approximately hourly persistence once initialized. Measure conditioning duration using monotonic time. A reset or reinitialization must not count as uninterrupted conditioning. Baseline writes are separate from user settings so routine baseline updates do not rewrite credentials or configuration unnecessarily.

## 29.11 Time and Calendar Services

### 29.11.1 UTC and RTC Validation

Maintain DS3231 and internal system time in UTC. Convert to local civil time only for display and calendar evaluation. This avoids storing ambiguous times during DST transitions. If previously deployed RTC data used local time, define a migration/setup path; never guess its interpretation.

At startup inspect RTC communication, lost-power/oscillator validity indication, and calendar plausibility. Do not substitute compilation time as a silently trusted clock. Invalid time produces an explicit unset state; offer manual date/time configuration for offline use or await NTP.

After a valid RTC seeds the system clock, use the system clock for routine ticks and read the RTC periodically as needed. Maintain source/quality state such as unset, manual, RTC-derived, and network-synchronized.

### 29.11.2 Network Synchronization

Synchronize after network connection and periodically thereafter. Validate received time before updating system time and correcting the RTC. A recommended starting RTC correction threshold is 2 seconds; make it a documented policy constant.

Time changes shall trigger reevaluation of local date, DST state, holidays, and sun-event presentation. They shall not restart monotonic sensor/retry schedules. Loss of network changes synchronization status while local timekeeping continues.

If valid clock time is unavailable for certificate validation, allow time synchronization as a separate prerequisite before HTTPS weather retrieval. Do not resolve this bootstrap issue by permanently disabling certificate verification.

### 29.11.3 Holiday Calculation

Calculate supported U.S. federal holidays locally from fixed-date and weekday rules. Keep actual dates and observed dates distinct, with labels such as Observed. The supported set and rule table shall be documented; it must include the federal holidays intended by the requirements rather than an unexplained subset.

Evaluate neighboring calendar years because New Year's observance can fall on December 31 of the preceding year. Define handling when labels coincide. Recalculate on local date change, time-zone change, and significant clock correction. Holiday presentation must not require Internet access.

## 29.12 Wi-Fi and Weather Services

### 29.12.1 Connection Management

Track unconfigured, scanning, connecting, connected, and retry-wait states. Use bounded connection attempts and increasing retry intervals, with a proposed sequence of 5, 15, 30, and 60 seconds and a 5-minute maximum. Reset backoff after a stable connection and allow explicit user retry.

Scanning and connection callbacks shall not draw directly. Differentiate unavailable network, authentication failure where identifiable, and lost connection. Stop or supersede obsolete operations when the user changes credentials.

### 29.12.2 Provider Abstraction

Hide provider endpoints, authentication, parameter names, condition codes, and JSON layout behind a weather adapter. No provider is fixed by this design. Choose one that supplies current conditions, daily high/low, sunrise, and sunset for configured coordinates, and document terms, quotas, credentials, and limitations before integration.

Normalize provider output to the application weather record. Do not assume current conditions and daily forecasts have the same observation timestamp. Associate daily high/low and sun events with the intended date at the weather location, whose time zone may differ from the widget's. If those differ, label dates/times clearly instead of combining mismatched days.

### 29.12.3 Request and Response Handling

Use HTTPS with certificate validation. Bound connection time, total transaction time, response bytes, JSON memory, and redirect count. Proposed starting limits are 5 seconds for connection and 15 seconds overall; response capacity is chosen from measured provider payloads and a documented maximum.

Validate HTTP status, payload length, required fields, numeric types/ranges, coordinates/date association, and timestamps before publishing. Reject malformed or oversized responses without replacing the last good snapshot. An unavailable sunrise/sunset, such as a provider reporting no event, shall have an explicit state.

Handle authentication errors without rapid retries; honor rate-limit retry information where available. Use backoff for transient errors. Cancel or discard results from an obsolete location/provider revision. Do not overlap repeated refresh requests for the same configuration.

### 29.12.4 Cache and Staleness

Keep last valid weather in RAM with fetch age and provider observation time. A new failed request updates error state, not the successful timestamp. Mark weather stale using the proposed threshold in 29.4.2, and invalidate cached daily values when their associated day no longer applies.

Weather persistence across power cycles is optional and shall not result in a write every refresh to the 24LC32. Missing cache after reboot is acceptable until a valid response arrives. An Internet failure shall leave time, local sensors, and controls operational.

## 29.13 Configuration Persistence

### 29.13.1 Storage Layout and Serialization

Use explicit serialization with fixed-width values, defined byte order, bounded strings, and payload length. Do not persist an unversioned raw C++ structure whose padding or pointers can change. Records include magic/type, schema version, sequence, length, payload, and CRC or equivalent integrity check.

A recommended 4 KiB allocation is two 1 KiB configuration slots, two 256-byte baseline slots, and 1,536 bytes reserved. This is a proposed layout; the developer shall measure serialized sizes before adopting it. Unknown future schemas shall not be silently overwritten on boot.

Prefer credentials in a separate NVS-backed record through the same configuration service. NVS storage alone does not prove encryption; if confidentiality at rest is claimed, configure and verify the necessary platform protection. Never log passwords, PSKs, or provider secrets.

### 29.13.2 Power-Loss-Safe Save

Retain the previous valid slot while writing a replacement. Write the inactive slot, read back and verify it, then write a commit marker last. At startup select the newest complete valid record using rollover-aware sequence comparison. A torn write must leave the previous record usable.

The EEPROM driver shall split writes at 32-byte page boundaries and respect transfer-buffer limits. Use acknowledgement polling with a deadline or a documented write-completion method. Release the shared bus while the EEPROM is internally programming so inputs and sensors can continue.

Skip unchanged saves and coalesce deliberate edits. Do not persist encoder movement, clock ticks, raw samples, weather refreshes, or diagnostic counters continuously.

### 29.13.3 Configuration Across Storage Backends

If credentials and general settings use different devices, a save is not automatically atomic. Use matching configuration generation identifiers or a staged manifest so recovery selects a complete consistent pair. Report success only when all parts are committed. Keep previous credential/configuration generations until replacement is recoverable.

If EEPROM is unavailable, allow an explicitly identified temporary RAM configuration; do not imply it will survive restart. Migration shall validate the old record, preserve it until the new record commits, and document supported source versions.

### 29.13.4 Restore Defaults

Require an explicit confirmation screen. Define reset scope: general preferences, weather location/provider settings, and Wi-Fi credentials. Preserve SGP30 calibration by default; provide a separately identified calibration clear action if needed. After completion return to setup or the main screen with accurate save status.

## 29.14 Shared Bus, Fault Handling, and Recovery

Maintain one source of truth for board pins and addresses. Expected device addresses are AHT20 0x38, seesaw nominally 0x36, DS3231 0x68, SGP30 0x58, and EEPROM nominally 0x50; verify actual assembled-board settings and expander address before integration. Keep the board expander behind board services so button reads cannot overwrite display-control outputs.

Use short bounded transactions and avoid holding a global bus lock through an entire sensor conversion delay if the device/backend permits releasing it safely. If a library blocks within an acquisition, account for that duration rather than declaring the call nonblocking. Prefer 400 kHz only after the complete bus is verified.

| Failure | Visible behavior | Recovery |
|---|---|---|
| AHT20 unavailable | Temperature/RH unavailable or aged; compensation degraded | Bounded reinitialize |
| SGP30 unavailable | TVOC/eCO2 unavailable or aged | Reinitialize and reassess conditioning |
| RTC invalid | Time unset until manual/NTP source available | Validate replacement time before correction |
| EEPROM failure | Save failure or temporary-settings indication | Retry deliberate writes; preserve prior slot |
| Wi-Fi loss | Offline indication; local functions continue | Backoff reconnect |
| Weather failure | Last valid data with age/stale indication | Provider-aware retry |
| Shared bus stuck | Affected devices unavailable | Controlled bus recovery and device reinitialize |
| Essential display allocation failure | Serial diagnostic; limited/recovery mode | Bounded retry or controlled restart |

Bus recovery shall coordinate all clients. A stuck bus can affect the onboard buttons and panel initialization; do not promise independent operation of devices on the same failed bus. Recovery may recreate the bus or use a board-appropriate clock/reset procedure only after verifying electrical and backend safety.

Use watchdogs to detect stalled essential execution contexts, not to reboot because an external service is absent. Feed health supervision from evidence of progress. Track repeated resets and provide a reduced startup/recovery mode rather than a permanent boot loop.

## 29.15 Memory, Diagnostics, and Maintenance

Check allocations and bound all queues, network bodies, strings, logs, and asset buffers. Preallocate predictable resources where practical. Track internal heap, largest free block, PSRAM availability, and task stack margins. Do not treat total free PSRAM as proof that TLS or DMA allocations can succeed.

Diagnostics shall include firmware/build identifier, dependency set, uptime, reset cause, subsystem status, last sensor success, time quality, weather age, missed deadlines, queue overflow, save failures, and retry counts. Expose a useful subset on the diagnostics screen and more detail through USB serial. Logs shall be rate-limited and exclude credentials.

USB programming is the baseline update method. OTA updates, data logging, remote provisioning, touch interaction, additional sensors, and automatic brightness are optional future work. The architecture should permit extensions through new services or screens without placing their logic into the renderer.

## 29.16 Verification and Developer Handoff

### 29.16.1 Traceability

| Requirement group | Primary design coverage |
|---|---|
| DW-SW-001–008, 010–014 | Startup, runtime scheduling, diagnostics |
| DW-UI-001–014 | Main-screen model and renderer |
| DW-UI-020–026, 030–034 | Input normalization, navigation, edit workflows |
| DW-ENV-001–010 | Acquisition, validity, compensation, conditioning |
| DW-NET-001–012, 020–029 | Weather adapter, cache, Wi-Fi state machine |
| DW-CFG-001–010, 020–027 | Settings and credential entry |
| DW-CFG-030–036, 040–043 | Serialized storage, migration, baseline lifecycle |
| DW-TIME-001–010, 020–025 | UTC clock, RTC/NTP, local calendar |
| DW-SW-050–056, 060–065 | Fault containment, scheduling, dirty-region updates |
| DW-SW-070–076 | Module boundaries and adapter interfaces |

This table identifies design ownership. The developer shall expand it into requirement-level verification coverage; grouping does not establish that every requirement has been tested.

### 29.16.2 Recommended Implementation Sequence

1. Establish reproducible board/display build and verify PSRAM, panel profile, and backlight capability.
2. Bring up the complete I²C bus, encoder, board buttons, and peripheral examples.
3. Implement canonical data records, time/measurement validity, and local main-screen presentation.
4. Implement navigation, edit drafts, and power-loss-safe storage.
5. Add UTC RTC/NTP handling, time-zone conversion, and holiday rules.
6. Add asynchronous Wi-Fi, provider adapter, stale cache, and error recovery.
7. Integrate baseline persistence, diagnostics, and combined-load verification.

### 29.16.3 Verification Cases

Test pure logic on a host where practical: configuration validation/serialization, CRC rejection, sequence rollover, migration, unit conversion, humidity conversion, character entry, holiday rules, DST boundaries, and weather decoding using representative valid, malformed, missing-field, and oversized responses.

On hardware, verify cold startup, first-run/offline setup, all input controls, required display fields, sensor acquisition, RTC retention, credential entry, network recovery, and successful weather/NTP operation. Inject sensor removal, invalid RTC state, missing EEPROM, interrupted saves, and network timeouts.

Interrupt power at multiple points during configuration/baseline saves and confirm recovery selects a complete valid record. If multiple storage backends are used, interrupt between their commits as well.

Measure input latency and SGP30 intervals while HTTPS requests time out, Wi-Fi scans run, screens change, and EEPROM writes complete. Record timing distributions and maximum observed delay rather than relying only on average loop time.

Run a proposed 24-hour combined-operation test with periodic network outages and screen interaction. Check heap trends, deadline misses, restarts, stale indicators, baseline persistence, and recovery. This does not replace the longer sensor conditioning exercise needed to verify baseline behavior.

### 29.16.4 Delivery Contents and Open Decisions

The implementation handoff shall include source, reproducible build instructions, pinned dependencies and licenses, module/interface notes, storage map and schema, provider configuration instructions, requirement-level verification results, known limitations, and recovery/programming instructions. Keep secrets out of source and provide configuration examples with placeholders.

Resolve and document the following during early implementation: graphics backend/adaptor choice; measured brightness capability; weather provider and credentials; supported time-zone representation; precise sensor baseline eligibility rules; supported network security modes; memory/buffer limits; and any adjusted timing targets. These choices are open within the design contracts and do not require redesigning the entire application.

## 29.17 Technical References

Use current upstream documentation for the dependency versions selected for the release:

- [Adafruit Qualia Arduino Rainbow Demo and panel profiles](https://learn.adafruit.com/adafruit-qualia-esp32-s3-for-rgb666-displays/arduino-rainbow-demo)
- [Adafruit Qualia library installation](https://learn.adafruit.com/adafruit-qualia-esp32-s3-for-rgb666-displays/arduino-library-installation)
- [Adafruit SGP30 Arduino guidance and baseline management](https://learn.adafruit.com/adafruit-sgp30-gas-tvoc-eco2-mox-sensor/arduino-code)
- [Adafruit SGP30 humidity compensation example](https://github.com/adafruit/Adafruit_SGP30/blob/master/examples/sgp30test/sgp30test.ino)

The proposed application policies in this section are design recommendations. Manufacturer guidance and the selected dependency documentation govern device protocol and operating constraints.
