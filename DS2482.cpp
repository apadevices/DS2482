/**
 * APADevices - DS2482.cpp - Implementation for DS2482-800 I2C to 1-Wire bridge
 * 
 * This implementation includes non-blocking operations where possible and
 * provides comprehensive error checking and status reporting. Critical timing
 * requirements from the DS2482 datasheet are maintained while longer delays
 * are replaced with polling mechanisms.
 */

#include "DS2482.h"

/**
 * Constructor - Initialize member variables
 * @param address I2C address of DS2482 (default 0x18)
 */
DS2482::DS2482(uint8_t address) : 
    address(address),
    currentState(DS2482State::IDLE),
    conversionStartTime(0),
    currentChannel(0) {}

/**
 * Initialize the DS2482 device
 * Performs device reset, verifies communication, and prepares for operation
 * @return true if initialization successful, false on any error
 */
bool DS2482::begin() {
    Wire.begin();
    DEBUG_PRINTLN("Initializing DS2482...");
    
    if (!reset()) {
        DEBUG_PRINTLN("Reset failed");
        return false;
    }
    
    if (!wakeUp()) {
        DEBUG_PRINTLN("Wake up failed");
        return false;
    }

    uint8_t status = readStatus();
    if (status == 0x18) {
        DEBUG_PRINTLN("DS2482-800 Initialized Successfully");
        currentState = DS2482State::IDLE;
        return true;
    } else {
        DEBUG_PRINT("Initialization Failed, Status: 0x");
        DEBUG_PRINTLN_HEX(status);
        currentState = DS2482State::ERROR;
        return false;
    }
}

/**
 * Reset the DS2482 with timeout checking
 * Non-blocking implementation that polls for completion
 * @return true if reset successful, false on timeout or error
 */
bool DS2482::reset() {
    DEBUG_PRINTLN("Resetting DS2482");
    writeCommand(DS2482_CMD_RESET);
    
    unsigned long startTime = millis();
    while (millis() - startTime < 100) {  // 100ms timeout
        uint8_t status = readStatus();
        if (status & DS2482_STATUS_RST) {
            currentState = DS2482State::IDLE;
            return true;
        }
        delayMicroseconds(100);  // Short delay between checks
    }
    currentState = DS2482State::ERROR;
    return false;
}

/**
 * Wake up the DS2482 and verify it's ready
 * @return true if device responds and becomes ready
 */
bool DS2482::wakeUp() {
    DEBUG_PRINTLN("Waking up DS2482");
    writeCommand(DS2482_CMD_READ_BYTE);
    
    unsigned long startTime = millis();
    while (millis() - startTime < 100) {  // 100ms timeout
        uint8_t status = readStatus();
        if (!(status & DS2482_STATUS_1WB)) {  // Check if 1-Wire Busy bit is clear
            return true;
        }
        delayMicroseconds(100);
    }
    return false;
}

/**
 * Read the status register
 * @return Status register value or 0xFF on error
 */
uint8_t DS2482::readStatus() {
    setReadPointer(0xF0);
    Wire.requestFrom(address, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0xFF;
}

/**
 * Select a specific 1-Wire channel
 * Includes verification of channel selection success
 * @param channel Channel number (0-7)
 * @return true if channel selected and verified
 */
bool DS2482::selectChannel(uint8_t channel) {
    if (channel > 7) {
        DEBUG_PRINTLN("Invalid channel number");
        return false;
    }

    // Channel selection codes from DS2482 datasheet
    static const uint8_t channelCodes[8] = {0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0x96, 0x87};
    static const uint8_t readBackValues[8] = {0xB8, 0xB1, 0xAA, 0xA3, 0x9C, 0x95, 0x8E, 0x87};

    DEBUG_PRINT("Selecting channel ");
    DEBUG_PRINTLN(channel);
    
    Wire.beginTransmission(address);
    Wire.write(DS2482_CMD_CHANNEL_SELECT);
    Wire.write(channelCodes[channel]);
    
    if (Wire.endTransmission() != 0) {
        DEBUG_PRINTLN("Channel selection command failed");
        currentState = DS2482State::ERROR;
        return false;
    }

    delayMicroseconds(100);  // Required by DS2482 specification

    setReadPointer(DS2482_CHANNEL_READBACK);
    Wire.requestFrom(address, (uint8_t)1);
    
    if (!Wire.available()) {
        DEBUG_PRINTLN("No response during channel verification");
        currentState = DS2482State::ERROR;
        return false;
    }

    uint8_t readBack = Wire.read();
    currentChannel = channel;

    DEBUG_PRINT("Expected readback: 0x");
    DEBUG_PRINT_HEX(readBackValues[channel]);
    DEBUG_PRINT(" Got: 0x");
    DEBUG_PRINTLN_HEX(readBack);

    bool success = (readBack == readBackValues[channel]);
    if (!success) {
        currentState = DS2482State::ERROR;
    }
    return success;
}

/**
 * Print current status register (debug)
 */
void DS2482::printStatus() {
    uint8_t status = readStatus();
    DEBUG_PRINT("Status: 0x");
    DEBUG_PRINTLN_HEX(status);
}

/**
 * Reset the 1-Wire bus and check for device presence
 * @return true if device presence detected
 */
bool DS2482::wireReset() {
    DEBUG_PRINTLN("Performing 1-Wire reset");
    writeCommand(DS2482_CMD_WIRE_RESET);
    
    unsigned long startTime = millis();
    while (millis() - startTime < 100) {  // 100ms timeout
        uint8_t status = readStatus();
        if (!(status & DS2482_STATUS_1WB)) {
            bool presenceDetected = (status & DS2482_STATUS_PPD) != 0;
            DEBUG_PRINT("Wire reset result: ");
            DEBUG_PRINTLN(presenceDetected ? "Device detected" : "No device");
            
            if (!presenceDetected) {
                currentState = DS2482State::ERROR;
            }
            return presenceDetected;
        }
        delayMicroseconds(100);
    }
    
    currentState = DS2482State::ERROR;
    return false;
}

/**
 * Write a single bit to the 1-Wire bus
 * @param bit Bit value to write (0 or 1)
 */
void DS2482::wireWriteBit(uint8_t bit) {
    if (!waitFor1Wire()) {
        DEBUG_PRINTLN("1-Wire bus busy during bit write");
        currentState = DS2482State::ERROR;
        return;
    }
    
    Wire.beginTransmission(address);
    Wire.write(DS2482_CMD_SINGLE_BIT);
    Wire.write(bit ? 0x80 : 0x00);
    Wire.endTransmission();
}

/**
 * Read a single bit from the 1-Wire bus
 * @return Bit value read (0 or 1), or 0 on error
 */
uint8_t DS2482::wireReadBit() {
    if (!waitFor1Wire()) {
        DEBUG_PRINTLN("1-Wire bus busy during bit read");
        currentState = DS2482State::ERROR;
        return 0;
    }
    
    Wire.beginTransmission(address);
    Wire.write(DS2482_CMD_SINGLE_BIT);
    Wire.write(0x80);
    Wire.endTransmission();

    if (!waitFor1Wire()) {
        DEBUG_PRINTLN("Bit read timeout");
        currentState = DS2482State::ERROR;
        return 0;
    }

    return (readStatus() & DS2482_STATUS_SBR) ? 1 : 0;
}

/**
 * Write a byte to the 1-Wire bus
 * @param byte Byte value to write
 */
void DS2482::wireWriteByte(uint8_t byte) {
    if (!waitFor1Wire()) {
        DEBUG_PRINTLN("1-Wire bus busy during byte write");
        currentState = DS2482State::ERROR;
        return;
    }
    
    Wire.beginTransmission(address);
    Wire.write(DS2482_CMD_WRITE_BYTE);
    Wire.write(byte);
    Wire.endTransmission();
}

/**
 * Read a byte from the 1-Wire bus
 * @return Byte value read, or 0xFF on error
 */
uint8_t DS2482::wireReadByte() {
    if (!waitFor1Wire()) {
        DEBUG_PRINTLN("1-Wire bus busy during byte read");
        currentState = DS2482State::ERROR;
        return 0xFF;
    }
    
    writeCommand(DS2482_CMD_READ_BYTE);
    
    if (!waitFor1Wire()) {
        DEBUG_PRINTLN("Read operation timeout");
        currentState = DS2482State::ERROR;
        return 0xFF;
    }

    setReadPointer(0xE1);
    Wire.requestFrom(address, (uint8_t)1);
    uint8_t value = Wire.available() ? Wire.read() : 0xFF;
    
    DEBUG_PRINT("Read byte: 0x");
    DEBUG_PRINTLN_HEX(value);
    
    return value;
}

/**
 * Start temperature conversion on specified channel
 * @param channel Channel number (0-7)
 * @return true if conversion started successfully
 */
bool DS2482::startTemperatureConversion(uint8_t channel) {
    DEBUG_PRINT("Starting temperature conversion on channel ");
    DEBUG_PRINTLN(channel);
    
    currentState = DS2482State::IDLE;
    
    if (!selectChannel(channel)) {
        DEBUG_PRINTLN("Failed to select channel for conversion");
        return false;
    }

    if (!beginTemperatureOperation()) {
        DEBUG_PRINTLN("Failed to begin temperature operation");
        return false;
    }

    wireWriteByte(0xCC); // Skip ROM
    wireWriteByte(0x44); // Convert T
    
    conversionStartTime = millis();
    currentState = DS2482State::CONVERTING_TEMPERATURE;
    DEBUG_PRINTLN("Conversion started successfully");
    return true;
}

/**
 * Check if temperature conversion is complete
 * @return true if conversion complete
 */
bool DS2482::checkConversionStatus() {
    if (currentState != DS2482State::CONVERTING_TEMPERATURE) {
        return false;
    }

    if (millis() - conversionStartTime >= 750) {  // DS18B20 conversion time
        DEBUG_PRINTLN("Temperature conversion complete");
        currentState = DS2482State::IDLE;
        return true;
    }
    
    return false;
}

/**
 * Read temperature from specified channel
 * @param channel Channel number (0-7)
 * @param temperature Pointer to store temperature value
 * @return true if temperature read successfully
 */
bool DS2482::readTemperature(uint8_t channel, float* temperature) {
    DEBUG_PRINT("Reading temperature from channel ");
    DEBUG_PRINTLN(channel);
    
    currentState = DS2482State::IDLE;
    
    if (!selectChannel(channel)) {
        DEBUG_PRINTLN("Failed to select channel for reading");
        return false;
    }

    if (!beginTemperatureOperation()) {
        DEBUG_PRINTLN("Failed to begin temperature operation");
        return false;
    }

    uint8_t scratchpad[9];
    wireWriteByte(0xCC); // Skip ROM
    wireWriteByte(0xBE); // Read Scratchpad
    
    DEBUG_PRINTLN("Reading scratchpad");
    for (int i = 0; i < 9; i++) {
        scratchpad[i] = wireReadByte();
    }
    
    printScratchpad(scratchpad);
    
    int16_t raw = (scratchpad[1] << 8) | scratchpad[0];
    *temperature = raw / 16.0;
    
    DEBUG_PRINT("Temperature: ");
    DEBUG_PRINT(*temperature);
    DEBUG_PRINTLN(" °C");
    
    currentState = DS2482State::IDLE;
    return true;
}

/**
 * Read sensor scratchpad data
 * @param scratchpad Array to store 9 bytes of scratchpad data
 * @return true if scratchpad read successfully
 */
bool DS2482::readScratchpad(uint8_t* scratchpad) {
    if (!beginTemperatureOperation()) {
        return false;
    }
    
    wireWriteByte(0xCC); // Skip ROM
    wireWriteByte(0xBE); // Read Scratchpad
    
    for (int i = 0; i < 9; i++) {
        scratchpad[i] = wireReadByte();
    }
    
    return true;
}

/**
 * Print scratchpad data (debug)
 * @param scratchpad Array of 9 bytes of scratchpad data
 */
void DS2482::printScratchpad(uint8_t* scratchpad) {
    DEBUG_PRINT("Scratchpad:");
    for (int i = 0; i < 9; i++) {
        DEBUG_PRINT(" ");
        DEBUG_PRINT_HEX(scratchpad[i]);
    }
    DEBUG_PRINTLN("");
}

/**
 * Write command to DS2482
 * @param command Command byte to write
 */
void DS2482::writeCommand(uint8_t command) {
    Wire.beginTransmission(address);
    Wire.write(command);
    Wire.endTransmission();
}

/**
 * Set the read pointer
 * @param readPointer Read pointer value
 */
void DS2482::setReadPointer(uint8_t readPointer) {
    Wire.beginTransmission(address);
    Wire.write(DS2482_CMD_SET_READ);
    Wire.write(readPointer);
    Wire.endTransmission();
}

/**
 * Wait for 1-Wire bus to be ready
 * @return true if bus becomes ready before timeout
 */
bool DS2482::waitFor1Wire() {
    unsigned long startTime = millis();
    while ((readStatus() & DS2482_STATUS_1WB) && (millis() - startTime < 100)) {
        delayMicroseconds(100);  // Short delay between checks
    }
    return (millis() - startTime < 100);
}

/**
 * Initialize temperature operation with wire reset
 * @return true if wire reset successful
 */
bool DS2482::beginTemperatureOperation() {
    currentState = DS2482State::IDLE;
    if (!wireReset()) {
        DEBUG_PRINTLN("1-Wire reset failed, no device detected");
        return false;
    }
    return true;
}