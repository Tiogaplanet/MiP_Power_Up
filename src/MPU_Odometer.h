/**
 * @file MPU_Odometer.h
 * @brief Defines the public interface for odometer tracking in the MiP library.
 *
 * @details This header declares the odometer API used to read and reset
 * distance data.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef MPU_ODOMETER_H
#define MPU_ODOMETER_H

#include <stdint.h>

// Forward-declare the main MiP class to avoid circular include dependencies.
class MiP;

/**
 * @brief Manages MiP's odometer tracking and distance measurement.
 */
class MiP_Odometer {
public:
  /**
   * @brief Reads MiP's total distance travelled.
   *
   * @details Queries MiP's wheel encoder ticks and converts the result to
   * centimeters (at 48.5 ticks per centimeter). Performs a verified read with
   * automatic retries on error.
   *
   * @return float Accumulated distance in centimeters. Returns 0.0f on
   * communication failure.
   */
  float read();

  /**
   * @brief Resets the odometer distance counter back to zero.
   *
   * @details Sends the reset command, then reads the odometer back and verifies
   * that the reported distance is essentially zero. Retries on failure.
   */
  void reset();

  /**
   * @brief Reads MiP's raw left and right wheel encoder tick counts.
   *
   * @details Undocumented command queries individual wheel encoder registers 
   * (command 0x87) over UART. Useful for custom dead-reckoning, differential
   * steering, or turn tracking. Performs a verified read with automatic retries on
   * communication error.
   *
   * @param[out] leftTicks  Receives raw tick count for the left wheel encoder (16-bit).
   * @param[out] rightTicks Receives raw tick count for the right wheel encoder (16-bit).
   */
  void readEncoders(uint16_t& leftTicks, uint16_t& rightTicks);

  /**
   * @brief Undocumented command resets MiP's raw left and right wheel encoder tick
   * counters to zero.
   *
   * @details Sends command 0x88 over UART to zero out individual wheel encoder registers.
   */
  void resetEncoders();

protected:
  /**
   * @brief MiP protocol command byte to read the accumulated wheel tick
   * odometer count.
   */
  static constexpr uint8_t MIP_CMD_READ_ODOMETER = 0x85;

  /**
   * @brief MiP protocol command byte to reset the odometer wheel tick counter
   * to zero.
   */
  static constexpr uint8_t MIP_CMD_RESET_ODOMETER = 0x86;

  /**
   * @brief MiP undocumented protocol command byte to read raw individual wheel encoder ticks.
   */
  static constexpr uint8_t MIP_CMD_READ_RAW_ENCODERS = 0x87;

  /**
   * @brief MiP undocumented protocol command byte to reset raw individual wheel encoder counters to zero.
   */
  static constexpr uint8_t MIP_CMD_RESET_RAW_ENCODERS = 0x88;

  /**
   * @brief Encoder wheel ticks per centimeter ratio (48.5 ticks/cm).
   */
  static constexpr float TICKS_PER_CM = 48.5f;

  /**
   * @brief Tolerance (cm) used when verifying that a reset succeeded.
   * Allows for minor floating-point / timing noise after the reset command.
   */
  static constexpr float RESET_VERIFY_EPSILON_CM = 0.5f;

private:
  /**
   * @brief Private constructor; instantiated strictly by MiP orchestrator.
   *
   * @param mip A reference to the main MiP object to access core communication
   * services.
   */
  explicit MiP_Odometer(MiP& mip);

  /**
   * @brief Low-level send of the reset command (no verification).
   */
  void rawReset();

  /**
   * @brief Sends reset, then reads the odometer and checks that the value is
   * near zero. Returns MIP_ERROR_NONE on success.
   */
  int8_t verifiedReset();

  int8_t rawRead(float& distanceInCm);
  int8_t rawReadEncoders(uint16_t& leftTicks, uint16_t& rightTicks);

  MiP& m_mip;  // Stores a reference to the main MiP class.

  /**
   * @brief Allows MiP to call private constructor.
   */
  friend class MiP;
};

#endif  // MPU_ODOMETER_H
