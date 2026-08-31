/**
 * @file MPU_Weight.cpp
 * @brief Implements weight reporting for the MiP library.
 *
 * @details This source file implements weight-sensor reading logic and cached
 * event processing.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Weight.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Weight::MiP_Weight(MiP& mip) : m_mip(mip) {
  clear();
}

void MiP_Weight::processEvent(int8_t weightValue) {
  m_lastWeight = weightValue;
  m_mip.m_flags |= MiP::MIP_FLAG_WEIGHT_VALID;
}

void MiP_Weight::clear() {
  m_lastWeight = 0;
}

int8_t MiP_Weight::read() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Weight->read()"));

  // Fetch bytes from the Serial receive buffer and process any event data found
  // within.
  m_mip.serial.processAllResponseData();

  if (m_mip.m_flags & MiP::MIP_FLAG_WEIGHT_VALID) {
    m_mip.m_lastError = MiP::MIP_ERROR_NONE;
    return m_lastWeight;
  }

  int8_t result = MiP::MIP_ERROR_NONE;
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    int8_t weight = 0;
    result = rawGet(weight);
    if (result == MiP::MIP_ERROR_NONE) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      m_lastWeight = weight;
      m_mip.m_flags |= MiP::MIP_FLAG_WEIGHT_VALID;
      return weight;
    }

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  m_mip.m_lastError = result;
  return 0;
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

int8_t MiP_Weight::rawGet(int8_t& weight) {
  const uint8_t getWeight[1] = { MIP_CMD_GET_WEIGHT };
  uint8_t response[1 + 1];
  size_t responseLength = 0;

  int8_t result = m_mip.serial.rawReceive(
    getWeight, sizeof(getWeight), response, sizeof(response), responseLength);
  if (result) return result;
  return parse(weight, response, responseLength);
}

int8_t MiP_Weight::parse(int8_t& weight, const uint8_t response[], size_t responseLength) {
  if (responseLength != 2 || response[0] != MIP_CMD_GET_WEIGHT) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  weight = static_cast<int8_t>(response[1]);
  return MiP::MIP_ERROR_NONE;
}
