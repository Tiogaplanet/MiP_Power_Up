/**
 * @file MPU_Battery.cpp
 * @brief Implements battery voltage monitoring for the MiP library.
 *
 * @details This source file implements the battery-monitoring logic used by the
 * MiP library.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Battery.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Battery::MiP_Battery(MiP& mip) : m_mip(mip) {}

/**
 * @brief Reads the most recent cached value of MiP's battery voltage.
 *
 * This function processes any pending Out-Of-Band status events to keep the
 * cache up to date. It does not transmit a new request to MiP.
 *
 * @return float Battery voltage in Volts, typically 4.0V (low) to 6.4V (fully
 * charged). Returns 0.0f if MiP is uninitialized.
 */
float MiP_Battery::readVoltage() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Battery->readVoltage()"));

  // Fetch bytes from the Serial receive buffer and process any event data found
  // within.
  m_mip.serial.processAllResponseData();

  if (!m_mip.isInitialized()) {
    m_mip.m_lastError = MiP::MIP_ERROR_TIMEOUT;
    return 0.0f;
  }

  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return m_mip.m_lastStatus.battery;
}
