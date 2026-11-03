# Raccoon Works: Desk Widget

## **Purpose**

This document defines the design specification, system requirements, and preliminary design for the **Raccoon Works Desk Widget**.

The Desk Widget is intended to be a compact, self-contained IoT information display suitable for use on an office desk or workbench. The device will combine information obtained from Internet-based services with locally measured environmental data and present the information through an integrated graphical display.

The design shall emphasize reliability, readability, ease of use, maintainability, and the ability to expand the device with additional functionality in future revisions.

# **2\. Design Goals**

The primary goal is to develop a self-contained IoT device capable of continuously displaying useful environmental, weather, and time information without requiring connection to a computer or mobile device during normal operation.

The system shall provide the following major functions.

## **2.1 Local Weather**

The device shall retrieve and display weather information for a configured geographic location.

Displayed weather information shall include:

* Current weather conditions  
* Current outdoor temperature  
* Daily high temperature  
* Daily low temperature  
* Graphical weather-condition symbol  
* Sunrise time  
* Sunset time

Weather information shall be periodically updated from an Internet-accessible weather service.

The device shall retain the most recently received weather information if Internet connectivity is temporarily unavailable.

## **2.2 Indoor Environmental Monitoring**

The device shall measure and display environmental conditions at its installed location.

Measurements shall include:

* Ambient temperature  
* Relative humidity  
* Total Volatile Organic Compounds (TVOC)  
* Equivalent carbon dioxide (eCO2)

Environmental measurements shall be obtained from sensors integrated into the Desk Widget.

## **2.3 Date and Time**

The device shall display:

* Current time  
* Day of week  
* Calendar date  
* Applicable United States holidays

The system shall contain a dedicated Real-Time Clock (RTC).

Network time shall be used to periodically synchronize the RTC when Internet connectivity is available.

The RTC shall maintain date and time information during temporary loss of network connectivity.

The system shall support:

* Configurable time zone  
* Daylight Saving Time  
* Calendar date changes  
* Leap years  
* United States federal holidays

  # 

# **3\. System Requirements**

## **3.1 Processing**

The system shall contain an embedded processor or microcontroller capable of:

* Driving a 480 × 480 pixel graphical display  
* Reading the rotary encoder and push buttons  
* Communicating with environmental sensors  
* Communicating with the RTC  
* Maintaining local date and time  
* Connecting to a Wi-Fi network  
* Performing HTTP/HTTPS communication  
* Parsing weather-service data  
* Maintaining configuration information in nonvolatile memory  
* Rendering the graphical user interface  
* Providing an interactive configuration interface

The processor shall provide sufficient unused memory, storage, GPIO, and processing capacity to support future functionality.

## **3.2 Network Connectivity**

The Desk Widget shall provide 2.4 GHz Wi-Fi connectivity.

Wi-Fi shall be used for:

* Weather data retrieval  
* Network time synchronization  
* Initial and subsequent network configuration  
* Firmware updates, if implemented

The device shall automatically attempt to reconnect following loss of Wi-Fi connectivity.

Loss of network connectivity shall not prevent the device from displaying:

* Locally measured environmental information  
* Date and time maintained by the RTC  
* Previously downloaded weather information

  # 

# **4\. Display Requirements**

## **4.1 Display Size**

The device shall incorporate a nominal **3.4-inch square graphical display**.

## **4.2 Display Resolution**

The display shall have a native resolution of:

**480 × 480 pixels**

The user interface shall be designed specifically for the square 1:1 display aspect ratio.

## **4.3 Display Content**

The graphical interface shall support display of:

* Text  
* Numerical values  
* Weather icons  
* Environmental status information  
* Menus  
* Configuration pages  
* Dialog boxes  
* System status indicators

The display shall be readable at normal desk-viewing distances.

Frequently changing values should be updated without performing a complete display redraw where practical.

# 

# **5\. User Input**

## **5.1 Input Devices**

User input shall be provided through:

* One rotary encoder  
  * Clockwise rotation  
  * Counterclockwise rotation  
  * Integrated push-button input  
* Two standalone momentary push buttons

No touchscreen shall be required for normal operation.

## **5.2 Rotary Encoder Functions**

The rotary encoder shall serve as the primary navigation control.

Rotational input shall normally be used for:

* Moving between menu items  
* Moving between selectable fields  
* Scrolling lists  
* Adjusting numerical values  
* Moving through characters during text entry  
* Changing configuration values

The encoder push button shall normally be used for:

* Selecting a menu item  
* Confirming a selection  
* Opening a selected function  
* Accepting an entered value

  ## **5.3 Standalone Button Functions**

Two standalone push buttons shall provide secondary user input.

Their exact functions may vary depending on the active screen, but their intended primary functions shall be:

* **Button 1 — Back / Cancel**  
* **Button 2 — Menu / Home / Context Function**

The user interface should provide consistent button behavior wherever practical.

Long-press functionality may be implemented for secondary operations.

# **6\. Configuration Interface**

The device shall provide an integrated graphical **Configuration Page** accessible using the physical user controls.

Configuration shall not require firmware recompilation.

The configuration system shall provide, at minimum:

* Wi-Fi network configuration  
* Wi-Fi network selection  
* Wi-Fi password entry  
* Geographic location configuration  
* Time-zone configuration  
* Temperature unit selection  
* Display preferences

## **6.1 Wi-Fi Network Selection**

The configuration interface shall allow the user to initiate a scan for available Wi-Fi networks.

Available networks shall be presented in a scrollable list.

The rotary encoder shall allow the user to scroll through the list.

The encoder push button shall select the highlighted network.

The interface should display an indication of:

* Network name / SSID  
* Signal strength  
* Security status

The system should provide an option to manually enter an SSID for networks that are not broadcast.

## 

## **6.2 Password Entry**

The configuration interface shall provide a mechanism for entering a Wi-Fi password using the rotary encoder and push buttons.

A character-entry interface should provide:

* Uppercase letters  
* Lowercase letters  
* Numbers  
* Common symbols  
* Delete / backspace  
* Confirm  
* Cancel

Password characters should be hidden or masked after entry.

The interface may temporarily show the currently selected or recently entered character to improve usability.

## **6.3 Configuration Storage**

Configuration information shall be stored in nonvolatile memory.

Stored configuration shall survive:

* Normal power removal  
* Software reset  
* Firmware restart

Configuration parameters shall include, at minimum:

* Wi-Fi SSID  
* Wi-Fi credentials  
* Geographic location  
* Time zone  
* Temperature units  
* User display preferences

A method shall be provided to clear or restore configuration settings.

# 

# **7\. Real-Time Clock**

The system shall contain a dedicated hardware RTC.

The RTC shall maintain:

* Seconds  
* Minutes  
* Hours  
* Day  
* Month  
* Year

The RTC should provide battery-backed or otherwise retained timekeeping during removal of primary USB power.

On startup, the system shall obtain its initial time from the RTC.

When Internet connectivity is available, the system shall periodically synchronize time using NTP or an equivalent network time source.

Following successful network synchronization, the system shall update the RTC when necessary.

The RTC subsystem should be designed such that temporary network outages do not cause loss of accurate date and time information.

# **8\. Power**

## **8.1 External Power**

The Desk Widget shall be powered through a **USB Type-C connector**.

The device shall operate from a standard 5 V USB power source.

The design should support operation from common:

* USB-C wall adapters  
* USB-C desktop power supplies  
* USB-A to USB-C cables where electrically appropriate  
* USB-C to USB-C cables

  ## **8.2 USB Type-C Requirements**

If USB Power Delivery is not required, the USB-C interface shall implement the appropriate USB Type-C configuration-channel resistors to request standard 5 V source operation.

The device shall not require USB Power Delivery negotiation for normal operation unless a later power analysis demonstrates that it is necessary.

## **8.3 Power Budget**

The power subsystem shall provide sufficient capacity for simultaneous operation of:

* Processor  
* Wi-Fi radio  
* 480 × 480 display and backlight  
* Environmental sensors  
* RTC  
* User input electronics  
* Any required level translators or peripheral devices

The power supply design shall provide margin for peak current associated with Wi-Fi transmission and display operation.

# **9\. User Interface**

The normal operating screen should be divided into logical information regions optimized for the 480 × 480 square display.

A preliminary arrangement may contain:

## **Header Area**

* Current time  
* Day and date  
* Holiday indication  
* Wi-Fi status

  ## **Weather Area**

* Current outdoor temperature  
* Large weather-condition icon  
* Daily high and low temperatures  
* Sunrise and sunset times

  ## **Indoor Environment Area**

* Indoor temperature  
* Relative humidity  
* TVOC  
* eCO2

  ## **System Status Area**

Status indicators may be provided for:

* Wi-Fi connectivity  
* Weather data availability  
* Time synchronization  
* Sensor status  
* Configuration warnings  
* System faults

# **10\. Menu Structure**

A preliminary menu architecture shall include:

**Main Display**

→ Configuration  
→ Network  
→ Location  
→ Date & Time  
→ Display  
→ System Information

The configuration menu should be accessible using the Menu/Home button.

### **Network**

* Wi-Fi Enable  
* Scan Networks  
* Select Network  
* Enter Password  
* Connection Status  
* IP Address

  ### **Location**

* Weather location  
* Location name  
* Latitude / longitude, if used

  ### **Date & Time**

* Time zone  
* Daylight Saving Time  
* RTC status  
* NTP synchronization status  
* Manual time setting, if required

  ### **Display**

* Brightness  
* Temperature units  
* Screen timeout, if implemented

  ### **System Information**

* Firmware version  
* Hardware revision  
* Network status  
* RTC status  
* Sensor status  
* Device uptime

# 

# **11\. Environmental Sensor System**

The environmental sensor subsystem shall provide the following measurements:

| Parameter | Display Unit |
| ----- | ----- |
| Temperature | °F or °C |
| Relative Humidity | %RH |
| TVOC | ppb |
| eCO2 | ppm |

The design shall account for heat generated internally by:

* Processor  
* Display  
* Display backlight  
* Voltage regulators  
* Wi-Fi radio

Environmental sensors shall be located so that internal heating has minimal influence on measurements.

The enclosure shall provide adequate airflow around the environmental sensing elements.

# 

# **12\. Software Architecture**

The firmware should be divided into independent functional modules.

Recommended modules include:

### **System Manager**

Responsible for:

* Initialization  
* Device state  
* Error handling  
* Watchdog management

  ### **Input Manager**

Responsible for:

* Rotary encoder decoding  
* Encoder push-button handling  
* Push-button handling  
* Debouncing  
* Short-press and long-press detection

  ### **Display Manager**

Responsible for:

* Screen layout  
* Menu rendering  
* Graphics  
* Weather icons  
* Text rendering  
* Display updates

  ### **User Interface Manager**

Responsible for:

* Screen navigation  
* Menu state  
* Configuration dialogs  
* Text-entry interface  
* User interaction state

  ### **Environmental Sensor Manager**

Responsible for:

* Sensor initialization  
* Sensor polling  
* Data validation  
* Filtering  
* Unit conversion

  ### **Weather Service**

Responsible for:

* Weather API communication  
* Response parsing  
* Weather-data storage  
* Update scheduling

  ### **Network Manager**

Responsible for:

* Wi-Fi scanning  
* Wi-Fi connection  
* Reconnection  
* Network configuration  
* Network status

  ### **Time Manager**

Responsible for:

* RTC communication  
* NTP synchronization  
* Time-zone conversion  
* Daylight Saving Time  
* Date handling

  ### **Holiday Manager**

Responsible for:

* Holiday calculations  
* Observed holiday dates  
* Providing holiday descriptions to the display

  ### **Configuration Manager**

Responsible for persistent storage of:

* Network credentials  
* Location  
* Time zone  
* Display options  
* User preferences

# **13\. System Operating States**

The firmware shall support the following minimum operating states:

### **Initialization**

Hardware and software subsystems are initialized.

### **Network Connection**

The system attempts to connect to the configured Wi-Fi network.

### **Normal Operation**

Weather, environmental, date, and time information are displayed.

### **Configuration**

The user can modify system parameters using the rotary encoder and push buttons.

### **Offline Operation**

Local sensors and RTC continue normal operation while previously downloaded weather information remains available.

### **Fault Operation**

Subsystem failures shall be reported without preventing unaffected functions from operating.

# **14\. Data Update Rates**

Initial target update intervals are:

| Data | Target Update Interval |
| ----- | ----- |
| Displayed time | 1 second |
| Indoor temperature | 5–30 seconds |
| Indoor humidity | 5–30 seconds |
| TVOC/eCO2 | Sensor dependent |
| Current weather | 10–15 minutes |
| Daily forecast | 30–60 minutes |
| Sunrise / sunset | Daily |
| NTP synchronization | Several hours |
| RTC synchronization | Following successful NTP synchronization |
| Holiday calculation | Startup and date change |

# **15\. Requirements**

This section defines the hardware and software requirements for the Raccoon Works Desk Widget.

Requirements are identified using the following conventions:

* **DW-HW-xxx** — Hardware requirements  
* **DW-SW-xxx** — Software requirements  
* **DW-UI-xxx** — User-interface requirements  
* **DW-NET-xxx** — Network requirements  
* **DW-TIME-xxx** — Timekeeping requirements  
* **DW-ENV-xxx** — Environmental sensing requirements  
* **DW-CFG-xxx** — Configuration and storage requirements

Each requirement is intended to be independently testable or verifiable.

## 15.1 Hardware Requirements

### 15.1.1 Processing Hardware

**DW-HW-001**  
The system shall use the Adafruit ESP32-S3 Qualia board as the primary processing platform.

**DW-HW-002**  
The processor shall provide Wi-Fi capability for network connectivity.

**DW-HW-003**  
The processor shall provide sufficient RAM and nonvolatile storage to support the graphical user interface, networking, configuration storage, and sensor-processing functions.

**DW-HW-004**  
The processor shall support direct control of the selected parallel RGB TFT display.

**DW-HW-005**  
The system shall provide sufficient processing capability to update the graphical interface without preventing periodic sensor acquisition or network communications.

## 

### 15.1.2 Display Hardware

**DW-HW-010**  
The system shall use a nominal 3.4-inch square TFT display.

**DW-HW-011**  
The display shall provide a native resolution of 480 × 480 pixels.

**DW-HW-012**  
The display shall use a parallel RGB TTL interface.

**DW-HW-013**  
The display shall support at least 16-bit color representation.

**DW-HW-014**  
The display backlight shall be powered from the system power architecture.

**DW-HW-015**  
The system should provide software-controllable display brightness.

### 15.1.3 User Input Hardware

**DW-HW-020**  
The system shall include one rotary encoder with an integrated push-button function.

**DW-HW-021**  
The rotary encoder shall interface to the primary processor using I²C.

**DW-HW-022**  
The system shall provide two standalone user push buttons.

**DW-HW-023**  
The two standalone push buttons shall use the user buttons available on the Qualia board.

**DW-HW-024**  
The rotary encoder shall support clockwise and counterclockwise rotational input.

**DW-HW-025**  
The rotary encoder shall support push-button input.

### 15.1.4 Environmental Sensor Hardware

**DW-HW-030**  
The system shall include an AHT20 temperature and relative-humidity sensor.

**DW-HW-031**  
The AHT20 shall communicate with the primary processor using I²C.

**DW-HW-032**  
The system shall include an SGP30 indoor air-quality sensor.

**DW-HW-033**  
The SGP30 shall communicate with the primary processor using I²C.

**DW-HW-034**  
The environmental sensors shall be positioned such that internal system heating does not significantly bias room-environment measurements.

**DW-HW-035**  
The enclosure shall provide airflow to the environmental sensors.

### 15.1.5 Real-Time Clock Hardware

**DW-HW-040**  
The system shall include a DS3231 real-time clock.

**DW-HW-041**  
The DS3231 shall communicate with the processor using I²C.

**DW-HW-042**  
The RTC shall retain date and time during loss of primary USB power.

**DW-HW-043**  
The RTC shall include an appropriate backup power source.

### 15.1.6 Nonvolatile Configuration Storage

**DW-HW-050**  
The system shall include a 24LC32 I²C EEPROM.

**DW-HW-051**  
The EEPROM shall provide nonvolatile storage for system configuration and calibration information.

**DW-HW-052**  
The EEPROM shall retain stored information following removal of primary power.

**DW-HW-053**  
The EEPROM shall share the system I²C bus with the environmental sensors, RTC, and rotary encoder.

### 15.1.7 Communication Bus

**DW-HW-060**  
The system shall provide a common I²C bus for low-speed peripheral devices.

**DW-HW-061**  
All devices connected to the I²C bus shall use unique non-conflicting addresses.

**DW-HW-062**  
The I²C bus should operate at 400 kHz when supported by all connected devices.

**DW-HW-063**  
The design shall account for aggregate I²C pull-up resistance and bus capacitance.

### 15.1.8 Power Hardware

**DW-HW-070**  
The system shall receive primary power through a USB Type-C connector.

**DW-HW-071**  
The nominal system input voltage shall be 5 VDC.

**DW-HW-072**  
The system shall not require USB Power Delivery negotiation for normal operation.

**DW-HW-073**  
The power architecture shall support simultaneous operation of the processor, display, Wi-Fi radio, RTC, rotary encoder, EEPROM, and environmental sensors.

**DW-HW-074**  
The power architecture shall provide sufficient current margin for peak Wi-Fi activity and maximum display-backlight load.

**DW-HW-075**  
The system shall operate from a standard USB power adapter capable of supplying the required current.

# 

## 15.2 Software Requirements

### 15.2.1 System Initialization

**DW-SW-001**  
The software shall initialize all required hardware interfaces at startup.

**DW-SW-002**  
The software shall initialize the display during system startup.

**DW-SW-003**  
The software shall initialize the rotary encoder and push-button inputs during system startup.

**DW-SW-004**  
The software shall initialize all environmental sensors during system startup.

**DW-SW-005**  
The software shall initialize the RTC during system startup.

**DW-SW-006**  
The software shall read stored configuration information during startup.

**DW-SW-007**  
The software shall identify invalid or missing configuration data and enter an appropriate configuration state.

**DW-SW-008**  
The system should display useful status information while startup operations are being completed.

### 15.2.2 Main Application Operation

**DW-SW-010**  
The software shall support continuous unattended operation.

**DW-SW-011**  
The software shall update displayed information without requiring a device restart.

**DW-SW-012**  
A failure in one subsystem shall not prevent unrelated subsystems from continuing to operate where practical.

**DW-SW-013**  
The software shall periodically service network, sensor, user-interface, display, and timekeeping functions.

**DW-SW-014**  
The software should use a watchdog mechanism to recover from unrecoverable firmware faults.

## 15.3 Display and User Interface Requirements

### 15.3.1 Main Screen

**DW-UI-001**  
The system shall provide a primary operating screen.

**DW-UI-002**  
The primary operating screen shall display the current local time.

**DW-UI-003**  
The primary operating screen shall display the current date.

**DW-UI-004**  
The primary operating screen shall display the current outdoor weather condition.

**DW-UI-005**  
The primary operating screen shall display the current outdoor temperature.

**DW-UI-006**  
The primary operating screen shall display the daily high temperature.

**DW-UI-007**  
The primary operating screen shall display the daily low temperature.

**DW-UI-008**  
The primary operating screen shall display weather-condition symbology.

**DW-UI-009**  
The primary operating screen shall display sunrise time.

**DW-UI-010**  
The primary operating screen shall display sunset time.

**DW-UI-011**  
The primary operating screen shall display indoor temperature.

**DW-UI-012**  
The primary operating screen shall display indoor relative humidity.

**DW-UI-013**  
The primary operating screen shall display TVOC.

**DW-UI-014**  
The primary operating screen shall display eCO2.

### 15.3.2 User Controls

**DW-UI-020**  
Clockwise rotation of the rotary encoder shall provide positive menu or value navigation.

**DW-UI-021**  
Counterclockwise rotation of the rotary encoder shall provide negative menu or value navigation.

**DW-UI-022**  
Pressing the rotary encoder shall select or confirm the currently highlighted option.

**DW-UI-023**  
One standalone button shall provide a Back or Cancel function.

**DW-UI-024**  
One standalone button shall provide a Menu, Home, or context-sensitive secondary function.

**DW-UI-025**  
Button and encoder inputs shall be debounced in software or hardware.

**DW-UI-026**  
The firmware may distinguish between short-press and long-press button events.

### 15.3.3 Menu Operation

**DW-UI-030**  
The software shall provide a configuration menu accessible from the normal operating screen.

**DW-UI-031**  
The configuration menu shall be navigable without a touchscreen.

**DW-UI-032**  
The system shall provide a visible indication of the currently selected menu item.

**DW-UI-033**  
The user shall be able to exit a configuration screen without saving changes.

**DW-UI-034**  
The user shall be able to confirm and save modified configuration values.

## 15.4 Environmental Monitoring Requirements

**DW-ENV-001**  
The system shall measure indoor temperature.

**DW-ENV-002**  
The system shall measure indoor relative humidity.

**DW-ENV-003**  
The system shall measure or calculate TVOC using the SGP30.

**DW-ENV-004**  
The system shall calculate eCO2 using the SGP30.

**DW-ENV-005**  
The user interface shall identify the SGP30 output as eCO2 and shall not represent it as a direct CO2 measurement.

**DW-ENV-006**  
The software shall provide AHT20 humidity data to the SGP30 compensation function when supported by the sensor interface.

**DW-ENV-007**  
Indoor temperature and humidity values shall be periodically refreshed.

**DW-ENV-008**  
Air-quality values shall be updated at a rate consistent with SGP30 operating requirements.

**DW-ENV-009**  
Invalid sensor readings shall not be displayed as valid measurements.

**DW-ENV-010**  
The user interface shall provide an unavailable or fault indication when valid environmental data cannot be obtained.

# 

## 15.5 Weather Service Requirements

**DW-NET-001**  
The system shall obtain outdoor weather data using an Internet-based weather service.

**DW-NET-002**  
Weather data shall include current outdoor temperature.

**DW-NET-003**  
Weather data shall include daily high temperature.

**DW-NET-004**  
Weather data shall include daily low temperature.

**DW-NET-005**  
Weather data shall include a weather-condition value suitable for selecting graphical symbology.

**DW-NET-006**  
Weather data shall include or allow calculation of sunrise time.

**DW-NET-007**  
Weather data shall include or allow calculation of sunset time.

**DW-NET-008**  
The software shall periodically refresh weather data.

**DW-NET-009**  
The nominal current-weather update interval should be between 10 and 15 minutes.

**DW-NET-010**  
The software shall retain the most recently valid weather information following temporary loss of Internet connectivity.

**DW-NET-011**  
The user interface shall indicate when displayed weather information is stale or cannot be updated.

**DW-NET-012**  
A failed weather-service request shall not cause the system to stop normal local operation.

# 

## 15.6 Wi-Fi Requirements

**DW-NET-020**  
The system shall support connection to 2.4 GHz Wi-Fi networks.

**DW-NET-021**  
The device shall support scanning for available Wi-Fi networks.

**DW-NET-022**  
Available Wi-Fi networks shall be displayed to the user in a selectable list.

**DW-NET-023**  
The configuration interface shall allow the user to select a detected network.

**DW-NET-024**  
The configuration interface shall allow manual entry of a Wi-Fi SSID.

**DW-NET-025**  
The configuration interface shall allow entry of a Wi-Fi password.

**DW-NET-026**  
The device shall store the selected Wi-Fi configuration in nonvolatile memory.

**DW-NET-027**  
The device shall automatically attempt to reconnect following loss of Wi-Fi connectivity.

**DW-NET-028**  
Loss of Wi-Fi connectivity shall not prevent local environmental sensing.

**DW-NET-029**  
Loss of Wi-Fi connectivity shall not prevent RTC-based time display.

# 

## 15.7 Configuration Requirements

**DW-CFG-001**  
The device shall provide an on-device graphical configuration page.

**DW-CFG-002**  
The configuration page shall be operable using only the rotary encoder and the two standalone buttons.

**DW-CFG-003**  
Configuration shall not require firmware recompilation.

**DW-CFG-004**  
The configuration interface shall allow selection of temperature units.

**DW-CFG-005**  
The configuration interface shall allow configuration of the weather location.

**DW-CFG-006**  
The configuration interface shall allow configuration of the local time zone.

**DW-CFG-007**  
The configuration interface shall allow Wi-Fi network selection.

**DW-CFG-008**  
The configuration interface shall allow Wi-Fi password entry.

**DW-CFG-009**  
The configuration interface should allow adjustment of display brightness.

**DW-CFG-010**  
The configuration interface shall provide a method for restoring default configuration values.

## 

## **15.8 Password Entry**

**DW-CFG-020**  
The password-entry interface shall support uppercase letters.

**DW-CFG-021**  
The password-entry interface shall support lowercase letters.

**DW-CFG-022**  
The password-entry interface shall support numeric characters.

**DW-CFG-023**  
The password-entry interface shall support common Wi-Fi password symbols.

**DW-CFG-024**  
The password-entry interface shall provide a backspace or delete function.

**DW-CFG-025**  
The password-entry interface shall provide a Confirm function.

**DW-CFG-026**  
The password-entry interface shall provide a Cancel function.

**DW-CFG-027**  
Previously entered password characters should be masked during normal display.

## 15.9. Nonvolatile Storage Requirements

**DW-CFG-030**  
User configuration shall persist through power cycles.

**DW-CFG-031**  
Configuration data stored in the 24LC32 shall include a structure or format version identifier.

**DW-CFG-032**  
Stored configuration data shall include a validity or integrity check.

**DW-CFG-033**  
The software shall detect invalid or corrupted configuration data.

**DW-CFG-034**  
The software shall use safe default values when stored configuration is invalid.

**DW-CFG-035**  
The firmware shall avoid unnecessary repeated EEPROM writes.

**DW-CFG-036**  
Frequently changing temporary runtime information shall not be continuously written to EEPROM.

### 15.9.1 SGP30 Baseline Storage

**DW-CFG-040**  
The firmware shall support storing SGP30 baseline information in nonvolatile memory.

**DW-CFG-041**  
The firmware shall restore a valid stored SGP30 baseline following startup when appropriate.

**DW-CFG-042**  
Stored SGP30 baseline data shall include sufficient information to determine whether the stored baseline is valid for reuse.

**DW-CFG-043**  
The firmware shall not write SGP30 baseline information more frequently than required by the sensor-management strategy.

## 15.10 Timekeeping Requirements

**DW-TIME-001**  
The DS3231 shall provide the local time source immediately following startup.

**DW-TIME-002**  
The software shall obtain network time when Internet connectivity is available.

**DW-TIME-003**  
Network time shall be used to periodically correct the system clock.

**DW-TIME-004**  
The DS3231 shall be updated following successful network synchronization when correction is required.

**DW-TIME-005**  
The system shall continue displaying time during loss of network connectivity.

**DW-TIME-006**  
The system shall support configuration of the local time zone.

**DW-TIME-007**  
The system shall support Daylight Saving Time where applicable.

**DW-TIME-008**  
The system shall correctly handle leap years.

**DW-TIME-009**  
The system shall correctly handle month and year transitions.

**DW-TIME-010**  
The displayed time shall update at least once per second.

## 15.11 Holiday Requirements

**DW-TIME-020**  
The software shall determine applicable United States federal holidays.

**DW-TIME-021**  
The user interface shall display a holiday indication when the current date corresponds to a supported holiday.

**DW-TIME-022**  
The holiday algorithm shall support holidays that occur on fixed calendar dates.

**DW-TIME-023**  
The holiday algorithm shall support holidays defined by weekday-based rules.

**DW-TIME-024**  
The holiday algorithm shall support observed holiday dates where applicable.

**DW-TIME-025**  
Holiday information shall be recalculated following a date change.

# 

## 15.12 Fault Handling Requirements

**DW-SW-050**  
Failure to obtain weather data shall not prevent display of environmental measurements.

**DW-SW-051**  
Failure of an environmental sensor shall not prevent display of weather or time data.

**DW-SW-052**  
Loss of network connectivity shall not require a system restart.

**DW-SW-053**  
The software shall automatically retry failed network operations.

**DW-SW-054**  
The user interface shall provide a visible indication of major subsystem faults.

**DW-SW-055**  
The software shall avoid displaying invalid numerical values as valid data.

**DW-SW-056**  
The device should recover automatically from temporary peripheral communication failures where practical.

## 15.13 Performance Requirements

**DW-SW-060**  
Rotary encoder input shall provide visibly responsive menu navigation.

**DW-SW-061**  
Button presses shall provide visible user-interface response without noticeable delay.

**DW-SW-062**  
Normal display updates shall not visibly interfere with rotary encoder or button operation.

**DW-SW-063**  
Wi-Fi transactions shall not prevent normal user-interface operation for the duration of a network request.

**DW-SW-064**  
Environmental sensing shall continue while the device is connected to a weather service.

**DW-SW-065**  
The graphical user interface should avoid unnecessary full-screen redraws where partial updates are practical.

## 15.14 Maintainability and Expansion Requirements

**DW-SW-070**  
The firmware shall be divided into logical software modules.

**DW-SW-071**  
The display-management implementation shall be separated from sensor acquisition logic.

**DW-SW-072**  
Network communication shall be separated from user-interface logic.

**DW-SW-073**  
Configuration storage shall be implemented through a defined software interface rather than direct EEPROM accesses throughout the application.

**DW-SW-074**  
Hardware-specific drivers should be isolated from higher-level application logic.

**DW-SW-075**  
The software architecture should allow additional display pages to be added without redesigning the core application.

**DW-SW-076**  
The software architecture should allow additional I²C sensors to be incorporated where address space, bandwidth, and hardware resources permit.

# 16\. Preliminary Verification Requirements

The following system-level verification activities shall be performed.

**DW-VER-001**  
Verify successful startup from an unpowered state.

**DW-VER-002**  
Verify operation from a standard 5 V USB-C power source.

**DW-VER-003**  
Verify correct display initialization and operation at 480 × 480 resolution.

**DW-VER-004**  
Verify clockwise and counterclockwise rotary encoder operation.

**DW-VER-005**  
Verify encoder push-button operation.

**DW-VER-006**  
Verify operation of both standalone user buttons.

**DW-VER-007**  
Verify valid AHT20 temperature and humidity acquisition.

**DW-VER-008**  
Verify valid SGP30 TVOC and eCO2 acquisition.

**DW-VER-009**  
Verify DS3231 timekeeping following loss of primary power.

**DW-VER-010**  
Verify configuration retention following power cycling.

**DW-VER-011**  
Verify Wi-Fi network scanning.

**DW-VER-012**  
Verify Wi-Fi network selection.

**DW-VER-013**  
Verify password entry using the rotary encoder and buttons.

**DW-VER-014**  
Verify successful weather-service communication.

**DW-VER-015**  
Verify correct weather data display.

**DW-VER-016**  
Verify NTP synchronization.

**DW-VER-017**  
Verify DS3231 synchronization from network time.

**DW-VER-018**  
Verify operation following temporary loss of Wi-Fi connectivity.

**DW-VER-019**  
Verify continued environmental sensing during network loss.

**DW-VER-020**  
Verify continued RTC-based timekeeping during network loss.

**DW-VER-021**  
Verify recovery after weather-service communication failure.

**DW-VER-022**  
Verify successful display of supported United States holidays.

**DW-VER-023**  
Verify stable continuous operation during an extended-duration test.

# **17\. Hardware Design**

## **17.1 Hardware Architecture**

The Desk Widget hardware architecture is based on the **Adafruit Qualia ESP32-S3 for TTL RGB-666 Displays** development board.

The Qualia board shall serve as the primary processing, display-control, network-connectivity, and power-interface platform.

The major hardware elements are:

| Function | Selected Device |
| ----- | ----- |
| Main Processor / Display Controller | Adafruit Qualia ESP32-S3 |
| Display | 3.4-inch 480 × 480 RGB TTL TFT |
| Primary User Input | Adafruit I2C STEMMA QT Rotary Encoder |
| Secondary User Input | Two onboard Qualia push buttons |
| Temperature / Humidity | AHT20 |
| Air Quality | SGP30 |
| Real-Time Clock | DS3231 |
| Configuration Storage | 24LC32 EEPROM |
| Primary Power Interface | USB Type-C |

The system shall use the Qualia board's shared I²C interface for the rotary encoder, environmental sensors, RTC, and configuration EEPROM wherever practical.

## 

## **17.2 System Block Diagram** 

# **![][image1]**

# **18\. Main Processor**

## **18.1 Adafruit Qualia ESP32-S3**

The selected main processing board is the **Adafruit Qualia ESP32-S3 for TTL RGB-666 Displays**, Adafruit Product 5800\.

Reference:

[https\://www\.adafruit.com/product/5800](https://www.adafruit.com/product/5800)

The Qualia board integrates an ESP32-S3 processor with the support circuitry required to directly interface with high-resolution RGB TTL displays.

The selected board provides:

* ESP32-S3 microcontroller  
* Wi-Fi connectivity  
* 16 MB Flash memory  
* 8 MB Octal PSRAM  
* Native RGB display peripheral  
* 40-pin RGB TTL display connector  
* Display backlight driver  
* USB Type-C connector  
* Native USB programming interface  
* STEMMA QT / Qwiic I²C connection  
* PCA9554 I/O expander  
* Two onboard user push buttons

The ESP32-S3's RGB LCD peripheral shall continuously transfer framebuffer data to the display using DMA.

The onboard PSRAM shall provide storage for display framebuffers, graphics resources, icons, and other application data.

The Qualia board provides 16 RGB data signals plus horizontal sync, vertical sync, data-enable, and pixel-clock signals for the display interface.

The Qualia board uses an onboard I/O expander because the parallel display interface consumes a significant portion of the available ESP32-S3 GPIO resources. The expander also supports the two onboard user buttons and display-control functionality.

### 

### **18.1.1 Processor Responsibilities**

The ESP32-S3 shall be responsible for:

* Graphical user-interface rendering  
* Display refresh  
* Rotary encoder processing  
* Push-button processing  
* Environmental sensor acquisition  
* Air-quality calculations and display  
* RTC communication  
* Wi-Fi management  
* Weather-service communication  
* NTP synchronization  
* Configuration management  
* Holiday calculation  
* System diagnostics  
* Fault recovery

### **18.1.2 Memory Considerations**

A 480 × 480 framebuffer requires significant memory.

At 16-bit RGB565 color depth:

480 × 480 × 2 bytes \= **460,800 bytes**

Therefore, a single full-screen framebuffer requires approximately **450 KiB**.

Two complete framebuffers would require approximately **900 KiB**.

The Qualia board's 8 MB PSRAM provides sufficient capacity for framebuffer storage while leaving additional memory available for:

* Icons  
* Fonts  
* Weather graphics  
* Menu assets  
* Network data  
* JSON weather responses  
* Application buffers

# 

# **19\. Display**

## **19.1 3.4-inch RGB TTL TFT**

The selected display is the **3.4-inch Square RGB TTL TFT Display**, Adafruit Product 5825\.

Reference:

[https\://www\.adafruit.com/product/5825](https://www.adafruit.com/product/5825)

The display characteristics are:

* Diagonal size: 3.4 inches  
* Resolution: 480 × 480 pixels  
* Aspect ratio: 1:1  
* Display technology: IPS TFT  
* Interface: Parallel RGB TTL  
* Display controller: ST7701S  
* No touchscreen

The display uses the ST7701S controller and requires both:

* SPI-based initialization  
* Continuous RGB pixel data

SPI shall be used only for initialization and configuration of the display controller.

Normal display graphics shall be transmitted through the parallel RGB interface.

The display is specifically compatible with the class of RGB TTL displays supported by the Qualia ESP32-S3 board.

## **19.2 Display Color Interface**

The display interface is commonly described as RGB-666; however, the Qualia implementation drives the panel using a 16-bit RGB data arrangement consisting of:

* 5 red data bits  
* 6 green data bits  
* 5 blue data bits

The application graphics subsystem may therefore use RGB565 as its primary framebuffer format.

## **19.3 Display Backlight**

The Qualia board incorporates a dedicated constant-current display backlight driver.

Display brightness shall be software controllable.

The firmware should support configurable brightness levels.

Future versions may implement automatic brightness adjustment using an ambient-light sensor.

# 

# **20\. User Input System**

## **20.1 Rotary Encoder**

The primary user-control device shall be the **Adafruit I2C STEMMA QT Rotary Encoder Breakout with Encoder**, Adafruit Product 5880\.

Reference:

[https\://www\.adafruit.com/product/5880](https://www.adafruit.com/product/5880)

The rotary encoder board incorporates an Adafruit seesaw controller that performs local encoder decoding and push-button monitoring.

The ESP32-S3 shall communicate with the encoder over I²C.

The default encoder I²C address is:

**0x36**

The rotary encoder shall provide:

* Clockwise rotation  
* Counterclockwise rotation  
* Integrated push-button input

The seesaw implementation reduces the real-time GPIO and interrupt requirements placed on the ESP32-S3 because encoder pulse tracking is handled by the encoder-board controller.

### **20.1.1 Encoder Functions**

The rotary encoder shall serve as the primary menu-navigation device.

Rotation shall support:

* Menu navigation  
* List scrolling  
* Character selection  
* Parameter adjustment  
* Numerical adjustment

The encoder push button shall support:

* Selection  
* Confirmation  
* Menu entry  
* Field activation

## **20.2 Secondary Buttons**

The two user push buttons incorporated on the Qualia board shall be used as the two required standalone buttons.

Their primary software functions shall initially be assigned as:

**Button 1**

* Back  
* Cancel  
* Return to previous screen

**Button 2**

* Menu  
* Home  
* Context-specific secondary function

Button behavior shall remain consistent throughout the user interface wherever possible.

Both short-press and long-press events may be supported by firmware.

# **21\. Temperature and Humidity Sensor**

## **21.1 AHT20**

The selected temperature and relative-humidity sensor is the **Adafruit AHT20 Temperature & Humidity Sensor Breakout**, Adafruit Product 4566\.

Reference:

[https\://www\.adafruit.com/product/4566](https://www.adafruit.com/product/4566)

The AHT20 communicates using I²C.

The default I²C address is:

**0x38**

Nominal sensor performance includes approximately:

* ±0.3 °C typical temperature accuracy under normal indoor conditions  
* ±2% RH typical humidity accuracy under normal indoor conditions

The sensor shall provide measurements for:

* Indoor ambient temperature  
* Indoor relative humidity

The measured humidity and temperature shall additionally be made available to the air-quality subsystem for SGP30 humidity compensation.

## **21.2 Sensor Placement**

Because the Desk Widget contains several internal heat sources, including:

* ESP32-S3 processor  
* LCD panel  
* LCD backlight  
* Voltage regulators  
* Wi-Fi radio

the AHT20 shall be physically positioned to minimize thermal influence from the electronics.

The enclosure design shall provide:

* Ventilation openings near the sensor  
* Separation from the display backlight  
* Separation from voltage regulators  
* Minimal direct conduction from the processor PCB

Where practical, the sensor should be positioned near an external enclosure vent.

# 

# **22\. Air-Quality Sensor**

## **22.1 SGP30**

The selected air-quality sensor is the **Adafruit SGP30 Air Quality Sensor Breakout**, Adafruit Product 3709\.

Reference:

[https\://www\.adafruit.com/product/3709](https://www.adafruit.com/product/3709)

The SGP30 is a metal-oxide gas sensor intended for indoor air-quality monitoring.

The device communicates over I²C and provides calculated values for:

* Total Volatile Organic Compounds — TVOC  
* Equivalent Carbon Dioxide — eCO2

The system shall display:

* TVOC in ppb  
* eCO2 in ppm

The SGP30 eCO2 value is an estimated equivalent CO2 value derived from gas-sensor measurements. It shall not be described in the user interface or design documentation as a direct or true CO2 measurement.

## **22.2 Humidity Compensation**

The SGP30 supports external humidity compensation.

Temperature and relative-humidity measurements from the AHT20 shall therefore be supplied to the SGP30 environmental-compensation algorithm.

This pairing is intentional and should improve the stability of the reported TVOC and eCO2 values compared with operating the SGP30 without environmental compensation.

## **22.3 Air-Quality Sensor Baseline**

The SGP30 maintains baseline calibration values used by its internal air-quality algorithm.

Firmware shall periodically retrieve these baseline values after the sensor has completed its required conditioning period.

The baseline values shall be stored in nonvolatile memory so that they may be restored following a system power cycle.

The external 24LC32 EEPROM may be used for storage of:

* SGP30 TVOC baseline  
* SGP30 eCO2 baseline  
* Baseline timestamp or age  
* Baseline-valid flag

If no valid stored baseline exists, the firmware shall allow the SGP30 to establish a new baseline before storing it.

# **23\. Real-Time Clock**

## **23.1 DS3231 RTC**

The selected RTC is the **Adafruit DS3231 Precision RTC**, Adafruit Product 5188\.

Reference:

[https\://www\.adafruit.com/product/5188](https://www.adafruit.com/product/5188)

The DS3231 is a temperature-compensated real-time clock with an integrated oscillator.

The device communicates with the ESP32-S3 over I²C.

The RTC shall provide local time immediately following system startup without requiring network connectivity.

The RTC shall maintain:

* Seconds  
* Minutes  
* Hours  
* Day  
* Date  
* Month  
* Year

The RTC shall be battery-backed to maintain time while primary USB power is removed.

## 

## **23.2 RTC Synchronization**

The system time hierarchy shall be:

**Network NTP → ESP32 System Time → DS3231 RTC**

During startup:

1. The ESP32-S3 shall read the DS3231.  
2. The RTC value shall initialize the local system clock.  
3. The device shall establish Wi-Fi connectivity.  
4. The system shall attempt NTP synchronization.  
5. Following successful synchronization, the DS3231 shall be corrected if necessary.

This arrangement allows accurate operation when Internet connectivity is unavailable while periodically correcting RTC drift whenever network time is available.

# **24\. Configuration EEPROM**

## **24.1 24LC32**

The selected external configuration memory is the **Adafruit 24LC32 I2C EEPROM Breakout**, Adafruit Product 5146\.

Reference:

[https\://www\.adafruit.com/product/5146](https://www.adafruit.com/product/5146)

The 24LC32 provides:

* 32 Kbit storage capacity  
* 4 KB usable byte storage  
* I²C interface  
* Default I²C address of 0x50  
* Byte-write capability  
* 32-byte page-write buffer  
* Approximately 1,000,000 program/erase cycles  
* Nonvolatile data retention

The EEPROM shall be used for persistent device-specific configuration information.

## 

## **24.2 EEPROM Data**

Candidate EEPROM contents include:

* Configuration data version  
* Device configuration validity marker  
* Temperature-unit preference  
* Geographic location  
* Time-zone configuration  
* Display brightness  
* Display preferences  
* Weather-service configuration  
* Last successful weather-location information  
* SGP30 baseline values  
* SGP30 baseline timestamp

Wi-Fi credentials may also be stored in the EEPROM; however, storing them using the ESP32-S3's encrypted or protected nonvolatile storage facilities should be considered during software design.

The EEPROM shall use a defined data structure including a version number and integrity check such as a CRC.

This will permit firmware updates to detect incompatible or corrupted configuration data.

# 

# **25\. I²C Architecture**

A shared I²C bus shall be used to connect low-speed peripheral devices.

The expected I²C devices include:

| Device | Function | Address |
| ----- | ----- | ----- |
| AHT20 | Temperature / Humidity | 0x38 |
| Rotary Encoder / seesaw | User Input | 0x36 nominal |
| 24LC32 | Configuration EEPROM | 0x50 nominal |
| DS3231 | RTC | Device fixed address |
| SGP30 | Air Quality | Device fixed address |
| Qualia PCA9554 | Board I/O Expansion | Board-defined address |

All selected peripherals shall be checked for address conflicts prior to final integration.

The I²C bus should operate at **400 kHz Fast Mode** if all installed devices and wiring permit.

STEMMA QT interconnections may be used during prototyping.

The production or integrated version should minimize total I²C cable length and bus capacitance.

# 

# **26\. Power Architecture**

Primary system power shall enter through the **USB Type-C connector located on the Qualia ESP32-S3 board**.

The Qualia board shall provide the primary regulated power required by the processor and display subsystem.

Peripheral sensors shall obtain power from the appropriate regulated supply provided by the system.

The power budget shall include:

* ESP32-S3 processor  
* Wi-Fi radio  
* LCD interface  
* LCD backlight  
* AHT20  
* SGP30  
* DS3231  
* 24LC32  
* Rotary encoder / seesaw controller  
* I/O expander

The design shall provide sufficient margin for transient current associated with Wi-Fi operation and display-backlight load.

# **27\. Hardware Selection Assessment**

The selected hardware is considered suitable for the Desk Widget prototype.

## **27.1 Qualia ESP32-S3 and Display**

The Qualia ESP32-S3 is particularly well suited to the selected display because it was designed specifically to interface with RGB TTL panels. The board provides the PSRAM and dedicated ESP32-S3 RGB peripheral required to continuously refresh a display of this resolution.

The selected 3.4-inch display is therefore preferable to attempting to operate an equivalent display through a conventional SPI graphics interface.

## **27.2 Shared I²C Architecture**

The use of I²C peripherals significantly reduces GPIO requirements.

This is especially important because the RGB display consumes most of the ESP32-S3's directly available GPIO resources.

The STEMMA QT architecture also simplifies prototype wiring and component replacement.

## **27.3 External EEPROM**

The ESP32-S3 contains onboard flash that could technically store the configuration data without an external EEPROM.

The dedicated 24LC32 is nevertheless retained in the design because it provides:

* Independent configuration storage  
* High write endurance  
* Straightforward byte-level access  
* Separation between firmware storage and configuration storage  
* Convenient storage for SGP30 calibration baseline information

The external EEPROM therefore serves as a dedicated **system configuration and calibration store**, rather than being strictly required by the processor.

## 

## **27.4 Environmental Sensors**

The AHT20 and SGP30 form a complementary sensor set.

The AHT20 provides the temperature and humidity measurements required for the user interface while also supplying environmental compensation information to the SGP30.

The SGP30 is suitable for general indoor air-quality trend monitoring but shall not be considered a true CO2 sensor.

## **27.5 RTC**

The DS3231 provides a robust local time source independent of network availability.

Network synchronization shall be used to periodically correct the RTC rather than serving as the only available clock source.

# 

# **28\. Hardware Design Risks and Considerations**

The following items shall be addressed during prototype integration.

### **Thermal Measurement Error**

Internal heating from the LCD and processor may cause the AHT20 to report temperatures higher than actual room temperature.

Sensor positioning and enclosure ventilation shall therefore be evaluated during prototype testing.

### **SGP30 Warm-Up and Baseline**

The SGP30 requires conditioning and baseline management. Immediately after initialization, reported values should not automatically be treated as fully stabilized air-quality measurements.

### **I²C Bus Loading**

Multiple STEMMA QT devices will share the I²C bus. The number of pull-up resistors and total bus capacitance shall be evaluated if communication reliability problems occur.

### **Configuration Write Endurance**

Frequently changing runtime values shall not be continuously written to the configuration EEPROM.

Writes shall occur only when necessary, particularly for configuration changes and scheduled sensor-baseline persistence.

### **Display Memory**

Graphics resources shall be managed carefully despite the available 8 MB PSRAM. Large uncompressed bitmaps and multiple full-screen framebuffers can consume memory rapidly.

### **Environmental Sensor Ventilation**

The AHT20 and SGP30 shall be exposed to representative room air and should not be mounted in an unventilated region behind the LCD.

[image1]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAnAAAAGUCAYAAAC1Lh+qAAA9QUlEQVR4Xu2dCdgkVX2vO7nRCxONMYuyyGg0SlwSEjaXoCOjV3EDREWROGhUCIQRxJErQbAFEweIgDIsAioBRJbIMjFEUZlhRoLKiCCyCsFEGLhq3Ab1emOeut+v4N+e79/V/XX11+frOnXe93l+T3efqjpVXd+pOe+cWrqz//77F4SknA4AAEBuqAP86U9/SkiSQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCxB4EjKQeAAACBLEDiSchA4AADIEgSOpBwEDgAAsgSBIykHgQMAgCypK3Af+tCHit12263YYYcdire+9a3FtddeWzz44IN98w3Ld7/73XLZF77whbPKr7rqqrLe//iP/+hbZhKxdd5www1902655Zbiec97Xrn+XXbZpbjzzjv75lG63W6x6667lvO84x3vKJfz89SN9sew7Vq+fHlvu4466qi+eRTbLs036nZpfbvvvvvI+/vHP/5x+Tc66KCDyvXstNNOxTHHHFPcfvvtffNquo/27xve8Ibi4x//eN/84waBy5MDT1v/2OUnrn33ilXrHvzIp2958LIv3VvceM/Pitvv/6/izgd+SUiSUfu95ps/LNSe1a5XrFr/b2rnau/+GACoJXB333132RHvvPPOxctf/vJeJ37CCSf0zTssxx13XLnsQgqcJNPW6UXpyiuvLOVCgvSqV72qfNV8X/jCF2bNJ1ExEdE8Vd+hbrRd2h/Dtkvrse3S+2Hb9cpXvrK3XX4+n7oC9+pXv7onY9oeawNqDxL7cF6bb1DWr1/fV/84QeDy45CT1x5/8MlrfqkOzneAhLQpa77xn6XIqb0f1F3zKH8sQOaMKnAafTn88MPLDvOBBx4oy9asWVO8+MUvLsXBzz8sJiVefmIKnGTI1ulFSdIjEf3BD35QftarPr/+9a8v7r333rLse9/7XnHggQcWRxxxRPle85xyyinlfN/+9rf71jdqTNKGbZfWo89a5/HHHz90u1Rm2/Xa1762b31h6gictlN1HnbYYcVdd93VKz/zzDPL7de0cCRW+9rXre1TPRJBtZsNGzb0raduELi8WP6hq5eddvmtP9tw94N9nR0hbY3a+6EfWfdjtX9/TEDGjCpwGjHZa6+9ih/+8Iezys8+++yys/7Rj37Ut4yPSeBll11WvP3tbx9J4DSP5r3vvvt6o0s6pTjKKcJwnZIMW6cXJdV55JFHziqTqKj8vPPOKz9/8IMfLD/7+iUk2h7tF50K1TwSKpMpZc899ywFR/sqXNa2S/tj1O3S9xl1u1Tu/15hRhW4VatWlXXdf//9fdMUfe+3ve1txdVXX90rqxK4MM9//vOL5zznOcW6dev6ptUJApcHK1at/xojboT8stCxwClVKBlV4KpiclQlEFW54IILSpGR3NQR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>