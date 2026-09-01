/**
 * @file MPU_Serial.cpp
 * @brief Implements unified serial transport for ESP8266, ESP32, and AVR Pro Mini targets.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License, Version 2.0.
 */
#include "MPU_Serial.h"
#include "MiP_Power_Up.h"

// Define the hardware serial interface used to communicate with MiP based on architecture.
#if defined(ESP32)
  // ESP32-S2 will use Serial1 configured for pins 12 (RX) and 13 (TX) in MiP::begin()
  #define MIP_SERIAL Serial1
#else
  // ESP8266 (D1 mini) and AVR (Pro Mini) use the primary Serial interface
  #define MIP_SERIAL Serial
#endif

MiP_Serial::MiP_Serial(MiP& mip) : m_mip(mip) {
  clear();
}

void MiP_Serial::clear() {
  m_lastRequestTime = millis() - MIP_REQUEST_DELAY;
  m_expectedResponseSize = 0;
  m_expectedResponseCommand = 0;
  memset(m_responseBuffer, 0, sizeof(m_responseBuffer));

#if defined(__AVR__)
  pinMode(UART_SELECT_PIN, OUTPUT);
  digitalWrite(UART_SELECT_PIN, UART_SELECT_PC);
#endif
}

// ---------------------------------------------------------------------------
// Hardware Multiplexer Helpers (AVR Target Only)
// ---------------------------------------------------------------------------
#if defined(__AVR__)
void MiP_Serial::switchSerialToMiP() {
  digitalWrite(UART_SELECT_PIN, UART_SELECT_MIP);
  delay(5);  // Settling delay for UART multiplexer
}

void MiP_Serial::switchSerialToPC() {
  Serial.flush();
  digitalWrite(UART_SELECT_PIN, UART_SELECT_PC);
  delay(5);
}
#endif

// ---------------------------------------------------------------------------
// Public Transport API
// ---------------------------------------------------------------------------

void MiP_Serial::rawSend(const uint8_t request[], size_t requestLength) {
#if defined(__AVR__)
  switchSerialToMiP();
#endif

  transportSendRequest(request, requestLength, MIP_EXPECT_NO_RESPONSE);

#if defined(__AVR__)
  switchSerialToPC();
#endif
}

uint8_t MiP_Serial::rawReceive(const uint8_t request[],
                               size_t requestLength,
                               uint8_t responseBuffer[],
                               size_t responseBufferSize,
                               size_t& responseLength) {
#if defined(__AVR__)
  switchSerialToMiP();
#endif

  transportSendRequest(request, requestLength, MIP_EXPECT_RESPONSE);
  uint8_t result = transportGetResponse(responseBuffer, responseBufferSize, &responseLength);

#if defined(__AVR__)
  switchSerialToPC();
#endif

  return result;
}

bool MiP_Serial::processAllResponseData() {
#if defined(__AVR__)
  switchSerialToMiP();
#endif

  bool responseFound = false;
  uint8_t buffer[(MIP_RESPONSE_MAX_LEN - 1) * 2];
  size_t bytesToRead = 0;
  size_t bytesRead = 0;

  while (MIP_SERIAL.available() >= 2) {
    // Every MiP message starts with two hex ASCII digits that form the command byte.
    uint8_t commandByte = 0;

#if defined(__AVR__)
    // Pro Mini reads two Hex-ASCII characters per command byte
    uint8_t highNibble = MIP_SERIAL.read();
    uint8_t lowNibble = MIP_SERIAL.read();
    commandByte = (parseHexDigit(highNibble) << 4) | parseHexDigit(lowNibble);
#else
    // D1 mini reads raw binary byte
    commandByte = MIP_SERIAL.read();
#endif

    if (m_expectedResponseCommand != 0 && commandByte == m_expectedResponseCommand) {
      // This is the reply we are waiting for.
      m_responseBuffer[0] = commandByte;

      bytesToRead = m_expectedResponseSize - 1;
#if defined(__AVR__)
      bytesRead = MIP_SERIAL.readBytes(reinterpret_cast<char*>(buffer), bytesToRead * 2);
      if (bytesRead == bytesToRead * 2) {
        copyHexTextToBinary(&m_responseBuffer[1], buffer, bytesToRead);
        responseFound = true;
      }
#else
      bytesRead = MIP_SERIAL.readBytes(reinterpret_cast<char*>(&m_responseBuffer[1]), bytesToRead);
      if (bytesRead == bytesToRead) {
        responseFound = true;
      }
#endif
      else {
        // Incomplete response – abandon this attempt and flush partial framing.
        m_expectedResponseCommand = 0;
        m_expectedResponseSize = 0;
        m_responseBuffer[0] = 0;
        discardUnexpectedSerialData();

        MIP_DEBUG_ERROR_PREFIX();
        MIP_DEBUG_ERROR_PRINT(F("MiP: Response too short: "));
        MIP_DEBUG_ERROR_PRINT(static_cast<unsigned>(bytesRead));
        MIP_DEBUG_ERROR_PRINT(F(", expected "));
#if defined(__AVR__)
        MIP_DEBUG_ERROR_PRINTLN(static_cast<unsigned>(bytesToRead * 2));
#else
        MIP_DEBUG_ERROR_PRINTLN(static_cast<unsigned>(bytesToRead));
#endif
        break;
      }
    } else {
      // Not the expected reply -> treat as Out-Of-Band notification.
      processOobResponseData(commandByte);
    }
  }

#if defined(__AVR__)
  switchSerialToPC();
#endif

  return responseFound;
}

// ---------------------------------------------------------------------------
// Protected / Private helpers
// ---------------------------------------------------------------------------

void MiP_Serial::transportSendRequest(const uint8_t* pRequest,
                                      size_t requestLength,
                                      bool expectResponse) {
  m_mip.MIP_ASSERT(m_mip.isInitialized());

  // Honor the minimum inter-request delay.
  while (millis() - m_lastRequestTime < MIP_REQUEST_DELAY) { delay(1); }

  m_expectedResponseCommand = expectResponse ? pRequest[0] : 0;
  m_expectedResponseSize = 0;
  m_responseBuffer[0] = 0;

  // Transmit the raw binary bytes (MiP protocol uses binary over TX, Hex-ASCII over RX).
  while (requestLength-- > 0) { MIP_SERIAL.write(*pRequest++); }

  m_lastRequestTime = millis();
}

uint8_t MiP_Serial::transportGetResponse(uint8_t* pResponseBuffer,
                                         size_t responseBufferSize,
                                         size_t* pResponseLength) {
  m_mip.MIP_ASSERT(m_mip.isInitialized());
  m_mip.MIP_ASSERT(responseBufferSize <= MIP_RESPONSE_MAX_LEN);
  m_mip.MIP_ASSERT(m_expectedResponseCommand != 0);

  m_expectedResponseSize = static_cast<uint8_t>(responseBufferSize);

  uint32_t startTime = millis();
  bool responseFound = false;

  do {
    responseFound = processAllResponseData();
  } while (!responseFound && (millis() - startTime) < MIP_RESPONSE_TIMEOUT);

  if (!responseFound) {
    MIP_DEBUG_WARN_PREFIX();
    MIP_DEBUG_WARN_PRINTLN(F("MiP: Response timeout"));
    return MiP::MIP_ERROR_TIMEOUT;
  }

  // Copy the collected response to the caller and reset our state.
  memcpy(pResponseBuffer, m_responseBuffer, m_expectedResponseSize);
  *pResponseLength = m_expectedResponseSize;
  m_expectedResponseCommand = 0;
  m_expectedResponseSize = 0;
  m_responseBuffer[0] = 0;

  return MiP::MIP_ERROR_NONE;
}

void MiP_Serial::processOobResponseData(uint8_t commandByte) {
  size_t length = 0;
  size_t bytesRead = 0;

  // Determine payload length from the command byte.
  switch (commandByte) {
    case MiP_Radar::MIP_CMD_GET_RADAR_RESPONSE:
    // case MiP_Infrared::MIP_CMD_GET_DETECTED_MIP:  // Removed duplicate! Shares 0x04 with Radar.
    case MiP_Gesture::MIP_CMD_GET_GESTURE_RESPONSE:
    case MiP_Clap::MIP_CMD_CLAP_RESPONSE:
    case MiP_Weight::MIP_CMD_GET_WEIGHT:
      length = 1;
      break;

    case MiP_Shake::MIP_CMD_SHAKE_RESPONSE:
      length = 0;
      break;

    case MiP::MIP_CMD_GET_STATUS:
      length = 2;
      break;

    case MiP_Infrared::MIP_CMD_RECEIVE_IR_DONGLE_CODE:
      // Variable-length message – length is the next byte.
      if (!readIrLength(length)) { return; }
      break;

    default: {
      [[maybe_unused]] uint8_t discarded = discardUnexpectedSerialData();
      MIP_DEBUG_ERROR_PREFIX();
      MIP_DEBUG_ERROR_PRINT(F("MiP: Bad OOB command byte: 0x"));
      if (commandByte < 0x10) MIP_DEBUG_ERROR_PRINT(F("0"));
      MIP_DEBUG_ERROR_PRINT(commandByte, HEX);
      MIP_DEBUG_ERROR_PRINT(F(" (discarded "));
      MIP_DEBUG_ERROR_PRINT(static_cast<unsigned>(discarded));
      MIP_DEBUG_ERROR_PRINTLN(F(" bytes)"));
      return;
    }
  }

  uint8_t response[MIP_RESPONSE_MAX_LEN];
  response[0] = commandByte;

#if defined(__AVR__)
  uint8_t buffer[MIP_RESPONSE_MAX_LEN * 2];
  bytesRead = MIP_SERIAL.readBytes(reinterpret_cast<char*>(buffer), length * 2);
  if (bytesRead == length * 2) {
    copyHexTextToBinary(&response[1], buffer, length);
    m_mip.dispatchEvent(commandByte, response, length + 1);
  }
#else
  bytesRead = MIP_SERIAL.readBytes(reinterpret_cast<char*>(&response[1]), length);
  if (bytesRead == length) {
    m_mip.dispatchEvent(commandByte, response, length + 1);
  }
#endif
}

bool MiP_Serial::readIrLength(size_t& length) {
#if defined(__AVR__)
  uint8_t nibbles[2];
  if (MIP_SERIAL.readBytes(reinterpret_cast<char*>(nibbles), sizeof(nibbles)) != sizeof(nibbles)) {
    MIP_DEBUG_ERROR_PREFIX();
    MIP_DEBUG_ERROR_PRINTLN(F("MiP: Missing IR code length"));
    return false;
  }
  length = (parseHexDigit(nibbles[0]) << 4) | parseHexDigit(nibbles[1]);
#else
  uint8_t lenByte = 0;
  if (MIP_SERIAL.readBytes(reinterpret_cast<char*>(&lenByte), 1) != 1) {
    return false;
  }
  length = lenByte;
#endif

  if (length < 2 || length > 4) {
    uint8_t discarded = discardUnexpectedSerialData();
    MIP_DEBUG_ERROR_PREFIX();
    MIP_DEBUG_ERROR_PRINT(F("MiP: Bad IR code length: 0x"));
    MIP_DEBUG_ERROR_PRINT(static_cast<unsigned>(length), HEX);
    MIP_DEBUG_ERROR_PRINT(F(" (discarded "));
    MIP_DEBUG_ERROR_PRINT(static_cast<unsigned>(discarded));
    MIP_DEBUG_ERROR_PRINTLN(F(" bytes)"));
    return false;
  }
  return true;
}

uint8_t MiP_Serial::discardUnexpectedSerialData() {
  uint8_t discarded = 0;
  
  // Clear incoming hardware UART RX buffer with a slight delay between reads 
  // to ensure continuous draining of buffered data.
  while (MIP_SERIAL.available() > 0) {
    discarded++;
    MIP_SERIAL.read();
    delayMicroseconds(200);  // Safer polling interval across 9600 and 115200 baud
  }
  
  // Brief idle so a mid-byte framing error can finish propagating over UART.
  delay(2);
  
  while (MIP_SERIAL.available() > 0) {
    discarded++;
    MIP_SERIAL.read();
  }
  
  return discarded;
}

#if defined(__AVR__)
void MiP_Serial::copyHexTextToBinary(uint8_t* pDest, const uint8_t* pSrc, size_t length) {
  while (length-- > 0) {
    *pDest++ = (parseHexDigit(pSrc[0]) << 4) | parseHexDigit(pSrc[1]);
    pSrc += 2;
  }
}

constexpr uint8_t MiP_Serial::parseHexDigit(uint8_t digit) {
  return (digit >= '0' && digit <= '9') ? static_cast<uint8_t>(digit - '0')
       : (digit >= 'a' && digit <= 'f') ? static_cast<uint8_t>(digit - 'a' + 10)
       : (digit >= 'A' && digit <= 'F') ? static_cast<uint8_t>(digit - 'A' + 10)
       : 0;
}
#endif
