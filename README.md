# APADevices - DS2482 Temperature Sensor Library

A modern, lightweight Arduino library for the DS2482-800 I²C to 1-Wire bridge, optimized for use with DS18B20 temperature sensors. This library provides a simple yet powerful interface for temperature monitoring applications.

## Key Features

- **Simplified Single-Sensor Operation**: Designed for one DS18B20 per channel, eliminating the complexity of sensor addressing
- **Minimal Dependencies**: Relies only on the standard Wire library
- **Non-Blocking Operation**: Most functions operate without blocking, allowing your code to handle other tasks
- **Selective Diagnostics**: Comprehensive diagnostic system that can be enabled/disabled at compile time
- **Lightweight Implementation**: Modern, memory-efficient design optimized for Arduino

## Hardware Setup

- Connect DS2482-800 to Arduino via I2C:
  - SDA → Arduino SDA
  - SCL → Arduino SCL
  - VCC → 3.3V or 5V (check your module's requirements)
  - GND → GND
- Connect DS18B20 sensors:
  - One sensor per channel
  - !!Require passive 4.7kΩ pullup resistor for each sensor!!
  - Up to 8 sensors (one per channel)

### Important Note on Pullup Configuration
This library is designed to work with external passive pullup resistors (4.7kΩ) and intentionally does not use the DS2482's active or strong pullup features. This design choice:
- Provides more reliable communication
- Ensures consistent timing across all channels
- Reduces potential issues with power-sensitive configurations
- Simplifies the library implementation and reduces code complexity

The use of passive pullup resistors is mandatory for proper operation. Do not rely on internal pullups or the DS2482's active/strong pullup features.

## Installation

1. Download the library (ZIP file)
2. In Arduino IDE: Sketch → Include Library → Add .ZIP Library
3. Select the downloaded file
4. Restart Arduino IDE

## Quick Start

```cpp
#include <Wire.h>
#include "DS2482.h"

DS2482 ds2482;  // Create instance with default address (0x18)

void setup() {
    Serial.begin(9600);
    if (ds2482.begin()) {
        Serial.println("DS2482 initialized!");
    }
}

void loop() {
    float temperature;
    if (ds2482.startTemperatureConversion(0)) {  // Channel 0
        delay(750);  // Wait for conversion
        if (ds2482.readTemperature(0, &temperature)) {
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println(" °C");
        }
    }
    delay(1000);
}
```

## Advanced Features

### Non-Blocking Operation
```cpp
if (ds2482.startTemperatureConversion(channel)) {
    // Do other tasks while conversion is in progress
    if (ds2482.checkConversionStatus()) {
        // Read temperature when ready
    }
}
```

### Diagnostic Output
Enable detailed diagnostics by defining before including the library:
```cpp
#define DS2482_DIAGNOSTICS 1
#include "DS2482.h"
```

### Error Handling
```cpp
float temperature;
if (!ds2482.startTemperatureConversion(channel)) {
    // Handle conversion start failure
    return;
}
if (!ds2482.readTemperature(channel, &temperature)) {
    // Handle reading failure
    return;
}
```

## Benefits Over Other Libraries

1. **Simplified Operation**
   - No ROM code handling required
   - Direct channel-based addressing
   - Intuitive API design

2. **Modern Design**
   - Clean, well-documented code
   - Efficient memory usage
   - No external dependencies
   - Reliable communication with passive pullups

3. **Developer Friendly**
   - Comprehensive error reporting
   - Optional diagnostic system
   - Non-blocking operations
   - Clear hardware requirements

4. **Resource Efficient**
   - Minimal RAM usage
   - No dynamic memory allocation
   - Optimized for Arduino platforms
   - Simplified pullup configuration

## Technical Considerations

1. **Pullup Configuration**
   - External 4.7kΩ pullup resistors required for each channel
   - DS2482 active/strong pullups intentionally disabled
   - More consistent and reliable operation across different environments
   - Better power efficiency in most configurations

2. **Channel Operation**
   - One sensor per channel for simplified operation
   - No sensor addressing required
   - Reliable channel switching with proper timing
   - Built-in error checking and recovery

## License

This library is released under the MIT License.

## Author

APADevices [@kecup]

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Support

No support included! Use as is of leave it. Sorry about that.
