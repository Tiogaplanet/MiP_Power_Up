/**
 * @file MPU_Version.cpp
 * @brief Implements version reporting for the MiP library.
 *
 * @details This source file implements MiP's software and hardware version
 * queries and reports the MPU version number and string.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Version.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Version::MiP_Version(MiP& mip) : m_mip(mip) {}

void MiP_Version::readHardware(MiPHardwareInfo& hardware) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Version->readHardware()"));

  int8_t result = MiP::MIP_ERROR_NONE;

  // Retry the read if it should fail on the first attempt.
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    result = rawGetHardware(hardware);
    if (result == MiP::MIP_ERROR_NONE) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }
  m_mip.m_lastError = result;
}

void MiP_Version::readSoftware(MiPSoftwareVersion& software) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Version->readSoftware()"));

  int8_t result = MiP::MIP_ERROR_NONE;

  // Retry the read if it should fail on the first attempt.
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    result = rawGetSoftware(software);
    if (result == MiP::MIP_ERROR_NONE) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }
  m_mip.m_lastError = result;
}

const char* MiP_Version::readMPUString() const {
  return MIP_POWER_UP_VERSION;
}

uint32_t MiP_Version::readMPUNumber() const {
  return MIP_POWER_UP_VERSION_NUMBER;
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

// This internal protected method sends the get hardware info command with
// minimal error handling. The error recovery happens at a higher level of the
// driver.
int8_t MiP_Version::rawGetHardware(MiPHardwareInfo& hardware) {
  const uint8_t getHardwareInfo[1] = { MIP_CMD_GET_HARDWARE_INFO };
  uint8_t response[1 + 2];
  size_t responseLength = 0;
  int8_t result = m_mip.serial.rawReceive(
    getHardwareInfo, sizeof(getHardwareInfo), response, sizeof(response), responseLength);
  if (result) return result;
  if (responseLength != sizeof(response) || response[0] != MIP_CMD_GET_HARDWARE_INFO) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  hardware.voiceChip = response[1];
  hardware.hardware = response[2];
  return MiP::MIP_ERROR_NONE;
}

// This internal protected method sends the get software version command with
// minimal error handling. The error recovery happens at a higher level of the
// driver.
int8_t MiP_Version::rawGetSoftware(MiPSoftwareVersion& software) {
  const uint8_t getSoftwareVersion[1] = { MIP_CMD_GET_SOFTWARE_VERSION };
  uint8_t response[1 + 4];
  size_t responseLength = 0;
  int8_t result = m_mip.serial.rawReceive(
    getSoftwareVersion, sizeof(getSoftwareVersion), response, sizeof(response), responseLength);
  if (result) return result;
  if (responseLength != sizeof(response) || response[0] != MIP_CMD_GET_SOFTWARE_VERSION) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  software.year = 2000 + response[1];
  software.month = response[2];
  software.day = response[3];
  software.uniqueVersion = response[4];
  return MiP::MIP_ERROR_NONE;
}
