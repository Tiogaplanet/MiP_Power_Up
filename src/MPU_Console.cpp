/**
 * @file MPU_Console.cpp
 * @brief Implements multi-architecture console output for the MiP library.
 *
 * @details This source file routes Print class bytes to the correct hardware
 * serial port depending on the target microcontroller (AVR, ESP8266, ESP32).
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Console.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference and initialize pins.
MiP_Console::MiP_Console(MiP& mip) : m_mip(mip) {
#if defined(__AVR__)
  pinMode(UART_SELECT_PIN, OUTPUT);
  digitalWrite(UART_SELECT_PIN, UART_SELECT_MIP);
#endif
}

size_t MiP_Console::write(uint8_t byte) {
#if defined(__AVR__)
  switchSerialToPC();
  size_t ret = Serial.write(byte);
  switchSerialToMiP();
  return ret;
#elif defined(ESP8266)
  // D1 mini uses Serial1 for the debug console
  return Serial1.write(byte);
#elif defined(ESP32)
  // ESP32-S2 routes native USB CDC to Serial
  return Serial.write(byte);
#else
  return Serial.write(byte);
#endif
}

size_t MiP_Console::write(const uint8_t* buffer, size_t size) {
#if defined(__AVR__)
  switchSerialToPC();
  size_t ret = Serial.write(buffer, size);
  switchSerialToMiP();
  return ret;
#elif defined(ESP8266)
  return Serial1.write(buffer, size);
#elif defined(ESP32)
  return Serial.write(buffer, size);
#else
  return Serial.write(buffer, size);
#endif
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

#if defined(__AVR__)
void MiP_Console::switchSerialToMiP() {
  Serial.flush();
  digitalWrite(UART_SELECT_PIN, UART_SELECT_MIP);
  delay(5);
}

void MiP_Console::switchSerialToPC() {
  Serial.flush();
  digitalWrite(UART_SELECT_PIN, UART_SELECT_PC);
  delay(5);
}
#endif
