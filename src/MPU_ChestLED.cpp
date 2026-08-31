/**
 * @file MPU_ChestLED.cpp
 * @brief Implements chest LED control for the MiP library.
 *
 * @details This source file implements chest LED state updates and command
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
#include "MPU_ChestLED.h"
#include "MiP_Power_Up.h"

MiP_ChestLED::MiP_ChestLED(MiP& mip) : m_mip(mip) {}

void MiP_ChestLED::read(MiPChestLED& chestLED) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->ChestLED->read()"));

  int8_t result;

  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    result = rawGet(chestLED);
    if (result == MiP::MIP_ERROR_NONE) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  m_mip.m_lastError = result;
}

void MiP_ChestLED::write(uint8_t red, uint8_t green, uint8_t blue) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->ChestLED->write()"));

  int8_t result;

  // Blue channel is 6-bit; zero out lower 2 bits.
  blue &= ~3;

  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawSet(red, green, blue);

    MiPChestLED actualChestLED;
    result = rawGet(actualChestLED);
    if (result == MiP::MIP_ERROR_NONE && actualChestLED.red == red
        && actualChestLED.green == green && actualChestLED.blue == blue) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  m_mip.m_lastError = (result != MiP::MIP_ERROR_NONE) ? result : MiP::MIP_ERROR_MAX_RETRIES;
}

void MiP_ChestLED::write(
  uint8_t red, uint8_t green, uint8_t blue, uint16_t onTime, uint16_t offTime) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->ChestLED->write()"));

  int8_t result;

  // Convert on/off time from milliseconds to 20ms ticks.
  m_mip.MIP_ASSERT(onTime / 20 <= 255 && offTime / 20 <= 255);
  uint8_t onTicks = static_cast<uint8_t>((onTime + 10) / 20);
  uint8_t offTicks = static_cast<uint8_t>((offTime + 10) / 20);

  // Blue channel is 6-bit; zero out lower 2 bits.
  blue &= ~3;

  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawFlash(red, green, blue, onTicks, offTicks);

    MiPChestLED actualChestLED;
    result = rawGet(actualChestLED);
    if (result == MiP::MIP_ERROR_NONE && actualChestLED.red == red
        && actualChestLED.green == green && actualChestLED.blue == blue
        && actualChestLED.onTime == static_cast<uint16_t>(onTicks) * 20
        && actualChestLED.offTime == static_cast<uint16_t>(offTicks) * 20) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return;
    }
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  m_mip.m_lastError = (result != MiP::MIP_ERROR_NONE) ? result : MiP::MIP_ERROR_MAX_RETRIES;
}

void MiP_ChestLED::write(const MiPChestLED& chestLED) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->ChestLED->write()"));

  write(chestLED.red, chestLED.green, chestLED.blue, chestLED.onTime, chestLED.offTime);
}

void MiP_ChestLED::unverifiedWrite(uint8_t red, uint8_t green, uint8_t blue) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->ChestLED->unverifiedWrite()"));

  rawSet(red, green, blue);
}

void MiP_ChestLED::unverifiedWrite(
  uint8_t red, uint8_t green, uint8_t blue, uint16_t onTime, uint16_t offTime) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->ChestLED->unverifiedWrite()"));

  m_mip.MIP_ASSERT(onTime / 20 <= 255 && offTime / 20 <= 255);
  uint8_t onTicks = static_cast<uint8_t>((onTime + 10) / 20);
  uint8_t offTicks = static_cast<uint8_t>((offTime + 10) / 20);
  rawFlash(red, green, blue, onTicks, offTicks);
}

void MiP_ChestLED::unverifiedWrite(const MiPChestLED& chestLED) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->ChestLED->unverifiedWrite()"));

  unverifiedWrite(
    chestLED.red, chestLED.green, chestLED.blue, chestLED.onTime, chestLED.offTime);
}

int8_t MiP_ChestLED::rawGet(MiPChestLED& chestLED) {
  const uint8_t getChestLED[1] = { MIP_CMD_GET_CHEST_LED };
  uint8_t response[1 + 5];
  size_t responseLength = 0;
  uint8_t result = m_mip.serial.rawReceive(
    getChestLED, sizeof(getChestLED), response, sizeof(response), responseLength);
  if (result) return result;
  if (responseLength != sizeof(response) || response[0] != MIP_CMD_GET_CHEST_LED) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  chestLED.red = response[1];
  chestLED.green = response[2];
  chestLED.blue = response[3];

  // Convert on/off time from 20ms ticks back to milliseconds.
  chestLED.onTime = static_cast<uint16_t>(response[4]) * 20;
  chestLED.offTime = static_cast<uint16_t>(response[5]) * 20;
  return MiP::MIP_ERROR_NONE;
}

void MiP_ChestLED::rawSet(uint8_t red, uint8_t green, uint8_t blue) {
  uint8_t command[1 + 3] = { MIP_CMD_SET_CHEST_LED, red, green, blue };
  m_mip.serial.rawSend(command, sizeof(command));
}

void MiP_ChestLED::rawFlash(
  uint8_t red, uint8_t green, uint8_t blue, uint8_t onTicks, uint8_t offTicks) {
  uint8_t command[1 + 5] = {
    MIP_CMD_FLASH_CHEST_LED, red, green, blue, onTicks, offTicks
  };
  m_mip.serial.rawSend(command, sizeof(command));
}
