/**
 * APADevices - DS2482.h - APADevices library for DS2482-800 I2C to 1-Wire bridge with DS18B20 support
 * 
 * This library provides an interface for the DS2482-800 1-Wire master device,
 * specifically designed for use with DS18B20 temperature sensors. It features:
 * - Non-blocking temperature conversion
 * - Multi-channel support (8 channels)
 * - Comprehensive error checking
 * - Optional diagnostic output
 * 
 * To enable diagnostic output, define DS2482_DIAGNOSTICS before including this header:
 * #define DS2482_DIAGNOSTICS 1
 */

#ifndef DS2482_H
#define DS2482_H

#include <Arduino.h>
#include <Wire.h>

// Debug configuration - user can define this before including the library
#ifndef DS2482_DIAGNOSTICS
#define DS2482_DIAGNOSTICS 0 // Default to disabled
#endif

// Debug macros for diagnostic output
#if DS2482_DIAGNOSTICS
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINT_HEX(x) Serial.print(x, HEX)
    #define DEBUG_PRINTLN_HEX(x) Serial.println(x, HEX)
    #define DEBUG_PRINT_STATUS(msg, status) do { Serial.print(msg); Serial.println(status, HEX); } while(0)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINT_HEX(x)
    #define DEBUG_PRINTLN_HEX(x)
    #define DEBUG_PRINT_STATUS(msg, status)
#endif

// Device commands from DS2482 datasheet
#define DS2482_CHANNEL_READBACK    0xD2    // Channel selection readback register
#define DS2482_CMD_CHANNEL_SELECT  0xC3    // Channel select command
#define DS2482_CMD_RESET          0xF0    // Device reset
#define DS2482_CMD_SET_READ       0xE1    // Set read pointer
#define DS2482_CMD_WRITE_CONFIG   0xD2    // Write configuration
#define DS2482_CMD_WIRE_RESET     0xB4    // 1-Wire reset
#define DS2482_CMD_WRITE_BYTE     0xA5    // Write byte
#define DS2482_CMD_READ_BYTE      0x96    // Read byte
#define DS2482_CMD_SINGLE_BIT     0x87    // Single bit operation

// Status register bit masks
#define DS2482_STATUS_1WB     0x01    // 1-Wire Busy
#define DS2482_STATUS_PPD     0x02    // Presence Pulse Detect
#define DS2482_STATUS_SD      0x04    // Short Detected
#define DS2482_STATUS_LL      0x08    // Logic Level
#define DS2482_STATUS_RST     0x10    // Device Reset
#define DS2482_STATUS_SBR     0x20    // Single Bit Result
#define DS2482_STATUS_TSB     0x40    // Triple Search Bit
#define DS2482_STATUS_DIR     0x80    // Branch Direction Taken

// Operation states for state machine
enum class DS2482State {
    IDLE,                   // No operation in progress
    CONVERTING_TEMPERATURE, // Temperature conversion in progress
    ERROR                   // Error state requiring reset
};

class DS2482 {
public:
    // Constructor and initialization
    DS2482(uint8_t address = 0x18);
    bool begin();          // Initialize device
    bool reset();          // Reset device
    bool wakeUp();         // Wake up device
    
    // Basic device operations
    uint8_t readStatus();  // Read status register
    void printStatus();    // Print status register (if diagnostics enabled)
    
    // Channel operations
    bool selectChannel(uint8_t channel);  // Select 1-Wire channel (0-7)
    uint8_t getCurrentChannel() { return currentChannel; }
    
    // 1-Wire operations
    bool wireReset();                     // Reset 1-Wire bus
    void wireWriteBit(uint8_t bit);      // Write single bit
    uint8_t wireReadBit();               // Read single bit
    void wireWriteByte(uint8_t byte);    // Write byte
    uint8_t wireReadByte();              // Read byte

    // Temperature sensor operations
    bool startTemperatureConversion(uint8_t channel);  // Start conversion
    bool checkConversionStatus();                      // Check if conversion complete
    bool readTemperature(uint8_t channel, float* temperature);  // Read temperature
    bool readScratchpad(uint8_t* scratchpad);         // Read sensor scratchpad
    void printScratchpad(uint8_t* scratchpad);        // Print scratchpad data

    // State management
    DS2482State getState() { return currentState; }
    bool isBusy() { return currentState == DS2482State::CONVERTING_TEMPERATURE; }
    void clearState() { currentState = DS2482State::IDLE; }

private:
    uint8_t address;            // I2C address of DS2482
    DS2482State currentState;   // Current operation state
    unsigned long conversionStartTime;  // Timestamp for conversion timing
    uint8_t currentChannel;     // Currently selected channel
    
    // Private helper functions
    void writeCommand(uint8_t command);           // Write command to device
    void setReadPointer(uint8_t readPointer);     // Set read pointer
    bool waitFor1Wire();                          // Wait for 1-Wire bus ready
    bool beginTemperatureOperation();             // Initialize temperature operation
};

#endif