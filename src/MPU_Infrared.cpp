/**
 * @file MPU_Infrared.cpp
 * @brief Implements infrared communication for the MiP library.
 *
 * @details This source file implements infrared command handling and event
 * parsing.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Infrared.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Infrared::MiP_Infrared(MiP& mip) : m_mip(mip) {
  clear();
}

void MiP_Infrared::clear() {
  m_irCodeEvents.clear();
  m_detectedMiPEvents.clear();
  m_irId = 0x00;
}

void MiP_Infrared::enableMiPDetectionMode(uint8_t id, uint8_t txPower) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->enableMiPDetectionMode()"));
  m_irId = id;
  rawSetMiPDetectionMode(id, txPower);
}

void MiP_Infrared::disableMiPDetectionMode() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->disableMiPDetectionMode()"));
  m_irId = MIP_IR_DETECTION_MODE_DISABLE;

  // According to WowWee documentation, TX power must be between 1 and 120 even
  // when disabling.
  rawSetMiPDetectionMode(MIP_IR_DETECTION_MODE_DISABLE, 0x01);
}

bool MiP_Infrared::isMiPDetectionModeEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->isMiPDetectionModeEnabled()"));
  return m_irId > MIP_IR_DETECTION_MODE_DISABLE;
}

uint8_t MiP_Infrared::readDetectedMiP() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->readDetectedMiP()"));
  // Fetch bytes from the Serial receive buffer and process any event data found
  // within.
  m_mip.serial.processAllResponseData();
  uint8_t detectedMiPEvent = 0;
  if (!m_detectedMiPEvents.pop(detectedMiPEvent)) {
    m_mip.m_lastError = MiP::MIP_ERROR_NO_EVENT;
    return detectedMiPEvent;
  }
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return detectedMiPEvent;
}

uint8_t MiP_Infrared::availableDetectedMiPEvents() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->availableDetectedMiPEvents()"));
  // Fetch bytes from the Serial receive buffer and process any event data found
  // within.
  m_mip.serial.processAllResponseData();
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return m_detectedMiPEvents.available();
}

void MiP_Infrared::enableRemoteControl() {
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->enableRemoteControl()"));
  verifiedRemoteControl(MIP_IR_REMOTE_CONTROL_ENABLE);
}

void MiP_Infrared::disableRemoteControl() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->disableRemoteControl()"));
  verifiedRemoteControl(MIP_IR_REMOTE_CONTROL_DISABLE);
}

bool MiP_Infrared::isRemoteControlEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->isRemoteControlEnabled()"));
  uint8_t remoteControl = MIP_IR_REMOTE_CONTROL_DISABLE;
  int8_t result = rawGetRemoteControl(remoteControl);
  if (result != MiP::MIP_ERROR_NONE) {
    m_mip.m_lastError = result;
    return false;
  }
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return remoteControl == MIP_IR_REMOTE_CONTROL_ENABLE;
}

void MiP_Infrared::sendDongleCode(const MiPIRDongleCode& irCode, uint8_t transmitPower) {
  sendDongleCode(irCode.code, irCode.length, transmitPower);
}

void MiP_Infrared::sendDongleCode(uint32_t code, uint8_t length, uint8_t transmitPower) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->sendDongleCode()"));
  m_mip.MIP_ASSERT(transmitPower >= 1 && transmitPower <= 120);
  m_mip.MIP_ASSERT(length >= 2 && length <= 4);

  if (length < 2) length = 2;
  if (length > 4) length = 4;

  // Mask to the requested width (right-aligned in the 32-bit field).
  if (length < 4) { code &= (1UL << (length * 8)) - 1UL; }

  uint8_t command[1 + 4 + 1 + 1];
  command[0] = MIP_CMD_SEND_IR_DONGLE_CODE;

  // Always 4 data bytes, MSB first; unused high bytes are 0.
  command[1] = (length >= 4) ? static_cast<uint8_t>((code >> 24) & 0xFF) : 0;
  command[2] = (length >= 3) ? static_cast<uint8_t>((code >> 16) & 0xFF) : 0;
  command[3] = (length >= 2) ? static_cast<uint8_t>((code >> 8) & 0xFF) : 0;
  command[4] = static_cast<uint8_t>(code & 0xFF);

  command[5] = static_cast<uint8_t>(length * 8);  // 16, 24, or 32 bits
  command[6] = transmitPower;

  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

MiPIRDongleCode MiP_Infrared::readDongleCode() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->readDongleCode()"));
  m_mip.serial.processAllResponseData();

  MiPIRDongleCode irEvent;
  if (!m_irCodeEvents.pop(irEvent)) {
    m_mip.m_lastError = MiP::MIP_ERROR_NO_EVENT;
    return MiPIRDongleCode(0xFFFFFFFF, 0);
  }

  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return irEvent;
}

uint8_t MiP_Infrared::availableCodeEvents() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Infrared->availableCodeEvents()"));
  // Fetch bytes from the Serial receive buffer and process any event data found
  // within.
  m_mip.serial.processAllResponseData();
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return m_irCodeEvents.available();
}

void MiP_Infrared::processEvent(uint8_t command, const uint8_t* payload, size_t length) {
  switch (command) {
    case MIP_CMD_GET_DETECTED_MIP:
      if (length >= 2) { m_detectedMiPEvents.push(payload[1]); }
      break;

    case MIP_CMD_RECEIVE_IR_DONGLE_CODE:
      {
        uint8_t codeLen = static_cast<uint8_t>(length - 1);
        if (codeLen >= 2 && codeLen <= 4) {
          uint32_t parsedCode = 0;
          for (size_t i = 1; i < length; i++) {
            parsedCode = (parsedCode << 8) | payload[i];
          }
          m_irCodeEvents.push(MiPIRDongleCode(parsedCode, codeLen));
        }
        break;
      }

    default:
      MIP_DEBUG_WARN_PREFIX();
      MIP_DEBUG_WARN_PRINTLN(F("MiP: Unknown IR event"));
      break;
  }
}

// ==========================================================================
// Protected functions.
// ==========================================================================

// This internal protected method sends the set detection mode command with
// minimal error handling. The error recovery happens at a higher level of the
// driver.
void MiP_Infrared::rawSetMiPDetectionMode(uint8_t id, uint8_t txPower) {
  m_mip.MIP_ASSERT(0x01 <= txPower && txPower <= 0x78);
  uint8_t command[1 + 2] = { MIP_CMD_SET_DETECTION_MODE, id, txPower };
  m_mip.serial.rawSend(command, sizeof(command));
}

// This internal protected method verifies that IR remote control is enabled.
void MiP_Infrared::verifiedRemoteControl(uint8_t desiredRemoteControlMode) {
  int8_t result = MiP::MIP_ERROR_NONE;
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawSetRemoteControl(desiredRemoteControlMode);
    uint8_t actualMode = MIP_IR_REMOTE_CONTROL_DISABLE;

    // Read back and make sure that it was set as expected.
    result = rawGetRemoteControl(actualMode);
    if (result == MiP::MIP_ERROR_NONE && actualMode == desiredRemoteControlMode) {
      // The set was successful so return immediately.
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  if (result != MiP::MIP_ERROR_NONE) {
    // Kept getting an error back from rawGetIRRemoteControl().
    m_mip.m_lastError = result;
  } else {
    // rawGetIRRemoteControl() was successful but didn't match mode to which
    // we were attempting to change.
    m_mip.m_lastError = MiP::MIP_ERROR_MAX_RETRIES;
  }
}

// This internal protected method sends the get IR remote control status
// command with minimal error handling. The error recovery happens at a higher
// level of the driver.
int8_t MiP_Infrared::rawGetRemoteControl(uint8_t& remoteControl) {
  const uint8_t getIRRemoteControl[1] = { MIP_CMD_GET_IR_REMOTE_CONTROL };
  uint8_t response[1 + 1];
  size_t responseLength = 0;
  int8_t result = m_mip.serial.rawReceive(
    getIRRemoteControl, sizeof(getIRRemoteControl), response, sizeof(response), responseLength);
  if (result != MiP::MIP_ERROR_NONE) return result;
  if (responseLength != sizeof(response) || response[0] != MIP_CMD_GET_IR_REMOTE_CONTROL) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  remoteControl = response[1];
  return MiP::MIP_ERROR_NONE;
}

// This internal protected method sends the set IR remote control command with
// minimal error handling. The error recovery happens at a higher level of the
// driver.
void MiP_Infrared::rawSetRemoteControl(uint8_t remoteControl) {
  m_mip.MIP_ASSERT(remoteControl == MIP_IR_REMOTE_CONTROL_ENABLE
                   || remoteControl == MIP_IR_REMOTE_CONTROL_DISABLE);
  uint8_t command[1 + 1] = { MIP_CMD_SET_IR_REMOTE_CONTROL, remoteControl };
  m_mip.serial.rawSend(command, sizeof(command));
}
