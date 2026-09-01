/**
 * @file MPU_Radar.h
 * @brief Defines the public interface for radar distance sensing in the MiP library.
 *
 * @details This header declares the API used to manage MiP's IR proximity radar.
 * It provides methods to enable/disable continuous background polling, read cached
 * radar events, or execute a one-shot radar ping.
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
 * @brief Categorical distance ranges returned by MiP's IR radar sensor.
 */
enum MiPRadar : uint8_t {
  MIP_RADAR_NONE = 0x01,         ///< No object detected within range.
  MIP_RADAR_10CM_30CM = 0x02,    ///< Object detected between 10cm and 30cm (Distant).
  MIP_RADAR_0CM_10CM = 0x03,     ///< Object detected between 0cm and 10cm (Near).
  MIP_RADAR_INVALID = 0xFF       ///< Sentinel value indicating an invalid or empty reading.
};

/**
 * @brief Manages MiP's IR proximity radar sensor.
 */
class MiP_Radar {
 public:
  /**
   * @brief Enables continuous radar proximity detection.
   *
   * @details When enabled, MiP will actively scan for objects and send Out-Of-Band
   * (OOB) notifications over UART whenever the distance category changes.
   */
  void enable();

  /**
   * @brief Disables continuous radar proximity detection.
   *
   * @details Stops MiP from scanning and sending background OOB radar notifications.
   */
  void disable();

  /**
   * @brief Checks if continuous radar detection is currently enabled.
   *
   * @return true if enabled, false otherwise.
   */
  bool isEnabled();

  /**
   * @brief Reads the last cached radar event from the background queue.
   *
   * @details Returns the most recent radar detection event received from MiP.
   * This relies on enable() having been called previously. Once read, the 
   * cached event is cleared until a new one arrives.
   *
   * @return MiPRadar The detected distance category, or MIP_RADAR_INVALID if no new event exists.
   */
  MiPRadar read();

  /**
   * @brief Performs a one-shot radar ping to get the current distance instantly.
   *
   * @details Undocumented command (0x0B). Forces an immediate IR proximity query 
   * and blocks until MiP returns the current distance category. This allows you 
   * to check the radar distance without enabling continuous background polling.
   *
   * @return MiPRadar The detected distance category, or MIP_RADAR_INVALID on communication failure.
   */
  MiPRadar ping();

  /**
   * @brief Clears the internal radar event cache.
   */
  void clear();

 protected:
  /**
   * @brief MiP protocol command byte to configure continuous radar mode.
   */
  static constexpr uint8_t MIP_CMD_SET_RADAR_MODE = 0x0C;

  /**
   * @brief MiP protocol command byte received during an OOB radar event.
   */
  static constexpr uint8_t MIP_CMD_GET_RADAR_RESPONSE = 0x04;

  /**
   * @brief MiP protocol command byte to force a single, one-shot radar ping.
   */
  static constexpr uint8_t MIP_CMD_RADAR_PING = 0x0B;

 private:
  /**
   * @brief Private constructor; instantiated strictly by MiP orchestrator.
   *
   * @param mip Reference to the main MiP object for core communication services.
   */
  explicit MiP_Radar(MiP& mip);

  /**
   * @brief Handles an incoming radar event notification from the transport layer.
   *
   * @param radarValue Raw radar category byte reported by MiP.
   */
  void processEvent(uint8_t radarValue);

  MiP& m_mip;              ///< Stores a reference to the main MiP class.
  MiPRadar m_currentRadar; ///< The most recently cached radar event.
  bool m_isEnabled;        ///< Tracks whether continuous radar mode is active.

  friend class MiP;
  friend class MiP_Serial;
};

#endif  // MPU_RADAR_H
