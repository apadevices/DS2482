/*
 * APADevices - DS2482 Advanced Multichannel Temperature Reading Example
 * 
 * This example demonstrates advanced usage of the DS2482 library including:
 * - Non-blocking temperature reading
 * - Multi-channel support with retry mechanism
 * - Detailed error handling and recovery
 * - Diagnostic output
 * - Robust state machine operation
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
 * Features:
 * - Full diagnostic output
 * - Channel scanning with retry mechanism
 * - Non-blocking operation with proper timing
 * - Robust error recovery
 * - Detailed status reporting
 */

// To enable diagnostic output you have to modify following line in header file (.h):
//#define DS2482_DIAGNOSTICS 1

#include <Wire.h>
#include "DS2482.h"

// Create DS2482 object with default address (0x18)
DS2482 ds2482;

// Timing constants
const unsigned long SCAN_INTERVAL = 5000;    // Scan channels every 5 seconds
const unsigned long STATUS_INTERVAL = 1000;  // Print status every second
const uint8_t MAX_RETRIES = 3;              // Maximum retry attempts per channel

// State variables
unsigned long lastScanTime = 0;
unsigned long lastStatusTime = 0;
uint8_t currentChannel = 0;
uint8_t retryCount = 0;
bool conversionStarted = false;

// Temperature storage
float channelTemperatures[8];
bool channelActive[8] = {false};
uint8_t channelErrors[8] = {0};

void setup() {
    // Start serial communication
    Serial.begin(9600);
    while (!Serial) delay(10);
    
    Serial.println("\nDS2482 Advanced Temperature Example");
    Serial.println("----------------------------------");
    
    // Initialize DS2482
    if (ds2482.begin()) {
        Serial.println("DS2482 initialized successfully!");
        printDeviceStatus();
    } else {
        Serial.println("Failed to initialize DS2482!");
        Serial.println("Please check your connections.");
        while (1);
    }

    // Initialize arrays
    for (int i = 0; i < 8; i++) {
        channelTemperatures[i] = 0.0;
        channelActive[i] = false;
        channelErrors[i] = 0;
    }
}

void loop() {
    unsigned long currentTime = millis();
    
    // Regular status updates
    if (currentTime - lastStatusTime >= STATUS_INTERVAL) {
        lastStatusTime = currentTime;
        printDeviceStatus();
        printTemperatures();
    }
    
    // Channel scanning state machine
    if (!conversionStarted) {
        // Start new conversion if it's time
        if (currentTime - lastScanTime >= SCAN_INTERVAL) {
            startNextChannel();
        }
    } else {
        // Check if conversion is complete
        if (ds2482.checkConversionStatus()) {
            readCurrentChannel();
        }
    }
    
    // Check for device errors and reset if necessary
    if (ds2482.getState() == DS2482State::ERROR) {
        recoverFromError();
    }
}

/**
 * Start temperature conversion on next active channel
 */
void startNextChannel() {
    Serial.print("\nStarting conversion on channel ");
    Serial.println(currentChannel);
    
    // Check device status and reset if needed
    if (ds2482.getState() == DS2482State::ERROR) {
        recoverFromError();
        return;
    }
    
    if (ds2482.startTemperatureConversion(currentChannel)) {
        conversionStarted = true;
        Serial.println("Conversion started successfully");
    } else {
        handleChannelError();
    }
}

/**
 * Read temperature from current channel
 */
void readCurrentChannel() {
    float temperature;
    if (ds2482.readTemperature(currentChannel, &temperature)) {
        // Successful read
        channelTemperatures[currentChannel] = temperature;
        channelActive[currentChannel] = true;
        channelErrors[currentChannel] = 0;
        retryCount = 0;
        
        Serial.print("Channel ");
        Serial.print(currentChannel);
        Serial.print(" temperature: ");
        Serial.print(temperature);
        Serial.println(" °C");
        
        moveToNextChannel();
    } else {
        handleChannelError();
    }
}

/**
 * Handle channel errors with retry mechanism
 */
void handleChannelError() {
    channelErrors[currentChannel]++;
    
    if (retryCount < MAX_RETRIES) {
        retryCount++;
        Serial.print("Retry attempt ");
        Serial.print(retryCount);
        Serial.print(" for channel ");
        Serial.println(currentChannel);
        
        conversionStarted = false;
        delay(100);  // Short delay before retry
    } else {
        // Max retries reached
        channelActive[currentChannel] = false;
        Serial.print("Channel ");
        Serial.print(currentChannel);
        Serial.println(" failed after maximum retries");
        
        retryCount = 0;
        moveToNextChannel();
    }
}

/**
 * Move to next channel with proper timing
 */
void moveToNextChannel() {
    delay(100);  // Ensure proper timing between channels
    currentChannel = (currentChannel + 1) % 8;
    conversionStarted = false;
    retryCount = 0;
    lastScanTime = millis();
}

/**
 * Recover from device error state
 */
void recoverFromError() {
    Serial.println("Recovering from error state...");
    ds2482.reset();
    delay(100);
    
    if (ds2482.begin()) {
        Serial.println("Device recovered successfully");
    } else {
        Serial.println("Device recovery failed");
    }
}

/**
 * Print current device status
 */
void printDeviceStatus() {
    Serial.println("\n--- Device Status ---");
    Serial.print("Current channel: ");
    Serial.println(currentChannel);
    Serial.print("State: ");
    switch (ds2482.getState()) {
        case DS2482State::IDLE:
            Serial.println("IDLE");
            break;
        case DS2482State::CONVERTING_TEMPERATURE:
            Serial.println("CONVERTING");
            break;
        case DS2482State::ERROR:
            Serial.println("ERROR");
            break;
    }
    ds2482.printStatus();
    Serial.println("-------------------");
}

/**
 * Print temperatures and channel status
 */
void printTemperatures() {
    Serial.println("\nChannel Status and Temperatures:");
    for (int i = 0; i < 8; i++) {
        Serial.print("Ch ");
        Serial.print(i);
        Serial.print(": ");
        
        if (channelActive[i]) {
            Serial.print(channelTemperatures[i]);
            Serial.print(" °C");
            if (channelErrors[i] > 0) {
                Serial.print(" (Errors: ");
                Serial.print(channelErrors[i]);
                Serial.print(")");
            }
        } else {
            Serial.print("Inactive");
        }
        Serial.println();
    }
}
