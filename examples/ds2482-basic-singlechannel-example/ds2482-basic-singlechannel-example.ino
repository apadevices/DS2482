/*
 * APADevices - DS2482 Basic Temperature Singlechannel Reading Example
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
 */

// To enable diagnostic output you have to modify (and uncomment) following line in header file (.h):
//#define DS2482_DIAGNOSTICS 1

#include <Wire.h>
#include "DS2482.h"

// Create DS2482 object with default address (0x18)
DS2482 ds2482;

// Variables for timing
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 2000;  // Read every 2 seconds

void setup() {
    // Start serial communication
    Serial.begin(9600);
    while (!Serial) delay(10);  // Wait for serial port (needed for USB)
    
    Serial.println("DS2482 Basic Temperature Example");
    Serial.println("-------------------------------");
    
    // Initialize DS2482
    if (ds2482.begin()) {
        Serial.println("DS2482 initialized successfully!");
    } else {
        Serial.println("Failed to initialize DS2482!");
        Serial.println("Please check your connections.");
        while (1);  // Stop if initialization failed
    }
}

void loop() {
    // Check if it's time to read temperature
    if (millis() - lastReadTime >= READ_INTERVAL) {
        lastReadTime = millis();
        
        float temperature;
        // Try to read temperature from channel 0
        if (ds2482.startTemperatureConversion(0)) {
            // Wait for conversion to complete (750ms)
            delay(750);
            
            // Read the temperature
            if (ds2482.readTemperature(0, &temperature)) {
                Serial.print("Temperature: ");
                Serial.print(temperature);
                Serial.println(" °C");
            } else {
                Serial.println("Failed to read temperature!");
            }
        } else {
            Serial.println("No sensor found on channel 0");
        }
    }
}
