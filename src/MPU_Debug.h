/**
 * @file MPU_Debug.h
 * @brief Defines the debug logging and telnet remote console interface for the
 * MiP library.
 *
 * @details This header declares the MiP_Debug class, severity constants, and
 * global helper macros (`mDebug`, `mDebugI`, `mDebugE`, etc.) used for
 * multi-level logging, execution profiling, text filtering, and wireless telnet
 * debugging on ESP8266/ESP32 controllers.
 *
 * @author Joao Lopes (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the MIT License
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * https://opensource.org/licenses/MIT
 */
#ifndef MPU_DEBUG_H
#define MPU_DEBUG_H

#if defined(ESP8266) || defined(ESP32)

#include <ESP8266WiFi.h>
#include "Arduino.h"
#include "Print.h"

extern "C" {
  /**
   * @brief ESP8266 SDK low-level function to adjust system CPU clock frequency.
   *
   * @details Adjusts the ESP8266 CPU clock frequency dynamically at runtime.
   *
   * @param freq Target CPU clock frequency in MHz (typically 80 or 160).
   * @return **true** if the clock frequency change succeeded; **false**
   * otherwise.
   */
  bool system_update_cpu_freq(uint8 freq);
}

/**
 * @name Global Logging Helper Macros
 * @{
 *
 * @details Convenience macros that evaluate active logging severity thresholds
 * before formatting and emitting output over active debug channels (Telnet
 * and/or Serial).
 */

#define mDebug(...) \
  { \
    if (debug.isActive(debug.ANY)) debug.printf(__VA_ARGS__); \
  }

#define mDebugP(...) \
  { \
    if (debug.isActive(debug.PROFILER)) debug.printf(__VA_ARGS__); \
  }

#define mDebugV(...) \
  { \
    if (debug.isActive(debug.VERBOSE)) debug.printf(__VA_ARGS__); \
  }

#define mDebugD(...) \
  { \
    if (debug.isActive(debug.DEBUG)) debug.printf(__VA_ARGS__); \
  }

#define mDebugI(...) \
  { \
    if (debug.isActive(debug.INFO)) debug.printf(__VA_ARGS__); \
  }

#define mDebugW(...) \
  { \
    if (debug.isActive(debug.WARNING)) debug.printf(__VA_ARGS__); \
  }

#define mDebugE(...) \
  { \
    if (debug.isActive(debug.ERROR)) debug.printf(__VA_ARGS__); \
  }

/** @} */

/**
 * @brief Telnet and Serial debug server for real-time remote diagnostics.
 */
class MiP_Debug : public Print {
public:
  // --- System Configuration Constants ---
  static constexpr uint16_t TELNET_PORT = 23;  ///< Default TCP telnet port
                                               ///< (23).

  /**
   * @name Logging Severity Levels
   * @{
   */
  static constexpr uint8_t PROFILER = 0;  ///< Execution timing section
                                          ///< profiling.
  static constexpr uint8_t VERBOSE = 1;   ///< Detailed verbose messages.
  static constexpr uint8_t DEBUG = 2;     ///< Standard debug messages.
  static constexpr uint8_t INFO = 3;      ///< Informational status messages.
  static constexpr uint8_t WARNING = 4;   ///< Warning messages.
  static constexpr uint8_t ERROR = 5;     ///< Critical error messages.
  static constexpr uint8_t ANY = 6;       ///< Messages output unconditionally.
  /** @} */

  void begin(const String& hostname, uint8_t startingDebugLevel = VERBOSE);
  void stop();
  void handle();
  void setSerialEnabled(bool enable);
  void setResetCmdEnabled(bool enable);
  void setHelpProjectsCmds(const String& help);
  void setCallBackProjectCmds(void (*callback)());
  String getLastCommand() const;
  void clearLastCommand();
  void showTime(bool show);
  void showProfiler(bool show, uint32_t minTime = 0);
  void showDebugLevel(bool show);
  void showColors(bool show);
  void autoProfilerLevel(uint32_t millisElapsed);
  void setFilter(const String& filter);
  void setNoFilter();
  bool isActive(uint8_t debugLevel = DEBUG);

  virtual size_t write(uint8_t byte) override;
  virtual size_t write(const uint8_t* buffer, size_t size) override;

  String expand(const String& string);

protected:
  // --- System Configuration Constants ---
  static constexpr uint32_t MAX_TIME_INACTIVE = 3600000;  ///< Inactivity
                                                          ///< disconnect
                                                          ///< timeout (1 hour).
  static constexpr size_t BUFFER_PRINT = 150;  ///< Print character buffer size.

#ifdef CLIENT_BUFFERING
  static constexpr uint32_t DELAY_TO_SEND = 10;  ///< Packet send delay in ms.
  static constexpr size_t MAX_SIZE_SEND = 1460;  ///< TCP MSS limit.
#endif

  // --- ANSI Terminal Escape Codes ---
  static constexpr const char* COLOR_RESET = "\x1B[0m";
  static constexpr const char* COLOR_BLACK = "\x1B[0;30m";
  static constexpr const char* COLOR_RED = "\x1B[0;31m";
  static constexpr const char* COLOR_GREEN = "\x1B[0;32m";
  static constexpr const char* COLOR_YELLOW = "\x1B[0;33m";
  static constexpr const char* COLOR_BLUE = "\x1B[0;34m";
  static constexpr const char* COLOR_MAGENTA = "\x1B[0;35m";
  static constexpr const char* COLOR_CYAN = "\x1B[0;36m";
  static constexpr const char* COLOR_WHITE = "\x1B[0;37m";
  static constexpr const char* COLOR_BACKGROUND_RED = "\x1B[41m";
  static constexpr const char* COLOR_BACKGROUND_GREEN = "\x1B[42m";
  static constexpr const char* COLOR_BACKGROUND_YELLOW = "\x1B[43m";
  static constexpr const char* COLOR_BACKGROUND_MAGENTA = "\x1B[45m";
  static constexpr const char* COLOR_BACKGROUND_CYAN = "\x1B[46m";
  static constexpr const char* COLOR_BACKGROUND_WHITE = "\x1B[47m";

private:
  void showHelp();
  void processCommand();
  String formatNumber(uint32_t value, uint8_t size, char insert = '0');
  bool isCRLF(char character);

  String m_hostname = "";
  bool m_connected = false;
  uint8_t m_clientDebugLevel = DEBUG;
  uint8_t m_lastDebugLevel = DEBUG;
  uint32_t m_lastTimePrint = millis();
  uint8_t m_levelBeforeProfiler = DEBUG;
  uint32_t m_levelProfilerDisable = 0;
  uint32_t m_autoLevelProfiler = 0;
  bool m_showTime = false;
  bool m_showProfiler = false;
  uint32_t m_minTimeShowProfiler = 0;
  bool m_showDebugLevel = true;
  bool m_showColors = false;
  bool m_serialEnabled = false;
  bool m_resetCommandEnabled = false;
  bool m_newLine = true;
  String m_command = "";
  String m_lastCommand = "";
  uint32_t m_lastTimeCommand = millis();
  String m_helpProjectCmds = "";
  void (*m_callbackProjectCmds)() = nullptr;
  String m_filter = "";
  bool m_filterActive = false;
  String m_bufferPrint = "";

#ifdef CLIENT_BUFFERING
  String m_bufferSend = "";
  uint16_t m_sizeBufferSend = 0;
  uint32_t m_lastTimeSend = 0;
#endif
};

#endif  // defined(ESP8266) || defined(ESP32)
#endif  // MPU_DEBUG_H
