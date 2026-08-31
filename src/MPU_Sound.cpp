/**
 * @file MPU_Sound.cpp
 * @brief Implements sound playback for the MiP library.
 *
 * @details This source file implements sound-list creation, playback, and
 * volume control.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Sound.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Sound::MiP_Sound(MiP& mip) : m_mip(mip) {
  m_playVolume = MIP_VOLUME_OFF;
  m_soundIndex = -1;
  memset(m_playCommand, 0, sizeof(m_playCommand));
}

void MiP_Sound::beginList() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Sound->beginList()"));

  m_soundIndex = 0;
  m_playVolume = MIP_VOLUME_DEFAULT;
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Sound::addEntryToList(MiPSoundIndex sound, uint16_t delayTime /* = 0 */,
                               MiPVolume volume /* = MIP_VOLUME_DEFAULT */) {
  // Must call beginSoundList() before calling this function.
  m_mip.MIP_ASSERT(m_soundIndex != -1);

  // Delay is in units of 30 msecs and can't exceed 255 * 30.
  m_mip.MIP_ASSERT(delayTime <= 255 * 30);

  // Volume can only be set to values between 0 and 7 or 0xFF (which means keep
  // volume as it was).
  m_mip.MIP_ASSERT(volume <= MIP_VOLUME_7 || volume == MIP_VOLUME_DEFAULT);

  // Need to issue volume command if volume is being changed and
  // if we have to inject a volume instruction, verify we don't overflow the
  // buffer.
  if (volume != MIP_VOLUME_DEFAULT && volume != m_playVolume) {
    // Safe check to prevent index 18 out-of-bounds write.
    // The sound list can only hold 8 sound entries.
    m_mip.MIP_ASSERT(m_soundIndex < 8);
    m_playCommand[1 + m_soundIndex * 2] = MIP_SOUND_VOLUME_OFF + volume;
    m_playCommand[1 + m_soundIndex * 2 + 1] = 0;
    m_playVolume = volume;
    m_soundIndex++;
  }

  // The sound list can only hold 8 sound entries.
  m_mip.MIP_ASSERT(m_soundIndex < 8);
  m_playCommand[1 + m_soundIndex * 2] = sound;
  m_playCommand[1 + m_soundIndex * 2 + 1] = static_cast<uint8_t>(delayTime / 30);
  m_soundIndex++;

  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Sound::playList(uint8_t repeatCount) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Sound->playList()"));

  // Must call beginList() and addEntryToList() before calling this
  // function.
  m_mip.MIP_ASSERT(m_soundIndex >= 1);
  m_playCommand[0] = MIP_CMD_PLAY_SOUND;

  // Fill out the rest of the command buffer with mute sounds.
  while (m_soundIndex < 8) {
    m_playCommand[1 + m_soundIndex * 2] = MIP_SOUND_SHORT_MUTE_FOR_STOP;
    m_playCommand[1 + m_soundIndex * 2 + 1] = 0;
    m_soundIndex++;
  }

  // The last byte in the command is the repeat count.
  m_playCommand[17] = repeatCount;

  // Send this command blindly with no error checking since there is no way to
  // determine if it has failed.
  m_mip.serial.rawSend(m_playCommand, sizeof(m_playCommand));

  // Set the index to 8 to flag that no more items can be added to the sound
  // list but you can still play it again.
  m_soundIndex = 8;
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Sound::play(
  MiPSoundIndex sound, MiPVolume volume /* = MIP_VOLUME_DEFAULT */) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Sound->play()"));

  beginList();
  addEntryToList(sound, 0, volume);
  playList();
}

uint8_t MiP_Sound::readVolume() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Sound->readVolume()"));

  int8_t result = MiP::MIP_ERROR_NONE;

  // Retry the read if it should fail on the first attempt.
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    uint8_t volume = 0;
    result = rawGetVolume(volume);
    if (result == MiP::MIP_ERROR_NONE) {
      m_mip.m_lastError = MiP::MIP_ERROR_NONE;
      return volume;
    }

    // An error was encountered so we will loop around and try again.
    // Wait for a bit before the next retry.
    delay(MiP_Serial::MIP_RETRY_WAIT);
  }

  m_mip.m_lastError = result;
  return 0;
}

void MiP_Sound::writeVolume(uint8_t volume) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Sound->writeVolume()"));

  int8_t result = MiP::MIP_ERROR_NONE;

  // Send the set command and then issue the corresponding get command. Retry if
  // the get fails or doesn't return the expected new setting.
  for (uint8_t retry = 0; retry < MiP_Serial::MIP_MAX_RETRIES; retry++) {
    rawSetVolume(volume);

    // Read back and make sure that it was set as expected.
    uint8_t updatedVolume = 0;
    result = rawGetVolume(updatedVolume);
    if (result == MiP::MIP_ERROR_NONE && updatedVolume == volume) {
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

void MiP_Sound::end() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Sound->end()"));

  writeVolume(MIP_VOLUME_7);
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

// This internal protected method sends the get volume command with minimal
// error handling. The error recovery happens at a higher level of the driver.
int8_t MiP_Sound::rawGetVolume(uint8_t& volume) {
  const uint8_t getVolume[1] = { MIP_CMD_GET_VOLUME };
  uint8_t response[1 + 1];
  size_t responseLength = 0;
  volume = 0;
  int8_t result = m_mip.serial.rawReceive(
    getVolume, sizeof(getVolume), response, sizeof(response), responseLength);
  if (result != MiP::MIP_ERROR_NONE) return result;
  if (responseLength != sizeof(response) || response[0] != MIP_CMD_GET_VOLUME
      || response[1] > 7) {
    return MiP::MIP_ERROR_BAD_RESPONSE;
  }
  volume = response[1];
  return MiP::MIP_ERROR_NONE;
}

// This internal protected method sends the set volume command with no error
// checking. The error handling / recovery happens at a higher level of the
// driver.
void MiP_Sound::rawSetVolume(uint8_t volume) {
  m_mip.MIP_ASSERT(volume <= 7);
  uint8_t command[1 + 1] = { MIP_CMD_SET_VOLUME, volume };
  m_mip.serial.rawSend(command, sizeof(command));
}
