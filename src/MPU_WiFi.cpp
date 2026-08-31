/**
 * @file MPU_WiFi.cpp
 * @brief Implements WiFi management for the MiP library.
 *
 * @details This source file implements Wi-Fi setup, connection handling, and
 * cleanup on ESP8266 and ESP32 targets.
 *
 * @author Samuel Trassare (Original Author)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may
 * obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#if defined(ESP8266) || defined(ESP32)

#include "MPU_WiFi.h"
#include "MiP_Power_Up.h"

// Implement the constructor to store the MiP reference.
MiP_WiFi::MiP_WiFi(MiP& mip) : m_mip(mip) {
  clear();
}

uint8_t MiP_WiFi::begin(
  const char* ssid, const char* password, const char* hostname /* = "MiP" */) {
  // Memory-safe string copy operations:
  strncpy(m_ssid, ssid, sizeof(m_ssid) - 1);
  m_ssid[sizeof(m_ssid) - 1] = '\0';

  strncpy(m_password, password, sizeof(m_password) - 1);
  m_password[sizeof(m_password) - 1] = '\0';

  strncpy(m_hostname, hostname, sizeof(m_hostname) - 1);
  m_hostname[sizeof(m_hostname) - 1] = '\0';

  WiFi.hostname(m_hostname);

  return connect();
}

void MiP_WiFi::enableAirplaneMode() {
  WiFi.disconnect();       // Disconnect from current network.
  WiFi.mode(WIFI_OFF);     // Turn off WiFi radio.
  WiFi.forceSleepBegin();  // Put the WiFi modem to sleep.

  // App mode broadcasts BLE. If MiP is currently in app mode, switch to
  // the default gesture mode.
  if (m_mip.mode.isAppEnabled()) { m_mip.gesture.enable(); }
}

uint8_t MiP_WiFi::disableAirplaneMode() {
  WiFi.forceSleepWake();  // Wake WiFi modem from force-sleep mode
  delay(1);
  WiFi.mode(WIFI_STA);
  return connect();
}

uint8_t MiP_WiFi::connect() {
  // Safety check: ensure we have a valid SSID configured
  if (m_ssid[0] == '\0' || strlen(m_ssid) == 0) {
    MIP_DEBUG_ERROR_PRINTLN(m_mip, F("MiP: No SSID configured. Call wifi.begin() first."));
    return WL_DISCONNECTED;
  }

  WiFi.begin(m_ssid, m_password);

  // Save original head LED state before starting animation
  MiPHeadLEDs originalLEDs;
  m_mip.headLEDs.read(originalLEDs);

  uint8_t attempts = 0;
  uint8_t ledPos = 0;
  bool direction = true;  // true = left-to-right, false = right-to-left

  while (WiFi.status() != WL_CONNECTED && attempts < MAX_CONNECT_ATTEMPTS) {
    // Animate the four head LEDs in a back-and-forth scanning pattern
    MiPHeadLED leds[4] = { MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF };

    leds[ledPos] = MIP_HEAD_LED_ON;

    m_mip.headLEDs.unverifiedWrite(leds[0], leds[1], leds[2], leds[3]);

    // Update scanner position for next frame
    if (direction) {
      ledPos++;
      if (ledPos >= 3) direction = false;
    } else {
      ledPos--;
      if (ledPos == 0) direction = true;
    }

    MIP_DEBUG_WARN_PRINTLN(m_mip, F("MiP: WiFi connection attempt..."));
    delay(ANIMATION_DELAY_MS);
    attempts++;
  }

  // Restore original head LED state
  m_mip.headLEDs.unverifiedWrite(
    originalLEDs.led1, originalLEDs.led2, originalLEDs.led3, originalLEDs.led4);

  uint8_t connectStatus = WiFi.status();
  if (connectStatus == WL_CONNECTED) {
    MIP_DEBUG_INFO_PRINTLN(m_mip, F("MiP: WiFi connected successfully"));

    if (!MDNS.begin(m_hostname)) {
      MIP_DEBUG_ERROR_PRINTLN(m_mip, F("MiP: Error setting up mDNS responder."));
    } else {
      MIP_DEBUG_INFO_PRINT(m_mip, F("MiP: mDNS responder started with hostname of "));
      MIP_DEBUG_INFO_PRINT(m_mip, m_hostname);
      MIP_DEBUG_INFO_PRINTLN(m_mip, F(".local"));

      MIP_DEBUG_INFO_PRINT(m_mip, F("MiP: IP address: "));
      MIP_DEBUG_INFO_PRINTLN(m_mip, WiFi.localIP().toString().c_str());
    }

    // Configure ArduinoOTA callbacks
    ArduinoOTA.onStart([]() {
      String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
      // We can't safely capture m_mip in standard lambdas across all
      // architectures easily, so raw Serial1 is acceptable here if required,
      // though it is usually preferred to avoid debug prints in asynchronous
      // OTA ISR contexts anyway.
    });

    ArduinoOTA.begin();
    return WL_CONNECTED;
  } else {
    MIP_DEBUG_WARN_PRINTLN(m_mip, F("MiP: WiFi connection failed after maximum attempts"));
    // Pulse slow red on chest LED to indicate connection failure
    m_mip.chestLED.write(0xFF, 0x00, 0x00, 800, 800);
    return connectStatus;
  }
}

void MiP_WiFi::clear() {
  memset(m_ssid, 0, sizeof(m_ssid));
  memset(m_password, 0, sizeof(m_password));
  memset(m_hostname, 0, sizeof(m_hostname));
}

#endif  // defined(ESP8266) || defined(ESP32)
