/*
 * APADevices - DS2482 Multi-channel Sensor Temperature Reading Example
 * 
 * This example demonstrates the basic usage of the DS2482 library 
 * to read temperatures from DS18B20 sensors. It's designed to be
 * as simple as possible for first-time users.
 * 
 * Hardware Setup:
 * - Connect DS2482-800 to Arduino via I2C:
 *   * SDA to Arduino SDA
 *   * SCL to Arduino SCL
 *   * VCC to 3.3V or 5V (check your module's requirements)
 *   * GND to GND
 * - Connect DS18B20 sensors to DS2482 channels:
 *   * Each sensor needs a 4.7kΩ pullup resistor between data and VCC
 *   * Multiple sensors can be connected to different channels
 * 
 * This example:
 * - Reads temperature from a single sensor on channel 0
 * - Prints temperature every 2 seconds
 * - Shows basic error handling
 * Enable diagnostics by uncommenting the following line:
 */
// To enable diagnostic output you have to modify (and uncomment) following line in header file (.h):
//#define DS2482_DIAGNOSTICS 1

#include <Wire.h>
#include "DS2482.h"

#define DS2482_ADDRESS 0x18
DS2482 ds2482(DS2482_ADDRESS);

unsigned long lastCheck = 0;
const unsigned long CHECK_INTERVAL = 1000;  // Check every second
uint8_t currentChannel = 0;
bool conversionStarted = false;

void setup() {
    Serial.begin(9600);
    while (!Serial) delay(10);
    
    Serial.println("\nDS2482 Temperature Sensor Example");
    
    if (ds2482.begin()) {
        Serial.println("DS2482-800 initialized successfully");
        // Start first conversion
        ds2482.startTemperatureConversion(currentChannel);
    } else {
        Serial.println("Failed to initialize DS2482-800");
        while (1);
    }
}

void loop() {
    if (millis() - lastCheck >= CHECK_INTERVAL) {
        lastCheck = millis();
        
        if (!conversionStarted) {
            if (ds2482.startTemperatureConversion(currentChannel)) {
                conversionStarted = true;
            } else {
                // Move to next channel if current fails
                currentChannel = (currentChannel + 1) % 8;
            }
        }
        else if (ds2482.checkConversionStatus()) {
            float temperature;
            if (ds2482.readTemperature(currentChannel, &temperature)) {
                Serial.print("Channel ");
                Serial.print(currentChannel);
                Serial.print(": ");
                Serial.print(temperature);
                Serial.println(" °C");
            }
            
            // Move to next channel
            currentChannel = (currentChannel + 1) % 8;
            conversionStarted = false;
        }
    }
    
    // Other tasks can be done here
}
