/**
 * @file MPU_Radar.cpp
 * @brief Implements radar tracking for the MiP library.
 *
 * @details This source file implements radar mode switching and event handling.
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
}

void MiP_Radar::clear() {
  m_lastRadar = MIP_RADAR_INVALID;
}

void MiP_Radar::enable() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Radar->enable()"));

  verifiedSet(MIP_RADAR);
}

void MiP_Radar::disable() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Radar->disable()"));

  verifiedSet(MIP_RADAR_DISABLED);
}

bool MiP_Radar::isEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Radar->isEnabled()"));

  return check(MIP_RADAR);
}

MiPRadar MiP_Radar::read() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Radar->read()"));

  // Fetch bytes from the Serial receive buffer and process any event data found
  // within.
  m_mip.serial.processAllResponseData();
  if ((m_mip.m_flags & MiP::MIP_FLAG_RADAR_VALID) == 0) {
    // Haven't received a radar event yet.
    m_mip.m_lastError = MiP::MIP_ERROR_NO_EVENT;
    return MIP_RADAR_INVALID;
  }
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return m_lastRadar;
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

void MiP_Radar::processEvent(uint8_t radarCode) {
  if (radarCode >= MIP_RADAR_NONE && radarCode <= MIP_RADAR_0CM_10CM) {
    m_lastRadar = static_cast<MiPRadar>(radarCode);
    m_mip.m_flags |= MiP::MIP_FLAG_RADAR_VALID;
  }
}

// This internal protected method sends the command to change the radar/gesture
// mode and then sends a request to get the new state. If this request fails or
// the new state isn't as expected, it will retry the command.
void MiP_Radar::verifiedSet(MiPRadarMode desiredMode) {
  int8_t result = MiP::MIP_ERROR_NONE;

  // Always mark cached RADAR data as invalid when changing modes.
  m_mip.m_flags &= ~MiP::MIP_FLAG_RADAR_VALID;

  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawSet(desiredMode);

    // Read back and make sure that it was set as expected.
    MiPRadarMode actualMode = MIP_RADAR_DISABLED;
    result = rawGet(actualMode);
    if (result == MiP::MIP_ERROR_NONE && actualMode == desiredMode) {
      // The set was successful so return immediately.
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  if (result != MiP::MIP_ERROR_NONE) {
    // Kept getting an error back from rawGetGestureRadarMode().
    m_mip.m_lastError = result;
  } else {
    // rawGetGestureRadarMode() was successful but didn't match mode to which we
    // were attempting to change.
    m_mip.m_lastError = MiP::MIP_ERROR_MAX_RETRIES;
  }
}

// This internal protected method requests the current radar/gesture mode and
// then returns whether it matches the passed in value or not. It includes retry
// code incase the request should fail.
bool MiP_Radar::check(MiPRadarMode expectedMode) {
  int8_t result = MiP::MIP_ERROR_NONE;
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    MiPRadarMode currentMode;
    result = rawGet(currentMode);
    if (result == MiP::MIP_ERROR_NONE) return currentMode == expectedMode;

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }
  m_mip.m_lastError = result;
  return false;
}

// This internal protected method sends the get gesture/radar mode command with
// minimal error handling. The error recovery happens at a higher level of the
// driver.
int8_t MiP_Radar::rawGet(MiPRadarMode& mode) {
  const uint8_t getGestureRadarMode[1] = { MIP_CMD_GET_GESTURE_RADAR_MODE };
  uint8_t response[1 + 1];
  size_t responseLength = 0;
  int8_t result = m_mip.serial.rawReceive(
    getGestureRadarMode, sizeof(getGestureRadarMode), response, sizeof(response), responseLength);
  if (result) return result;
  if (responseLength != 2 || response[0] != MIP_CMD_GET_GESTURE_RADAR_MODE
      || (response[1] != MIP_GESTURE_RADAR_DISABLED
          && response[1] != MIP_GESTURE && response[1] != MIP_RADAR)) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  mode = static_cast<MiPRadarMode>(response[1]);
  return MiP::MIP_ERROR_NONE;
}

// This internal protected method sends the set gesture/radar mode command with
// no error checking. The error handling / recovery happens at a higher level of
// the driver.
void MiP_Radar::rawSet(MiPRadarMode mode) {
  uint8_t command[1 + 1] = { MIP_CMD_SET_GESTURE_RADAR_MODE, mode };
  m_mip.serial.rawSend(command, sizeof(command));
}
