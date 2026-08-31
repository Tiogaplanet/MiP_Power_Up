/**
 * @file MPU_Odometer.cpp
 * @brief Implements odometer tracking for the MiP library.
 *
 * @details This source file implements odometer read and reset operations.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Odometer.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Odometer::MiP_Odometer(MiP& mip) : m_mip(mip) {}

float MiP_Odometer::read() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Odometer->read()"));

  int8_t result = MiP::MIP_ERROR_NONE;

  // Retry the read if it should fail on the first attempt.
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    float distance = 0.0f;
    result = rawRead(distance);
    if (result == MiP::MIP_ERROR_NONE) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return distance;
    }

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }
  m_mip.m_lastError = result;
  return 0.0f;
}

void MiP_Odometer::reset() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Odometer->reset()"));

  int8_t result = MiP::MIP_ERROR_NONE;

  // Retry the verified reset (send → read-back → compare).
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    result = verifiedReset();
    if (result == MiP::MIP_ERROR_NONE) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }

    // Wait before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  m_mip.m_lastError = result;
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

void MiP_Odometer::rawReset() {
  const uint8_t command[1] = { MIP_CMD_RESET_ODOMETER };
  // Blind send – verification is performed by verifiedReset().
  m_mip.serial.rawSend(command, sizeof(command));
}

int8_t MiP_Odometer::verifiedReset() {
  // 1. Send the reset command.
  rawReset();

  // 2. Read the odometer back.
  float distance = 0.0f;
  int8_t result = rawRead(distance);
  if (result != MiP::MIP_ERROR_NONE) { return result; }

  // 3. Compare: after a successful reset the reported distance must be
  //    essentially zero (allow a small epsilon for timing / float noise).
  if (distance > RESET_VERIFY_EPSILON_CM) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }

  return MiP::MIP_ERROR_NONE;
}

// This internal protected method sends the read odometer command with minimal
// error handling. The error recovery happens at a higher level of the driver.
int8_t MiP_Odometer::rawRead(float& distanceInCm) {
  const uint8_t readOdometer[1] = { MIP_CMD_READ_ODOMETER };
  uint8_t response[1 + 4];
  size_t responseLength = 0;
  int8_t result = m_mip.serial.rawReceive(
    readOdometer, sizeof(readOdometer), response, sizeof(response), responseLength);
  if (result) return result;
  if (responseLength != sizeof(response) || response[0] != MIP_CMD_READ_ODOMETER) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }

  // Tick count is stored as big-endian in response buffer.
  uint32_t ticks = (static_cast<uint32_t>(response[1]) << 24)
                   | (static_cast<uint32_t>(response[2]) << 16)
                   | (static_cast<uint32_t>(response[3]) << 8) | response[4];

  // Odometer has 48.5 ticks / cm.
  distanceInCm = static_cast<float>(ticks) / TICKS_PER_CM;
  return MiP::MIP_ERROR_NONE;
}
