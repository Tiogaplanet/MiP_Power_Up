/**
 * @file MPU_Motion.cpp
 * @brief Implements motion control for the MiP library.
 *
 * @details This source file implements motion commands and movement helpers.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "MPU_Motion.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_Motion::MiP_Motion(MiP& mip) : m_mip(mip) {
  m_lastContinuousDriveTime = millis() - MIP_CONTINUOUS_DRIVE_DELAY;
}

void MiP_Motion::continuousDrive(int8_t velocity, int8_t turnRate) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->continuousDrive()"));

  uint8_t command[1 + 2];

  m_mip.MIP_ASSERT(velocity >= -32 && velocity <= 32);
  m_mip.MIP_ASSERT(turnRate >= -32 && turnRate <= 32);

  // Ignore requests if they come in too fast so that it can be done in a
  // tight loop but not overload MiP.
  if (millis() - m_lastContinuousDriveTime < MIP_CONTINUOUS_DRIVE_DELAY) {
    m_mip.m_lastError = MiP::MIP_ERROR_NONE;
    return;
  }
  m_lastContinuousDriveTime = millis();

  command[0] = MIP_CMD_CONTINUOUS_DRIVE;
  command[1] = (velocity == 0) ? 0x00 : ((velocity < 0) ? (0x20 + (-velocity)) : velocity);
  command[2] =
    (turnRate == 0) ? 0x00 : ((turnRate < 0) ? (0x60 + (-turnRate)) : (0x40 + turnRate));

  // Send this command blindly with no error checking since there is no way to
  // determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::distanceDrive(MiPDriveDirection driveDirection, uint8_t cm,
                               MiPTurnDirection turnDirection, uint16_t degrees) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->distanceDrive()"));

  uint8_t command[1 + 5];
  m_mip.MIP_ASSERT(degrees <= 360);

  command[0] = MIP_CMD_DISTANCE_DRIVE;
  command[1] = driveDirection;
  command[2] = cm;
  command[3] = turnDirection;
  command[4] = static_cast<uint8_t>(degrees >> 8);
  command[5] = static_cast<uint8_t>(degrees & 0xFF);

  // Send this command blindly with no error checking since there is no way to
  // determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::driveForward(uint8_t speed, uint16_t time) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->driveForward()"));

  // The time parameter is in units of 7 milliseconds.
  uint8_t command[1 + 2];

  m_mip.MIP_ASSERT(speed <= 30);
  m_mip.MIP_ASSERT(time <= 255 * 7);

  if (time > 255 * 7) { time = 255 * 7; }

  command[0] = MIP_CMD_DRIVE_FORWARD;
  command[1] = speed;
  command[2] = static_cast<uint8_t>(time / 7);

  // Send this command blindly with no error checking since there is no way to
  // determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::driveBackward(uint8_t speed, uint16_t time) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->driveBackward()"));

  // The time parameter is in units of 7 milliseconds.
  uint8_t command[1 + 2];

  m_mip.MIP_ASSERT(speed <= 30);
  m_mip.MIP_ASSERT(time <= 255 * 7);

  if (time > 255 * 7) { time = 255 * 7; }

  command[0] = MIP_CMD_DRIVE_BACKWARD;
  command[1] = speed;
  command[2] = static_cast<uint8_t>(time / 7);

  // Send this command blindly with no error checking since there is no way to
  // determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::turnLeft(uint16_t degrees, uint8_t speed) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->turnLeft()"));

  // The turn command is in units of 5 degrees.
  m_mip.MIP_ASSERT(degrees <= 255 * 5);
  m_mip.MIP_ASSERT(speed <= 24);

  if (degrees > 255 * 5) { degrees = 255 * 5; }

  uint8_t angle = static_cast<uint8_t>(degrees / 5);
  uint8_t command[1 + 2];

  command[0] = MIP_CMD_TURN_LEFT;
  command[1] = angle;
  command[2] = speed;

  // Send this command blindly with no error checking since there is no way to
  // determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::turnRight(uint16_t degrees, uint8_t speed) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->turnRight()"));

  // The turn command is in units of 5 degrees.
  m_mip.MIP_ASSERT(degrees <= 255 * 5);
  m_mip.MIP_ASSERT(speed <= 24);

  if (degrees > 255 * 5) { degrees = 255 * 5; }

  uint8_t angle = static_cast<uint8_t>(degrees / 5);
  uint8_t command[1 + 2];

  command[0] = MIP_CMD_TURN_RIGHT;
  command[1] = angle;
  command[2] = speed;

  // Send this command blindly with no error checking since there is no way to
  // determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::stop() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->stop()"));

  uint8_t command[1] = { MIP_CMD_STOP };

  // Send this command blindly with no error checking since there is no way to
  // determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::fallForward() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->fallForward()"));

  fallDown(MIP_FALL_FACE_DOWN);
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::fallBackward() {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->fallBackward()"));

  fallDown(MIP_FALL_ON_BACK);
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

void MiP_Motion::getUp(MiPGetUp getup /* = MIP_GETUP_FROM_EITHER */) {
  MIP_DEBUG_INFO_PREFIX();
  MIP_DEBUG_INFO_PRINTLN(F("MiP->Motion->getUp()"));

  uint8_t command[1 + 1];
  command[0] = MIP_CMD_GET_UP;
  command[1] = static_cast<uint8_t>(getup);

  // Send this command blindly with no error checking since there is no easy
  // way to determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
  m_mip.m_lastError = MiP::MIP_ERROR_NONE;
}

// ==========================================================================
// Protected / Private functions.
// ==========================================================================

// This internal protected method sends the desired set position command to
// fall forward or backward.
void MiP_Motion::fallDown(MiPFallDirection direction) {
  uint8_t command[1 + 1];
  command[0] = MIP_CMD_SET_POSITION;
  command[1] = static_cast<uint8_t>(direction);

  // Send this command blindly with no error checking since there is no easy
  // way to determine if it has failed.
  m_mip.serial.rawSend(command, sizeof(command));
}
