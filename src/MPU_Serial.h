/**
 * @file MPU_Serial.h
 * @brief Unified serial transport interface for MiP across ESP8266, ESP32, and
 * AVR Pro Mini.
 *
 * @details This header declares the low-level serial API used for communicating
 * with MiP. It handles raw binary transmission, decodes hex-ASCII response
 * strings, and demultiplexes out-of-band (OOB) events.
 *
 * @author Adam Green (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef MPU_SERIAL_H
#define MPU_SERIAL_H

#include <Arduino.h>

// Forward-declare the main MiP class to avoid circular include dependencies.
class MiP;

/**
 * @brief Manages low-level UART transport and OOB event demuxing for MiP.
 */
class MiP_Serial {
public:
  /** @brief Maximum number of retry attempts for verified command operations.
   */
  static constexpr uint8_t MIP_MAX_RETRIES = 2;

  /** @brief Delay in milliseconds to pause between command retries. */
  static constexpr uint16_t MIP_RETRY_WAIT = 50;

  /**
   * @brief Sends a raw command to MiP without waiting for a response
   * (fire-and-forget).
   *
   * @param request       Buffer containing the raw byte array command to send
   * to MiP.
   * @param requestLength Number of bytes in the request buffer.
   */
  void rawSend(const uint8_t request[], size_t requestLength);

  /**
   * @brief Sends a raw command to MiP and blocks until the expected response is
   * received or times out.
   *
   * @param[in]  request            Buffer containing the raw byte command.
   * @param[in]  requestLength      Number of bytes in the request buffer.
   * @param[out] responseBuffer     Buffer where received response data from MiP
   * will be written.
   * @param[in]  responseBufferSize Maximum capacity of the response buffer.
   * @param[out] responseLength     Reference receiving the actual number of
   * response bytes written.
   * @return uint8_t Status code (returns MIP_ERROR_NONE on success).
   */
  uint8_t rawReceive(
    const uint8_t request[], size_t requestLength, uint8_t responseBuffer[],
    size_t responseBufferSize, size_t& responseLength);

  /**
   * @brief Processes all incoming UART data from MiP in the serial receive
   * buffer.
   *
   * @details Parses incoming hex-ASCII character pairs into binary payload
   * bytes, matching expected command responses or routing out-of-band events
   * (e.g., radar, clap, gesture) to MiP.
   *
   * @return true if an expected command response was successfully found and
   * processed.
   */
  bool processAllResponseData();

protected:
  static constexpr uint8_t MIP_REQUEST_DELAY = 8;       ///< Minimum delay (ms)
                                                        ///< between consecutive
                                                        ///< requests.
  static constexpr uint8_t MIP_RESPONSE_TIMEOUT = 100;  ///< Time (ms) to wait
                                                        ///< for a response from
                                                        ///< MiP before timeout.
  static constexpr bool MIP_EXPECT_NO_RESPONSE = false;  ///< Flag indicating no
                                                         ///< response is
                                                         ///< expected.
  static constexpr bool MIP_EXPECT_RESPONSE = true;      ///< Flag indicating a
                                                     ///< response is expected.

  static constexpr size_t MIP_REQUEST_MAX_LEN = 17 + 1;  ///< Longest request is
                                                         ///< MIP_CMD_PLAY_SOUND.
  static constexpr size_t MIP_RESPONSE_MAX_LEN = 5 + 1;  ///< Longest response
                                                         ///< is
                                                         ///< MIP_CMD_GET_CHEST_LED.

  void clear();
  uint8_t discardUnexpectedSerialData();

private:
  /**
   * @brief Private constructor; instantiated strictly by MiP orchestrator.
   * @param mip Reference to the main MiP instance for system access and event
   * dispatching.
   */
  explicit MiP_Serial(MiP& mip);

  void processOobResponseData(uint8_t commandByte);
  uint8_t transportGetResponse(
    uint8_t* pResponseBuffer, size_t responseBufferSize, size_t* pResponseLength);
  void transportSendRequest(const uint8_t* pRequest, size_t requestLength, bool expectResponse);

  static void copyHexTextToBinary(uint8_t* pDest, const uint8_t* pSrc, size_t length);
  static constexpr uint8_t parseHexDigit(uint8_t digit);
  bool readIrLength(size_t& length);

  MiP& m_mip;  ///< Reference to the main MiP orchestrator object
  uint32_t m_lastRequestTime;         ///< Timestamp of last request sent to MiP
  uint8_t m_expectedResponseSize;     ///< Size in bytes of expected response
  uint8_t m_expectedResponseCommand;  ///< Command byte of expected response
  uint8_t m_responseBuffer[MIP_RESPONSE_MAX_LEN];  ///< Internal buffer for
                                                   ///< response construction

  friend class MiP;
};

#endif  // MPU_SERIAL_H
