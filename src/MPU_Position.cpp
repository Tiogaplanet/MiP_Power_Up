/**
 * @file MPU_Position.cpp
 * @brief Implements position reporting for the MiP library.
 *
 * @details This source file implements position queries from cached MiP status
 * data.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Position.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Position::MiP_Position(MiP& mip) : m_mip(mip) {}

MiPPosition MiP_Position::read() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Position->read()"));

  // Fetch bytes from the Serial receive buffer and process any event data found
  // within.
  m_mip.serial.processAllResponseData();

  if (!m_mip.isInitialized()) {
    m_mip.m_lastError = MiP::MIP_ERROR_TIMEOUT;
    return MIP_POSITION_ON_BACK_WITH_KICKSTAND;
  }

  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return m_mip.m_lastStatus.position;
}

bool MiP_Position::isOnBack() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Position->isOnBack()"));

  return read() == MIP_POSITION_ON_BACK;
}

bool MiP_Position::isFaceDown() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Position->isFaceDown()"));

  return read() == MIP_POSITION_FACE_DOWN;
}

bool MiP_Position::isUpright() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Position->isUpright()"));

  return read() == MIP_POSITION_UPRIGHT;
}

bool MiP_Position::isPickedUp() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Position->isPickedUp()"));

  return read() == MIP_POSITION_PICKED_UP;
}

bool MiP_Position::isHandStanding() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Position->isHandStanding()"));

  return read() == MIP_POSITION_HAND_STAND;
}

bool MiP_Position::isFaceDownOnTray() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Position->isFaceDownOnTray()"));

  return read() == MIP_POSITION_FACE_DOWN_ON_TRAY;
}

bool MiP_Position::isOnBackWithKickstand() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Position->isOnBackWithKickstand()"));

  return read() == MIP_POSITION_ON_BACK_WITH_KICKSTAND;
}
