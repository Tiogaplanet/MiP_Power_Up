/**
 * @file MPU_WiFi.h
 * @brief Defines the public interface for WiFi management in the MiP library.
 *
 * @details This header declares the WiFi API used to connect MiP to a network,
 * establishing capabilities like Over-The-Air (OTA) updates and mDNS name
 * resolution on ESP8266 and ESP32 targets.
 *
 * @author Samuel Trassare (Original Author)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may
 * obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef MPU_WIFI_H
#define MPU_WIFI_H

#if defined(ESP8266) || defined(ESP32)

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

// Forward-declare the main MiP class to avoid circular include dependencies.
class MiP;

/**
 * @brief Manages MiP's WiFi capability.
 */
class MiP_WiFi {
public:
  /**
   * @brief Connects to a wireless access point and configures MiP with OTA
   * programming support.
   *
   * @param ssid     The station SSID.
   * @param password The access point's connection password.
   * @param hostname MiP's hostname on the wireless network (default: "MiP").
   * @return uint8_t WL_CONNECTED if successful, otherwise returns WiFi error
   * status code.
   */
  uint8_t begin(const char* ssid, const char* password, const char* hostname = "MiP");

  /**
   * @brief Connects to the configured WiFi access point.
   *
   * @details While attempting to connect, MiP's eyes light up in a
   * Knight Rider-style back-and-forth scanning animation.
   *
   * @return uint8_t WL_CONNECTED if successful, otherwise returns WiFi error
   * status code.
   */
  uint8_t connect();

  /**
   * @brief Turns off WiFi and Bluetooth.
   *
   * @details Disconnects from WiFi, turns off the modem, and if MiP is in app
   * mode, switches MiP to default gesture mode.
   */
  void enableAirplaneMode();

  /**
   * @brief Wakes the WiFi radio and attempts to reconnect to the access point.
   *
   * @return uint8_t WL_CONNECTED if successful, otherwise returns WiFi error
   * status code.
   */
  uint8_t disableAirplaneMode();

protected:
  static constexpr size_t MAX_SSID_LEN = 32;  ///< Max SSID buffer capacity.
  static constexpr size_t MAX_PASSPHRASE_LEN = 64;  ///< Max WPA2 passphrase
                                                    ///< buffer capacity.
  static constexpr size_t MAX_HOSTNAME_LEN = 63;  ///< Max mDNS hostname buffer
                                                  ///< capacity.
  static constexpr uint8_t MAX_CONNECT_ATTEMPTS = 20;  ///< Max connection retry
                                                       ///< iterations in
                                                       ///< connect().
  static constexpr uint16_t ANIMATION_DELAY_MS = 300;  ///< Eye LED scanner
                                                       ///< animation delay per
                                                       ///< iteration in ms.

  void clear();

private:
  /**
   * @brief Private constructor; instantiated strictly by MiP orchestrator.
   * @param mip Reference to the main MiP object for core communication
   * services.
   */
  explicit MiP_WiFi(MiP& mip);

  MiP& m_mip;  // Stores a reference to the main MiP class.
  char m_ssid[MAX_SSID_LEN];
  char m_password[MAX_PASSPHRASE_LEN];
  char m_hostname[MAX_HOSTNAME_LEN];

  /**
   * @brief Allows MiP to call private constructor.
   */
  friend class MiP;
};

#endif  // defined(ESP8266) || defined(ESP32)
#endif  // MPU_WIFI_H
