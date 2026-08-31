/**
 * @file MPU_Shake.cpp
 * @brief Implements shake detection for the MiP library.
 *
 * @details This source file implements shake-event parsing and state updates.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Shake.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Shake::MiP_Shake(MiP& mip) : m_mip(mip) {}

bool MiP_Shake::read() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Shake->read()"));

  // Fetch bytes from the Serial receive buffer and process any event data found
  // within.
  m_mip.serial.processAllResponseData();
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  if (m_mip.m_flags & MiP::MIP_FLAG_SHAKE_DETECTED) {
    // A shake event has been received since the last call to this function.
    // Return true and clear the shake detected bit.
    m_mip.m_flags &= ~MiP::MIP_FLAG_SHAKE_DETECTED;
    return true;
  }
  return false;
}
