/**
 * @file MPU_Clap.cpp
 * @brief Implements clap event handling for the MiP library.
 *
 * @details This source file implements clap event parsing and queue management.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Clap.h"
#include "MiP_Power_Up.h"

MiP_Clap::MiP_Clap(MiP& mip) : m_mip(mip) {
  clear();
}

void MiP_Clap::enableEvents() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Clap->enableClapEvents()"));
  checkedEnableEvents(MIP_CLAP_ENABLED);
}

void MiP_Clap::disableEvents() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Clap->disableEvents()"));
  checkedEnableEvents(MIP_CLAP_DISABLED);
}

bool MiP_Clap::areEventsEnabled() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Clap->areEventsEnabled()"));
  MiPClapSettings settings;
  int8_t result = readSettings(settings);
  if (result != MiP::MIP_ERROR_NONE) {
    m_mip.m_lastError = result;
    return false;
  }
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return settings.enabled == MIP_CLAP_ENABLED;
}

uint8_t MiP_Clap::availableEvents() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Clap->availableEvents()"));
  m_mip.serial.processAllResponseData();

  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return m_clapEvents.available();
}

uint8_t MiP_Clap::readEvent() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Clap->readEvent()"));
  m_mip.serial.processAllResponseData();

  uint8_t clapEvent = 0;
  if (!m_clapEvents.pop(clapEvent)) {
    m_mip.m_lastError = MiP::MIP_ERROR_NO_EVENT;
    return 0;
  }
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return clapEvent;
}

void MiP_Clap::processEvent(uint8_t clapCode) {
  m_clapEvents.push(clapCode);
}

uint16_t MiP_Clap::readDelay() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Clap->readDelay()"));
  MiPClapSettings settings;
  int8_t result = readSettings(settings);
  if (result != MiP::MIP_ERROR_NONE) {
    m_mip.m_lastError = result;
    return 0;
  }
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
  return settings.delay;
}

void MiP_Clap::writeDelay(uint16_t delayTime) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Clap->writeDelay()"));
  int8_t result = MiP::MIP_ERROR_NONE;

  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawSetDelay(delayTime);

    MiPClapSettings settings;
    result = rawGetSettings(settings);
    if (result == MiP::MIP_ERROR_NONE && settings.delay == delayTime) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  m_mip.m_lastError = (result != MiP::MIP_ERROR_NONE) ? result : MiP::MIP_ERROR_MAX_RETRIES;
}

void MiP_Clap::clear() {
  m_clapEvents.clear();
}

void MiP_Clap::checkedEnableEvents(MiPClapEnabled enabled) {
  int8_t result = MiP::MIP_ERROR_NONE;
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawEnable(enabled);

    MiPClapSettings settings;
    result = rawGetSettings(settings);
    if (result == MiP::MIP_ERROR_NONE && settings.enabled == enabled) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  m_mip.m_lastError = (result != MiP::MIP_ERROR_NONE) ? result : MiP::MIP_ERROR_MAX_RETRIES;
}

int8_t MiP_Clap::readSettings(MiPClapSettings& settings) {
  int8_t result = MiP::MIP_ERROR_NONE;

  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    result = rawGetSettings(settings);
    if (result == MiP::MIP_ERROR_NONE) return MiP::MIP_ERROR_NONE;

    delay(MiP_Serial::MIP_RETRY_WAIT);
  }
  settings.clear();
  return result;
}

void MiP_Clap::rawEnable(MiPClapEnabled enabled) {
  uint8_t command[1 + 1] = { MIP_CMD_ENABLE_CLAP, static_cast<uint8_t>(enabled) };
  m_mip.serial.rawSend(command, sizeof(command));
}

void MiP_Clap::rawSetDelay(uint16_t delay) {
  uint8_t command[1 + 2] = { MIP_CMD_SET_CLAP_DELAY, static_cast<uint8_t>(delay >> 8),
                             static_cast<uint8_t>(delay & 0xFF) };
  m_mip.serial.rawSend(command, sizeof(command));
}

int8_t MiP_Clap::rawGetSettings(MiPClapSettings& settings) {
  const uint8_t getClapSettings[1] = { MIP_CMD_GET_CLAP_SETTINGS };
  uint8_t response[1 + 3];
  size_t responseLength = 0;
  int8_t result = m_mip.serial.rawReceive(
    getClapSettings, sizeof(getClapSettings), response, sizeof(response), responseLength);

  if (result) return result;

  if (responseLength != sizeof(response) || response[0] != MIP_CMD_GET_CLAP_SETTINGS
      || (response[1] != MIP_CLAP_DISABLED && response[1] != MIP_CLAP_ENABLED)) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }

  settings.enabled = static_cast<MiPClapEnabled>(response[1]);
  settings.delay = (static_cast<uint16_t>(response[2]) << 8) | response[3];
  return MiP::MIP_ERROR_NONE;
}
