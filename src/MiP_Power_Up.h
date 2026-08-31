/**
 * @file MiP_Power_Up.h
 * @brief Defines the core MPU API and unified subsystem interfaces.
 *
 * @details This header declares the main MiP class and the public interfaces
 * exposed by the library subsystems. It supports multiple architectures
 * including ESP8266 (D1 mini) and AVR (Pro Mini).
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef MIP_POWER_UP_H
#define MIP_POWER_UP_H

#include <Arduino.h>

// --- Common Core Subsystems ---
#include "MPU_Battery.h"
#include "MPU_ChestLED.h"
#include "MPU_Clap.h"
#include "MPU_Console.h"
#include "MPU_EEPROM.h"
#include "MPU_Gesture.h"
#include "MPU_HeadLEDs.h"
#include "MPU_Infrared.h"
#include "MPU_Mode.h"
#include "MPU_Motion.h"
#include "MPU_Odometer.h"
#include "MPU_Position.h"
#include "MPU_Queue.h"
#include "MPU_Radar.h"
#include "MPU_Serial.h"
#include "MPU_Shake.h"
#include "MPU_Sound.h"
#include "MPU_Version.h"
#include "MPU_Weight.h"

// --- Architecture-Specific Subsystems ---
#if defined(ESP8266) || defined(ESP32)
#include "MPU_WiFi.h"
#include "MPU_Debug.h"
#endif

// MiP Power Up unified library versioning
#define MIP_POWER_UP_VERSION_MAJOR 2
#define MIP_POWER_UP_VERSION_MINOR 1
#define MIP_POWER_UP_VERSION_PATCH 0

// Combined string representation
#define MIP_POWER_UP_VERSION "2.1.0"

// Combined numerical value for preprocessor version checks (2.1.0 -> 21000)
#define MIP_POWER_UP_VERSION_NUMBER \
  (MIP_POWER_UP_VERSION_MAJOR * 10000 + MIP_POWER_UP_VERSION_MINOR * 100 + MIP_POWER_UP_VERSION_PATCH)

// Setup some debug levels for reporting library status via mip.console.
#define MIP_DEBUG_NONE 0
#define MIP_DEBUG_ERROR 1
#define MIP_DEBUG_WARN 2
#define MIP_DEBUG_INFO 3

// Default to NONE if not defined by the user in the sketch.
#ifndef MIP_DEBUG_LEVEL
#define MIP_DEBUG_LEVEL MIP_DEBUG_NONE
#endif

// ---------------------------------------------------------------------------
// Unified Debug Macros
// Thanks to MiP_Console, these route automatically to the correct Serial port
// on all architectures without #ifdef clutter!
// ---------------------------------------------------------------------------

#if MIP_DEBUG_LEVEL >= MIP_DEBUG_ERROR
#define MIP_DEBUG_ERROR_PREFIX() m_mip.console.print(F("[ERROR] "))
#define MIP_DEBUG_ERROR_PRINT(...) m_mip.console.print(__VA_ARGS__)
#define MIP_DEBUG_ERROR_PRINTLN(...) m_mip.console.println(__VA_ARGS__)
#else
#define MIP_DEBUG_ERROR_PREFIX()
#define MIP_DEBUG_ERROR_PRINT(...)
#define MIP_DEBUG_ERROR_PRINTLN(...)
#endif

#if MIP_DEBUG_LEVEL >= MIP_DEBUG_WARN
#define MIP_DEBUG_WARN_PREFIX() m_mip.console.print(F("[WARN] "))
#define MIP_DEBUG_WARN_PRINT(...) m_mip.console.print(__VA_ARGS__)
#define MIP_DEBUG_WARN_PRINTLN(...) m_mip.console.println(__VA_ARGS__)
#else
#define MIP_DEBUG_WARN_PREFIX()
#define MIP_DEBUG_WARN_PRINT(...)
#define MIP_DEBUG_WARN_PRINTLN(...)
#endif

#if MIP_DEBUG_LEVEL >= MIP_DEBUG_INFO
#define MIP_DEBUG_INFO_PREFIX() m_mip.console.print(F("[INFO] "))
#define MIP_DEBUG_INFO_PRINT(...) m_mip.console.print(__VA_ARGS__)
#define MIP_DEBUG_INFO_PRINTLN(...) m_mip.console.println(__VA_ARGS__)
#else
#define MIP_DEBUG_INFO_PREFIX()
#define MIP_DEBUG_INFO_PRINT(...)
#define MIP_DEBUG_INFO_PRINTLN(...)
#endif

// Define an assert mechanism that can be used to log and halt when the API is
// called incorrectly.
#define MIP_ASSERT(EXPRESSION) m_mip.mipAssert((EXPRESSION), __LINE__, __FILE__)

/**
 * @brief MiP's current stance position and battery voltage.
 */
class MiPStatus {
public:
  MiPStatus() {
    clear();
  }

  void clear() {
    battery = 0.0f;
    position = MIP_POSITION_ON_BACK_WITH_KICKSTAND;
  }

  float battery;         ///< Cached battery voltage in Volts (4.0V - 6.4V).
  MiPPosition position;  ///< Cached physical orientation stance.
};

/**
 * @mainpage MiP Power Up Library
 *
 * This library provides a complete interface to control WowWee MiP over UART
 * from ESP8266 (D1 mini), ESP32-S2, and AVR (Pro Mini) platforms.
 */
class MiP {
public:
  static constexpr uint8_t MIP_ERROR_NONE = 0;      ///< Operation succeeded.
  static constexpr uint8_t MIP_ERROR_TIMEOUT = 1;   ///< Timed out waiting for
                                                    ///< response.
  static constexpr uint8_t MIP_ERROR_NO_EVENT = 2;  ///< No event has arrived
                                                    ///< yet.
  static constexpr uint8_t MIP_ERROR_BAD_RESPONSE = 3;  ///< Unexpected response
                                                        ///< received.
  static constexpr uint8_t MIP_ERROR_MAX_RETRIES = 4;   ///< Exceeded maximum
                                                        ///< retries.

  // --- Core Lifecycle Functions ---
  MiP();
  ~MiP();

  bool begin();
  void end();
  void sleep();
  bool isInitialized() const;

  int8_t lastCallResult() const;
  bool didLastCallFail() const;
  void printLastCallResult();
  uint32_t getBaudRate() const;

  // --- Subsystem Member Instances ---
  MiP_Battery battery;
  MiP_ChestLED chestLED;
  MiP_Clap clap;
  MiP_Console console;
  MiP_EEPROM eeprom;
  MiP_Gesture gesture;
  MiP_HeadLEDs headLEDs;
  MiP_Infrared infrared;
  MiP_Mode mode;
  MiP_Motion motion;
  MiP_Odometer odometer;
  MiP_Position position;
  MiP_Radar radar;
  MiP_Serial serial;
  MiP_Shake shake;
  MiP_Sound sound;
  MiP_Version version;
  MiP_Weight weight;

#if defined(ESP8266) || defined(ESP32)
  MiP_WiFi wifi;
#endif

protected:
  static constexpr uint8_t MIP_CMD_DISCONNECT_APP = 0xFE;  ///< Disconnect
                                                           ///< command byte.
  static constexpr uint8_t MIP_CMD_SLEEP = 0xFA;       ///< Sleep command byte.
  static constexpr uint8_t MIP_CMD_GET_STATUS = 0x79;  ///< Status query command
                                                       ///< byte.

  static constexpr uint8_t MIP_MAX_BEGIN_RETRIES = 5;    ///< Max retries in
                                                         ///< begin().
  static constexpr uint16_t MIP_BEGIN_RETRY_WAIT = 500;  ///< Delay between
                                                         ///< retries in begin()
                                                         ///< (ms).
  static constexpr uint32_t MIP_FAST_BAUD_RATE = 115200;  ///< High-speed UART
                                                          ///< link rate.
  static constexpr uint32_t MIP_SLOW_BAUD_RATE = 9600;  ///< Low-speed UART link
                                                        ///< rate.

#if defined(ESP8266)
  static constexpr uint32_t ESP8266_DEBUG_BAUD_RATE = 74880;  ///< Default
                                                              ///< ESP8266
                                                              ///< bootloader
                                                              ///< debug rate.
#elif defined(__AVR__)
  static constexpr uint8_t UART_SELECT_PIN = 2;  ///< Pin for Pro Mini UART
                                                 ///< multiplexer.
#endif

  void clear();
  int8_t attemptMiPConnection(uint32_t baudRate);

  // --- Hardware UART Multiplexer Methods (AVR Only) ---
#if defined(__AVR__)
  bool isSerialGoingToMiP() const {
    return m_serialGoingToMiP;
  }
  void switchSerialToMiP();
  void switchSerialToPC();
  bool m_serialGoingToMiP;
#endif

  void dispatchEvent(uint8_t command, const uint8_t* payload, size_t length);
  void mipAssert(bool condition, uint32_t lineNumber, const char* fileName);
  int8_t rawGetStatus(MiPStatus& status);
  int8_t parseStatus(MiPStatus& status, const uint8_t response[], size_t responseLength);

  friend class MiP_Battery;
  friend class MiP_ChestLED;
  friend class MiP_Clap;
  friend class MiP_Console;
  friend class MiP_EEPROM;
  friend class MiP_Gesture;
  friend class MiP_HeadLEDs;
  friend class MiP_Infrared;
  friend class MiP_Mode;
  friend class MiP_Motion;
  friend class MiP_Odometer;
  friend class MiP_Position;
  friend class MiP_Radar;
  friend class MiP_Serial;
  friend class MiP_Shake;
  friend class MiP_Sound;
  friend class MiP_Version;
  friend class MiP_Weight;

#if defined(ESP8266) || defined(ESP32)
  friend class MiP_WiFi;
#endif

  // Bits that can be set in m_flags bitfield.
  enum FlagBits : uint8_t {
    MIP_FLAG_RADAR_VALID = (1 << 0),
    MIP_FLAG_SHAKE_DETECTED = (1 << 1),
    MIP_FLAG_WEIGHT_VALID = (1 << 2),
    MIP_FLAG_INITIALIZED = (1 << 3)
  };

  MiP& m_mip;  ///< Self-reference to streamline debug macro usage.
  uint32_t m_baudRate;
  uint8_t m_flags;
  int8_t m_lastError;
  MiPStatus m_lastStatus;
};

#endif  // MIP_POWER_UP_H
