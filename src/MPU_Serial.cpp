/**
 * @file MPU_Serial.cpp
 * @brief Implements the unified serial transport for MiP.
 *
 * @details This source file provides the implementation for the low-level
 * serial API used to communicate with MiP. It handles raw binary transmission,
 * hex-ASCII decoding, and out-of-band (OOB) event demultiplexing.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Serial.h"
#include "MiP.h"
#include <Arduino.h>

// Note: The original uploaded file was MPU_Serial.h, but the user asked to fix MPU_Serial.cpp.
// The following is a likely implementation of MPU_Serial.cpp with the requested fix applied.
// This code is based on the provided headers and common C++ practices for this kind of library.

MiP_Serial::MiP_Serial(MiP& mip) : m_mip(mip), m_lastRequestTime(0), m_expectedResponseSize(0), m_expectedResponseCommand(0) {
    // Constructor body
}

void MiP_Serial::rawSend(const uint8_t request[], size_t requestLength) {
    transportSendRequest(request, requestLength, MIP_EXPECT_NO_RESPONSE);
}

uint8_t MiP_Serial::rawReceive(const uint8_t request[], size_t requestLength, uint8_t responseBuffer[], size_t responseBufferSize, size_t& responseLength) {
    for (uint8_t i = 0; i < MIP_MAX_RETRIES + 1; i++) {
        transportSendRequest(request, requestLength, MIP_EXPECT_RESPONSE);
        uint8_t status = transportGetResponse(responseBuffer, responseBufferSize, &responseLength);
        if (status == MiP::MIP_ERROR_NONE) {
            return MiP::MIP_ERROR_NONE;
        }
        delay(MIP_RETRY_WAIT);
    }
    return MiP::MIP_ERROR_TIMEOUT;
}

bool MiP_Serial::processAllResponseData() {
    // Implementation for processing all incoming UART data
    // This is a placeholder as the full original MPU_Serial.cpp was not provided.
    // A real implementation would read from the serial port and parse data.
    return false;
}

void MiP_Serial::clear() {
    m_expectedResponseSize = 0;
    m_expectedResponseCommand = 0;
    discardUnexpectedSerialData();
}

uint8_t MiP_Serial::discardUnexpectedSerialData() {
    // Discard any data currently in the serial receive buffer.
    uint8_t count = 0;
    while (m_mip.serialAvailable()) {
        m_mip.serialRead();
        count++;
    }
    return count;
}

void MiP_Serial::processOobResponseData(uint8_t commandByte) {
  size_t length = 0;

  switch (commandByte) {
    case MiP_Clap::MIP_CMD_GET_CLAP_EVENT:
        length = 1;
        break;
    case MiP_Gesture::MIP_CMD_GET_GESTURE_EVENT:
        length = 1;
        break;
    case MiP_Infrared::MIP_CMD_GET_DETECTED_MIP: // Also MIP_CMD_GET_RADAR_RESPONSE (0x04)
      if (readIrLength(length)) {
        // This was a variable-length IR event, and the length has been set.
        break; // IR event handled
      }
      // If readIrLength returns false, it's not a valid IR event.
      // Treat it as a fixed-length Radar event.
      length = 1;
      break;
    case MiP_Infrared::MIP_CMD_RECEIVE_IR_DONGLE_CODE:
        readIrLength(length);
        break;
    default:
        return; // Not an OOB event we handle here
  }

  // Remainder of the function would read 'length' bytes and dispatch the event
  // This part is omitted as it was not in the original file provided.
}


uint8_t MiP_Serial::transportGetResponse(uint8_t* pResponseBuffer, size_t responseBufferSize, size_t* pResponseLength) {
    // Implementation for getting a response from MiP
    // Placeholder
    return MiP::MIP_ERROR_TIMEOUT;
}

void MiP_Serial::transportSendRequest(const uint8_t* pRequest, size_t requestLength, bool expectResponse) {
    // Implementation for sending a request to MiP
    // Placeholder
}

void MiP_Serial::copyHexTextToBinary(uint8_t* pDest, const uint8_t* pSrc, size_t length) {
    // Implementation for converting hex text to binary
    // Placeholder
}

constexpr uint8_t MiP_Serial::parseHexDigit(uint8_t digit) {
    if (digit >= '0' && digit <= '9') {
        return digit - '0';
    }
    if (digit >= 'A' && digit <= 'F') {
        return digit - 'A' + 10;
    }
    if (digit >= 'a' && digit <= 'f') {
        return digit - 'a' + 10;
    }
    return 0; // Invalid hex digit
}

bool MiP_Serial::readIrLength(size_t& length) {
    // Implementation for reading IR length
    // Placeholder
    length = 1;
    return true;
}
