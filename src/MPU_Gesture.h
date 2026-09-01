/**
 * @file MPU_Gesture.h
 * @brief Defines the public interface for gesture handling in the MiP library.
 *
 * @details This header declares the gesture API used to enable and read gesture
 * events.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef MPU_GESTURE_H
#define MPU_GESTURE_H

#include <stdint.h>

#include "MPU_Queue.h"

// Forward-declare the main MiP class to avoid circular include dependencies.
class MiP;

/**
 * @brief Gesture or Radar operating mode states.
 */
enum MiPGestureMode : uint8_t {
  MIP_GESTURE_RADAR_DISABLED = 0x00,  ///< Both gesture detection and radar
                                      ///< tracking modes are disabled.
  MIP_GESTURE = 0x02                  ///< Gesture detection mode is active.
};

/**
 * @brief Recognized hand gesture motion directions detected by MiP's front IR
 * sensors.
 */
enum MiPGesture : uint8_t {
  MIP_GESTURE_LEFT = 0x0A,  ///< Hand swiped from right to left in front of MiP.
  MIP_GESTURE_RIGHT = 0x0B,  ///< Hand swiped from left to right in front of
                             ///< MiP.
  MIP_GESTURE_CENTER_SWEEP_LEFT = 0x0C,   ///< Center sweep gesture toward the
                                          ///< left.
  MIP_GESTURE_CENTER_SWEEP_RIGHT = 0x0D,  ///< Center sweep gesture toward the
                                          ///< right.
  MIP_GESTURE_CENTER_HOLD = 0x0E,  ///< Hand held steady in front of MiP's
                                   ///< center IR sensor.
  MIP_GESTURE_FORWARD = 0x0F,      ///< Hand moved closer toward MiP (forward
                                   ///< gesture).
  MIP_GESTURE_BACKWARD = 0x10,     ///< Hand pulled away from MiP (backward
                                   ///< gesture).
  MIP_GESTURE_INVALID = 0xFF  ///< Value returned when no valid gesture event is
                              ///< available.
};

/**
 * @brief Manages MiP's gesture detection subsystem and event queue.
 */
class MiP_Gesture {
public:
  /**
   * @brief Enables gesture detection mode on MiP.
   *
   * @details Uses verified mode switching (sends mode command + read-back
   * confirmation with automatic retry on failure).
   */
  void enable();

  /**
   * @brief Disables gesture detection mode.
   *
   * @details Uses verified mode switching (sends disable command + read-back
   * confirmation with automatic retry on failure).
   */
  void disable();

  /**
   * @brief Checks whether gesture detection mode is currently active on MiP.
   *
   * @return true if gesture mode is enabled (mode equals MIP_GESTURE), false
   * otherwise.
   */
  bool isEnabled();

  /**
   * @brief Returns the number of unread gesture events currently in the queue.
   *
   * @details Processes any pending serial data first to ensure the internal
   * queue is up to date.
   *
   * @return uint8_t Number of available gesture events in the queue.
   */
  uint8_t availableEvents();

  /**
   * @brief Reads the next available gesture event from the queue.
   *
   * @details Processes pending serial data first. Pops the oldest gesture event
   * from the queue. Returns MIP_GESTURE_INVALID and sets last error to
   * MIP_ERROR_NO_EVENT if the queue is empty.
   *
   * @return MiPGesture The gesture event direction code, or MIP_GESTURE_INVALID
   * if none available.
   */
  MiPGesture readEvent();

  /**
   * @brief Checks whether both gesture detection and radar tracking modes are
   * disabled.
   *
   * @return true if both modes are off (in MIP_GESTURE_RADAR_DISABLED state),
   * false otherwise.
   */
  bool areGestureAndRadarModesDisabled();

protected:
  /**
   * @brief MiP protocol command byte to query current gesture/radar operating
   * mode.
   */
  static constexpr uint8_t MIP_CMD_GET_GESTURE_RADAR_MODE = 0x0D;

  /**
   * @brief MiP protocol command byte to configure gesture/radar operating mode.
   */
  static constexpr uint8_t MIP_CMD_SET_GESTURE_RADAR_MODE = 0x0C;

  /**
   * @brief MiP protocol notification byte received when a gesture is
   * recognized.
   */
  static constexpr uint8_t MIP_CMD_GET_GESTURE_RESPONSE = 0x0A;

  void clear();

private:
  /**
   * @brief Private constructor; instantiated strictly by MiP orchestrator.
   *
   * @param mip Reference to the main MiP object to access core communication
   * services.
   */
  explicit MiP_Gesture(MiP& mip);

  // Helper utilities for sub-functions
  void verifiedSet(MiPGestureMode desiredMode);
  bool check(MiPGestureMode expectedMode);
  void rawSet(MiPGestureMode mode);
  int8_t rawGet(MiPGestureMode& mode);

  /**
   * @brief Handles an incoming gesture OOB event notification from the
   * transport layer.
   *
   * @param gestureCode Raw gesture direction byte received from MiP.
   */
  void processEvent(uint8_t gestureCode);

  MiP& m_mip;  // Stores a reference to the main MiP class.
  mip_detail::CircularQueue<MiPGesture, 8> m_gestureEvents;

  /**
   * @brief Allows MiP and transport components to access protected protocol
   * bytes.
   */
  friend class MiP;
  friend class MiP_Serial;
};

#endif  // MPU_GESTURE_H
