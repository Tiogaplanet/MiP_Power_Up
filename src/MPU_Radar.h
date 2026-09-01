/**
 * @file MPU_Radar.h
 * @brief Defines the public interface for radar tracking in the MiP library.
 *
 * @details This header declares the radar API used to enable and read radar
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
#ifndef MPU_RADAR_H
#define MPU_RADAR_H

#include <stdint.h>

// Forward-declare the main MiP class to avoid circular include dependencies.
class MiP;

/**
 * @brief Proximity distance range intervals reported by MiP's front IR sensors.
 */
enum MiPRadar : uint8_t {
  MIP_RADAR_NONE = 0x01,  ///< No obstacle detected within radar tracking range.
  MIP_RADAR_10CM_30CM = 0x02,  ///< Obstacle detected between 10cm and 30cm in
                               ///< front of MiP.
  MIP_RADAR_0CM_10CM = 0x03,   ///< Obstacle detected between 0cm and 10cm in
                               ///< front of MiP.
  MIP_RADAR_INVALID = 0xFF     ///< Initialized default state prior to receiving
                               ///< any radar event.
};

/**
 * @brief Gesture or Radar operating mode states.
 */
enum MiPRadarMode : uint8_t {
  MIP_RADAR_DISABLED = 0x00,  ///< Both radar tracking and gesture detection
                              ///< modes are disabled.
  MIP_RADAR = 0x04            ///< Radar proximity tracking mode is active.
};

/**
 * @brief Manages MiP's radar proximity tracking system.
 */
class MiP_Radar {
public:
  /**
   * @brief Resets cached radar tracking data back to MIP_RADAR_INVALID.
   */
  void clear();

  /**
   * @brief Enables radar tracking mode on MiP.
   *
   * @details Uses verified mode switching (sends mode command + read-back
   * confirmation with automatic retries on failure).
   */
  void enable();

  /**
   * @brief Disables radar tracking mode.
   *
   * @details Uses verified mode switching (sends disable command + read-back
   * confirmation with automatic retries on failure).
   */
  void disable();

  /**
   * @brief Checks whether radar tracking mode is currently active on MiP.
   *
   * @return true if radar mode is enabled (mode equals MIP_RADAR), false
   * otherwise.
   */
  bool isEnabled();

  /**
   * @brief Reads the most recent radar tracking distance data.
   *
   * @details Uses cached value from the latest Out-Of-Band status event.
   * Processes pending serial data first.
   *
   * @return MiPRadar Current radar proximity range, or MIP_RADAR_INVALID if no
   * data received yet.
   */
  MiPRadar read();

protected:
  /**
   * @brief MiP protocol command byte to query the current gesture/radar
   * operating mode.
   */
  static constexpr uint8_t MIP_CMD_GET_GESTURE_RADAR_MODE = 0x0D;

  /**
   * @brief MiP protocol command byte to configure the gesture/radar operating
   * mode.
   */
  static constexpr uint8_t MIP_CMD_SET_GESTURE_RADAR_MODE = 0x0C;

  /**
   * @brief MiP protocol notification byte received when a radar distance update
   * arrives.
   */
  static constexpr uint8_t MIP_CMD_GET_RADAR_RESPONSE = 0x0C;

private:
  /**
   * @brief Private constructor; instantiated strictly by MiP orchestrator.
   *
   * @param mip A reference to the main MiP object to access core services.
   */
  explicit MiP_Radar(MiP& mip);

  void verifiedSet(MiPRadarMode desiredMode);
  bool check(MiPRadarMode expectedMode);
  int8_t rawGet(MiPRadarMode& mode);
  void rawSet(MiPRadarMode mode);

  /**
   * @brief Handles an incoming radar OOB event from the transport layer.
   *
   * @details Called by MiP::dispatchEvent() when a MIP_CMD_GET_RADAR_RESPONSE
   * notification is received. Updates the cached distance and marks the radar
   * data as valid.
   *
   * @param radarCode Raw distance code from MiP (MIP_RADAR_NONE ..
   * MIP_RADAR_0CM_10CM).
   */
  void processEvent(uint8_t radarCode);

  MiP& m_mip;  // Stores a reference to the main MiP class.
  MiPRadar m_lastRadar;

  /**
   * @brief Allows MiP and transport components to access private constructor
   * and protected protocol bytes.
   */
  friend class MiP;
  friend class MiP_Serial;
};

#endif  // MPU_RADAR_H
