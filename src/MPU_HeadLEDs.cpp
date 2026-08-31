/**
 * @file MPU_HeadLEDs.cpp
 * @brief Implements head LED control for the MiP library.
 *
 * @details This source file implements head LED state updates and command
 * handling.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_HeadLEDs.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_HeadLEDs::MiP_HeadLEDs(MiP& mip) : m_mip(mip) {}

void MiP_HeadLEDs::read(MiPHeadLEDs& headLEDs) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->HeadLEDs->read()"));
  int8_t result = MiP::MIP_ERROR_NONE;

  // Retry the read if it should fail on the first attempt.
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    result = rawGet(headLEDs);
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

void MiP_HeadLEDs::write(MiPHeadLED led1, MiPHeadLED led2, MiPHeadLED led3, MiPHeadLED led4) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->HeadLEDs->write()"));
  int8_t result = MiP::MIP_ERROR_NONE;

  // Send the set command and then issue the corresponding get command. Retry if
  // the get fails or doesn't return the expected new setting.
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawSet(led1, led2, led3, led4);

    // Read back and make sure that it was set as expected.
    MiPHeadLEDs headLEDs;
    result = rawGet(headLEDs);
    if (result == MiP::MIP_ERROR_NONE && headLEDs.led1 == led1 && headLEDs.led2 == led2
        && headLEDs.led3 == led3 && headLEDs.led4 == led4) {
      // The set was successful so return immediately.
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  if (result != MiP::MIP_ERROR_NONE) {
    // Kept getting an error back from read attempt.
    m_mip.m_lastError = result;
  } else {
    // Read was successful but didn't match setting to which we were attempting
    // to change.
    m_mip.m_lastError = MiP::MIP_ERROR_MAX_RETRIES;
  }
}

void MiP_HeadLEDs::write(const MiPHeadLEDs& headLEDs) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->HeadLEDs->write()"));
  write(headLEDs.led1, headLEDs.led2, headLEDs.led3, headLEDs.led4);
}

void MiP_HeadLEDs::unverifiedWrite(
  MiPHeadLED led1, MiPHeadLED led2, MiPHeadLED led3, MiPHeadLED led4) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->HeadLEDs->unverifiedWrite()"));
  rawSet(led1, led2, led3, led4);
}

void MiP_HeadLEDs::unverifiedWrite(const MiPHeadLEDs& headLEDs) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->HeadLEDs->unverifiedWrite()"));
  unverifiedWrite(headLEDs.led1, headLEDs.led2, headLEDs.led3, headLEDs.led4);
}

// ==========================================================================
// Protected functions.
// ==========================================================================

// This internal protected method sends the get head LEDs command with minimal
// error handling. The error recovery happens at a higher level of the driver.
int8_t MiP_HeadLEDs::rawGet(MiPHeadLEDs& headLEDs) {
  const uint8_t getHeadLEDs[1] = { MIP_CMD_GET_HEAD_LEDS };
  uint8_t response[1 + 4];
  size_t responseLength = 0;
  int8_t result = m_mip.serial.rawReceive(
    getHeadLEDs, sizeof(getHeadLEDs), response, sizeof(response), responseLength);
  if (result) return result;
  if (responseLength != sizeof(response) || response[0] != MIP_CMD_GET_HEAD_LEDS
      || !isValidSingleLED(response[1]) || !isValidSingleLED(response[2])
      || !isValidSingleLED(response[3]) || !isValidSingleLED(response[4])) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  headLEDs.led1 = static_cast<MiPHeadLED>(response[1]);
  headLEDs.led2 = static_cast<MiPHeadLED>(response[2]);
  headLEDs.led3 = static_cast<MiPHeadLED>(response[3]);
  headLEDs.led4 = static_cast<MiPHeadLED>(response[4]);
  return MiP::MIP_ERROR_NONE;
}

// This internal protected method sends the set head LEDs command with no error
// checking. The error handling / recovery happens at a higher level of the
// driver.
void MiP_HeadLEDs::rawSet(MiPHeadLED led1, MiPHeadLED led2, MiPHeadLED led3, MiPHeadLED led4) {
  uint8_t command[1 + 4] = { MIP_CMD_SET_HEAD_LEDS, led1, led2, led3, led4 };
  m_mip.serial.rawSend(command, sizeof(command));
}

// This internal protected method is called to validate that each head LED value
// returned is within the expected range.
bool MiP_HeadLEDs::isValidSingleLED(uint8_t led) {
  return led <= MIP_HEAD_LED_BLINK_FAST;
}
