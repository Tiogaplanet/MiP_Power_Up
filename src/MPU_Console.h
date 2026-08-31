/**
 * @file MPU_Console.h
 * @brief Defines the public interface for the PC serial console in the MiP
 * library.
 *
 * @details This header declares the console API used to print diagnostic and
 * status messages to the PC. It abstracts the underlying serial hardware across
 * different architectures (AVR, ESP8266, ESP32).
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef MPU_CONSOLE_H
#define MPU_CONSOLE_H

#include <Arduino.h>
#include "Print.h"

// Forward-declare the main MiP class to avoid circular include dependencies.
class MiP;

/**
 * @brief Manages serial output to the PC console across multiple architectures.
 *
 * @details Inherits from the Arduino Print class, providing access to standard
 * print(), println(), and formatting functions. Automatically handles hardware
 * UART multiplexing on AVR targets and routes to appropriate debug serial ports
 * on ESP8266/ESP32 targets.
 */
class MiP_Console : public Print {
public:
  /**
   * @brief Writes a single byte to the PC console.
   *
   * @param byte The byte to send to the console.
   * @return size_t Number of bytes written (1 on success).
   */
  virtual size_t write(uint8_t byte) override;

  /**
   * @brief Writes a buffer of bytes to the PC console.
   *
   * @param buffer Pointer to the array of bytes to send.
   * @param size Number of bytes to send.
   * @return size_t Number of bytes successfully written.
   */
  virtual size_t write(const uint8_t* buffer, size_t size) override;

protected:
#if defined(__AVR__)
  /**
   * @brief Digital I/O pin used to control the Pro Mini UART hardware
   * multiplexer.
   */
  static constexpr uint8_t UART_SELECT_PIN = 4;

  /**
   * @brief Pin state to route UART to MiP.
   */
  static constexpr uint8_t UART_SELECT_MIP = LOW;

  /**
   * @brief Pin state to route UART to the PC console.
   */
  static constexpr uint8_t UART_SELECT_PC = HIGH;

  void switchSerialToMiP();
  void switchSerialToPC();
#endif

private:
  /**
   * @brief Private constructor; instantiated strictly by MiP orchestrator.
   *
   * @param mip A reference to the main MiP object.
   */
  explicit MiP_Console(MiP& mip);

  MiP& m_mip;  // Stores a reference to the main MiP class.

  /**
   * @brief Allows MiP to call private constructor.
   */
  friend class MiP;
};

#endif  // MPU_CONSOLE_H
