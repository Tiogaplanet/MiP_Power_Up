/**
 * @file MPU_Mode.cpp
 * @brief Implements mode switching for the MiP library.
 *
 * @details This source file implements mode-setting logic and status queries.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Mode.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Mode::MiP_Mode(MiP& mip) : m_mip(mip) {}

void MiP_Mode::enableApp() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->enableApp()"));
  verifiedSet(MIP_APP_MODE);
}

void MiP_Mode::enableCage() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->enableCage()"));
  verifiedSet(MIP_CAGE_MODE);
}

void MiP_Mode::enableDance() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->enableDance()"));
  verifiedSet(MIP_DANCE_MODE);
}

void MiP_Mode::enableStack() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->enableStack()"));
  verifiedSet(MIP_STACK_MODE);
}

void MiP_Mode::enableTrick() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->enableTrick()"));
  verifiedSet(MIP_TRICK_MODE);
}

void MiP_Mode::enableRoam() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->enableRoam()"));
  verifiedSet(MIP_ROAM_MODE);
}

bool MiP_Mode::isAppEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->isAppEnabled()"));
  return check(MIP_APP_MODE);
}

bool MiP_Mode::isCageEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->isCageEnabled()"));
  return check(MIP_CAGE_MODE);
}

bool MiP_Mode::isDanceEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->isDanceEnabled()"));
  return check(MIP_DANCE_MODE);
}

bool MiP_Mode::isStackEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->isStackEnabled()"));
  return check(MIP_STACK_MODE);
}

bool MiP_Mode::isTrickEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->isTrickEnabled()"));
  return check(MIP_TRICK_MODE);
}

bool MiP_Mode::isRoamEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->isRoamEnabled()"));
  return check(MIP_ROAM_MODE);
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

bool MiP_Mode::check(MiPGameMode expectedMode) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Mode->check()"));
  int8_t result = MiP::MIP_ERROR_NONE;
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    MiPGameMode currentMode;
    result = rawGet(currentMode);
    if (result == MiP::MIP_ERROR_NONE) return currentMode == expectedMode;

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }
  m_mip.m_lastError = result;
  return false;
}

// This internal protected method sends the set game mode command with no error
// checking. The error handling / recovery happens at a higher level of the
// driver.
void MiP_Mode::rawSet(MiPGameMode mode) {
  // Might not accept command if currently running another game mode so stop
  // first.
  m_mip.motion.stop();

  uint8_t command[1 + 1] = { MIP_CMD_SET_GAME_MODE, mode };
  m_mip.serial.rawSend(command, sizeof(command));
}

// This internal protected method sends the get game mode command with minimal
// error handling. The error recovery happens at a higher level of the driver.
int8_t MiP_Mode::rawGet(MiPGameMode& mode) {
  const uint8_t getGameMode[1] = { MIP_CMD_GET_GAME_MODE };
  uint8_t response[1 + 1];
  size_t responseLength = 0;

  // Might not accept get game mode command when currently running a game mode
  // so Stop first.
  m_mip.motion.stop();

  int8_t result = m_mip.serial.rawReceive(
    getGameMode, sizeof(getGameMode), response, sizeof(response), responseLength);
  if (result) return result;
  if (responseLength != 2 || response[0] != MIP_CMD_GET_GAME_MODE
      || (response[1] != MIP_APP_MODE && response[1] != MIP_CAGE_MODE
          && response[1] != MIP_TRACKING_MODE && response[1] != MIP_DANCE_MODE
          && response[1] != MIP_DEFAULT_MODE && response[1] != MIP_STACK_MODE
          && response[1] != MIP_TRICK_MODE && response[1] != MIP_ROAM_MODE)) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  mode = static_cast<MiPGameMode>(response[1]);

  // Restart the game mode now that we have successfully retrieved it.
  rawSet(mode);

  return MiP::MIP_ERROR_NONE;
}

// This internal protected method sends the command to change the game mode and
// then sends a request to get the new mode. If this request fails or the new
// mode isn't as expected, it will retry the command.
void MiP_Mode::verifiedSet(MiPGameMode desiredMode) {
  int8_t result = MiP::MIP_ERROR_NONE;
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawSet(desiredMode);

    // Read back and make sure that it was set as expected.
    MiPGameMode actualMode;
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
    // Kept getting an error back from rawGetGameMode().
    m_mip.m_lastError = result;
  } else {
    // rawGetGameMode() was successful but didn't match mode to which we were
    // attempting to change.
    m_mip.m_lastError = MiP::MIP_ERROR_MAX_RETRIES;
  }
}
