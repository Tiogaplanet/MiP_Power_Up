/**
 * @file MiP_Power_Up.cpp
 * @brief Implements the core MiP API and library initialization.
 *
 * @details This source file implements the main MiP object lifecycle and shared
 * multi-architecture hardware initialization logic.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0.
 */
#include "MiP_Power_Up.h"

MiP::MiP()
  : battery(*this), chestLED(*this), clap(*this), console(*this), eeprom(*this),
    gesture(*this), headLEDs(*this), infrared(*this), mode(*this),
    motion(*this), odometer(*this), position(*this), radar(*this),
    serial(*this), shake(*this), sound(*this), version(*this), weight(*this),
#if defined(ESP8266) || defined(ESP32)
    wifi(*this),
#endif
    m_mip(*this) {
  clear();
}

MiP::~MiP() {
  end();
}

// ---------------------------------------------------------------------------
// Hardware Multiplexer Helpers (AVR Only)
// ---------------------------------------------------------------------------
#if defined(__AVR__)
void MiP::switchSerialToMiP() {
  if (!m_serialGoingToMiP) {
    digitalWrite(UART_SELECT_PIN, HIGH);
    delayMicroseconds(50);  // let the mux settle
    m_serialGoingToMiP = true;
  }
}

void MiP::switchSerialToPC() {
  if (m_serialGoingToMiP) {
    digitalWrite(UART_SELECT_PIN, LOW);
    delayMicroseconds(50);  // let the mux settle
    m_serialGoingToMiP = false;
  }
}
#endif

// ---------------------------------------------------------------------------
// Core Lifecycle
// ---------------------------------------------------------------------------

bool MiP::begin() {
  // Set up the PC console / debug channel based on architecture
#if defined(ESP8266)
  Serial1.begin(ESP8266_DEBUG_BAUD_RATE);
#elif defined(ESP32)
  Serial.begin(115200);
#elif defined(__AVR__)
  clear();  // mux -> PC
  Serial.begin(MIP_FAST_BAUD_RATE);
#endif

  clear();
  m_flags |= MIP_FLAG_INITIALIZED;

  for (int8_t retry = 0; retry < MIP_MAX_BEGIN_RETRIES; ++retry) {
    MIP_DEBUG_INFO_PREFIX();
    MIP_DEBUG_INFO_PRINTLN(F("Attempting 115200"));

    if (attemptMiPConnection(MIP_FAST_BAUD_RATE) == MIP_ERROR_NONE) {
#if defined(__AVR__)
      switchSerialToMiP();  // Keep 115200 and route to MiP
#endif
      return true;
    }

    MIP_DEBUG_INFO_PREFIX();
    MIP_DEBUG_INFO_PRINTLN(F("Attempting 9600"));

    if (attemptMiPConnection(MIP_SLOW_BAUD_RATE) == MIP_ERROR_NONE) {
#if defined(__AVR__)
      // MiP only spoke at 9600. Switch UART back to 115200 so the console
      // works, then leave the mux routed to MiP.
      Serial.begin(MIP_FAST_BAUD_RATE);
      switchSerialToMiP();
#endif
      return true;
    }
  }

  m_flags &= ~MIP_FLAG_INITIALIZED;
  end();
  return false;
}

void MiP::end() {
  if (isInitialized()) {
    sound.end();
    const uint8_t command[] = { MIP_CMD_DISCONNECT_APP };
    serial.rawSend(command, sizeof(command));

#if defined(ESP32)
    Serial1.flush();
#else
    Serial.flush();
#endif
  }

  clear();

  // Shut down the MiP UART link, but KEEP the PC Console alive for error
  // reporting!
#if defined(__AVR__)
  // AVR shares the Serial port. Do not call Serial.end() so the console still
  // works.
#elif defined(ESP8266)
  Serial.swap();
  Serial.end();  // Kills the MiP link on Serial
  // Serial1.end();  <--- REMOVE THIS! Keep the debug console alive.
#elif defined(ESP32)
  Serial1.end();  // Kills the MiP link on Serial1 (Console is on Serial,
                  // untouched)
#endif
}

void MiP::sleep() {
  const uint8_t command[] = { MIP_CMD_SLEEP };
  serial.rawSend(command, sizeof(command));
}

bool MiP::isInitialized() const {
  return (m_flags & MIP_FLAG_INITIALIZED);
}

int8_t MiP::lastCallResult() const {
  return m_lastError;
}

bool MiP::didLastCallFail() const {
  return m_lastError != MIP_ERROR_NONE;
}

void MiP::printLastCallResult() {
  if (m_lastError != MIP_ERROR_NONE) {
    MIP_DEBUG_ERROR_PREFIX();
    MIP_DEBUG_ERROR_PRINT(F("MiP: API returned "));

    switch (m_lastError) {
      case MIP_ERROR_TIMEOUT:
        MIP_DEBUG_ERROR_PRINTLN(F("MIP_ERROR_TIMEOUT (Timed out waiting for response)"));
        break;
      case MIP_ERROR_NO_EVENT:
        MIP_DEBUG_ERROR_PRINTLN(F("MIP_ERROR_NO_EVENT (No event has arrived from MiP yet)"));
        break;
      case MIP_ERROR_BAD_RESPONSE:
        MIP_DEBUG_ERROR_PRINTLN(F("MIP_ERROR_BAD_RESPONSE (Unexpected response from MiP)"));
        break;
      case MIP_ERROR_MAX_RETRIES:
        MIP_DEBUG_ERROR_PRINTLN(
          F("MIP_ERROR_MAX_RETRIES (Exceeded maximum number of retries)"));
        break;
      default: MIP_DEBUG_ERROR_PRINTLN(F("unknown error")); break;
    }
  }
}

uint32_t MiP::getBaudRate() const {
  return m_baudRate;
}

// ---------------------------------------------------------------------------
// Protected / Internal Methods
// ---------------------------------------------------------------------------

void MiP::clear() {
  m_baudRate = 0;
  m_flags = 0;
  m_lastError = MIP_ERROR_NONE;
  m_lastStatus.clear();
  clap.clear();
  gesture.clear();
  infrared.clear();
  radar.clear();
  serial.clear();
  weight.clear();

#if defined(ESP8266) || defined(ESP32)
  wifi.clear();
#elif defined(__AVR__)
  pinMode(UART_SELECT_PIN, OUTPUT);
  digitalWrite(UART_SELECT_PIN, LOW);
  m_serialGoingToMiP = false;
#endif
}

int8_t MiP::attemptMiPConnection(uint32_t baudRate) {
#if defined(__AVR__)
  switchSerialToMiP();
  Serial.begin(baudRate);
#elif defined(ESP8266)
  Serial.end();
  delay(20);
  Serial.begin(baudRate, SERIAL_8N1);
  Serial.swap();  // -> GPIO15 TX / GPIO13 RX
  Serial.flush();
  while (Serial.available() > 0) { Serial.read(); }
#elif defined(ESP32)
  Serial1.end();
  delay(20);
  // ESP32-S2 utilizes pins 12 (RX) and 13 (TX) for hardware Serial1 to MiP
  Serial1.begin(baudRate, SERIAL_8N1, 12, 13);
  Serial1.flush();
  while (Serial1.available() > 0) { Serial1.read(); }
#endif

  // 0xFF tells MiP to enable its UART
  const uint8_t initMipCommand[] = { 0xFF };
  serial.rawSend(initMipCommand, sizeof(initMipCommand));

  // Required spec delay (give 9600 baud extra settle time)
#if defined(__AVR__)
  delay(30);
#else
  delay(baudRate <= 9600 ? 50 : 30);
#endif

  serial.discardUnexpectedSerialData();

  int8_t result = rawGetStatus(m_lastStatus);

#if defined(__AVR__)
  switchSerialToPC();
#elif defined(ESP8266)
  if (result != MIP_ERROR_NONE) {
    Serial.swap();
    Serial.end();
    delay(MIP_BEGIN_RETRY_WAIT);
  }
#elif defined(ESP32)
  if (result != MIP_ERROR_NONE) {
    Serial1.end();
    delay(MIP_BEGIN_RETRY_WAIT);
  }
#endif

  if (result == MIP_ERROR_NONE) {
    MIP_DEBUG_INFO_PREFIX();
    MIP_DEBUG_INFO_PRINT(F("MiP: Connected at "));
    MIP_DEBUG_INFO_PRINT((unsigned long)baudRate);
    MIP_DEBUG_INFO_PRINTLN(F(" baud"));
    m_baudRate = baudRate;
  }

  return result;
}

void MiP::dispatchEvent(uint8_t command, const uint8_t* payload, size_t length) {
  switch (command) {
    case MiP_Clap::MIP_CMD_CLAP_RESPONSE:
      if (length >= 2) clap.processEvent(payload[1]);
      break;
    case MiP_Weight::MIP_CMD_GET_WEIGHT:
      if (length >= 2) weight.processEvent(payload[1]);
      break;
    case MiP_Gesture::MIP_CMD_GET_GESTURE_RESPONSE:
      if (length >= 2) gesture.processEvent(payload[1]);
      break;
    case MIP_CMD_GET_STATUS:
      parseStatus(this->m_lastStatus, payload, length);
      break;
    case MiP_Infrared::MIP_CMD_GET_DETECTED_MIP:
    case MiP_Infrared::MIP_CMD_RECEIVE_IR_DONGLE_CODE:
      infrared.processEvent(command, payload, length);
      break;
    case MiP_Shake::MIP_CMD_SHAKE_RESPONSE:
      m_flags |= MIP_FLAG_SHAKE_DETECTED;
      break;
    case MiP_Radar::MIP_CMD_GET_RADAR_RESPONSE:
      if (length >= 2) radar.processEvent(payload[1]);
      break;
    default:
      MIP_DEBUG_WARN_PREFIX();
      MIP_DEBUG_WARN_PRINT(F("MiP: Unknown OOB Event: 0x"));
      if (command < 0x10) { MIP_DEBUG_WARN_PRINT(F("0")); }
      MIP_DEBUG_WARN_PRINTLN(command, HEX);
      break;
  }
}

int8_t MiP::rawGetStatus(MiPStatus& status) {
  const uint8_t getStatus[1] = { MIP_CMD_GET_STATUS };
  uint8_t response[1 + 2];
  size_t responseLength;
  int result = serial.rawReceive(
    getStatus, sizeof(getStatus), response, sizeof(response), responseLength);
  if (result) return result;
  return parseStatus(status, response, responseLength);
}

int8_t MiP::parseStatus(MiPStatus& status, const uint8_t response[], size_t responseLength) {
  if (responseLength != 3 || response[0] != MIP_CMD_GET_STATUS
      || response[2] > MIP_POSITION_ON_BACK_WITH_KICKSTAND) {
    return MIP_ERROR_BAD_RESPONSE;
  }

  status.battery =
    (float)(((response[1] - 0x4D) / (float)(0x7C - 0x4D)) * (6.4f - 4.0f)) + 4.0f;
  status.position = static_cast<MiPPosition>(response[2]);
  return MIP_ERROR_NONE;
}

void MiP::mipAssert(bool condition, [[maybe_unused]] uint32_t lineNumber,
                    [[maybe_unused]] const char* fileName) {
  if (!condition) {
    MIP_DEBUG_ERROR_PREFIX();
    MIP_DEBUG_ERROR_PRINT(F("MiP: Assert failed in file "));
    MIP_DEBUG_ERROR_PRINT(fileName);
    MIP_DEBUG_ERROR_PRINT(F(" at line: "));
    MIP_DEBUG_ERROR_PRINTLN(lineNumber);

    while (true) { delay(100); }
  }
}
