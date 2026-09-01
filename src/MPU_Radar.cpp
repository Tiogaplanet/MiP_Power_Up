/**
 * @file MPU_Radar.cpp
 * @brief Implements radar proximity sensing for the MiP library.
 *
 * @details This source file implements continuous radar mode management,
 * cached event processing, and synchronous one-shot radar pings.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Radar.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Radar::MiP_Radar(MiP& mip) : m_mip(mip) {
  clear();
  m_isEnabled = false;
}

void MiP_Radar::enable() {
  MIP_DEBUG_INFO_PRINTLN(m_mip, F("MiP->Radar->enable()"));
  uint8_t command[1 + 1] = {MIP_CMD_SET_RADAR_MODE, MIP_RADAR};

  m_mip.serial.rawSend(command, sizeof(command));
  m_isEnabled = true;
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Radar::disable() {
  MIP_DEBUG_INFO_PRINTLN(m_mip, F("MiP->Radar->disable()"));
  uint8_t command[1 + 1] = {MIP_CMD_SET_RADAR_MODE, 0x00};

  m_mip.serial.rawSend(command, sizeof(command));
  m_isEnabled = false;
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

bool MiP_Radar::isEnabled() {
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return m_isEnabled;
}

MiPRadar MiP_Radar::read() {
  MIP_DEBUG_INFO_PRINTLN(m_mip, F("MiP->Radar->read()"));
  
  // Fetch bytes from the Serial receive buffer to process any pending OOB events.
  m_mip.serial.processAllResponseData();

  if (m_mip.m_flags & MiP::MIP_FLAG_RADAR_VALID) {
    m_mip.m_flags &= ~MiP::MIP_FLAG_RADAR_VALID;
    m_mip.m_lastError = MiP::MIP_ERROR_NONE;
    return m_currentRadar;
  }

  m_mip.m_lastError = MiP::MIP_ERROR_NO_EVENT;
  return MIP_RADAR_INVALID;
}

MiPRadar MiP_Radar::ping() {
  MIP_DEBUG_INFO_PRINTLN(m_mip, F("MiP->Radar->ping()"));
  
  const uint8_t request[1] = {MIP_CMD_RADAR_PING};
  uint8_t response[1 + 1];
  size_t responseLength = 0;

  // Send the one-shot query and block until MiP responds
  int8_t result = m_mip.serial.rawReceive(request, sizeof(request), response, sizeof(response), responseLength);
  
  if (result != MiP::MIP_ERROR_NONE) {
    m_mip.m_lastError = result;
    return MIP_RADAR_INVALID;
  }
  
  if (responseLength != 2 || response[0] != MIP_CMD_RADAR_PING) {
    m_mip.m_lastError = MiP::MIP_ERROR_BAD_RESPONSE;
    return MIP_RADAR_INVALID;
  }

  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return static_cast<MiPRadar>(response[1]);
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

void MiP_Radar::clear() {
  m_currentRadar = MIP_RADAR_INVALID;
  m_mip.m_flags &= ~MiP::MIP_FLAG_RADAR_VALID;
}

void MiP_Radar::processEvent(uint8_t radarValue) {
  m_currentRadar = static_cast<MiPRadar>(radarValue);
  m_mip.m_flags |= MiP::MIP_FLAG_RADAR_VALID;
}
