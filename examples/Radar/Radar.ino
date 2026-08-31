/**
 * @file Radar.ino
 * @brief Exhaustive test and wireless demonstration of MiP's radar sensing.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Radar class. Because testing radar requires MiP
 * to be upright and balancing (which is easier with the USB cable disconnected),
 * this sketch connects to WiFi and outputs all test results wirelessly via a
 * Telnet server using the MiP_Debug class. WiFi credentials are securely loaded
 * from a local secrets.h file.
 *
 * On AVR targets (like the Pro Mini), the sketch falls back to standard USB
 * console output.
 *
 * Once you connect to MiP's IP address via a Telnet client, the sketch will
 * verify that MiP is upright, execute the automated tests for enabling and 
 * disabling radar mode, checking state, and querying the radar value. It then
 * prints an exhaustive summary table. Afterwards, it enters an interactive mode, 
 * printing MiP's detected radar ranges in real-time.
 *
 * Exhaustively tests the following APIs:
 *   - radar.enable()
 *   - radar.disable()
 *   - radar.isEnabled()
 *   - radar.read()
 *   - radar.clear()
 *
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <MiP_Power_Up.h>

#if defined(ESP8266) || defined(ESP32)
  #include <MPU_Debug.h>
  #include "secrets.h"

  const char* ssid = SECRET_SSID;
  const char* password = SECRET_PASS;
  const char* hostname = "MiP-Radar";
  MiP_Debug debug;
#endif

MiP mip;
bool connectResult;
bool hasRunTests = false;

// Helper macro to route printing to either Telnet (ESP) or USB Console (AVR)
#if defined(ESP8266) || defined(ESP32)
  #define TEST_PRINT(...)    debug.print(__VA_ARGS__)
  #define TEST_PRINTLN(...)  debug.println(__VA_ARGS__)
#else
  #define TEST_PRINT(...)    mip.console.print(__VA_ARGS__)
  #define TEST_PRINTLN(...)  mip.console.println(__VA_ARGS__)
#endif

/**
 * @brief Helper function to print a PASS/FAIL row in the summary table.
 *
 * @param testName The name of the method or feature tested.
 * @param passed True if the test passed, false if it failed.
 */
void printTestResult(const char* testName, bool passed) {
  TEST_PRINT(F(" "));
  TEST_PRINT(testName);
  
  // Calculate padding to align the results column (32 characters wide)
  int padding = 33 - strlen(testName);
  for (int i = 0; i < padding; i++) {
    TEST_PRINT(F(" "));
  }
  
  TEST_PRINT(F("| "));
  if (passed) {
    TEST_PRINTLN(F("PASS"));
  } else {
    TEST_PRINTLN(F("FAIL"));
  }
}

/**
 * @brief Runs the exhaustive API test sequence and prints the summary table.
 */
void runExhaustiveTests() {
  TEST_PRINTLN(F("Radar.ino: Starting Exhaustive MiP_Radar Tests..."));
  TEST_PRINTLN();

  bool t_disable = false;
  bool t_enable = false;
  bool t_isEnabled = false;
  bool t_clear = false;
  bool t_read = false;

  TEST_PRINTLN(F("Waiting for MiP to be standing upright to test radar..."));
  while (!mip.position.isUpright()) {
    delay(100); // Yield CPU time to prevent watchdog resets while waiting
  }
  
  // Give MiP a moment to settle after being stood up
  delay(1000);

  // ---------------------------------------------------------
  // TEST 1: disable() & isEnabled()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 1: disable() & isEnabled()"));
  mip.radar.disable();
  bool disableSuccess = !mip.didLastCallFail();

  bool isEnabledNow = mip.radar.isEnabled();
  bool checkDisableSuccess = !mip.didLastCallFail() && (isEnabledNow == false);
  t_disable = (disableSuccess && checkDisableSuccess);
  delay(500);

  // ---------------------------------------------------------
  // TEST 2: enable() & isEnabled()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 2: enable() & isEnabled()"));
  mip.radar.enable();
  bool enableSuccess = !mip.didLastCallFail();

  isEnabledNow = mip.radar.isEnabled();
  bool checkEnableSuccess = !mip.didLastCallFail() && (isEnabledNow == true);
  
  t_enable = enableSuccess;
  t_isEnabled = (checkDisableSuccess && checkEnableSuccess);
  delay(500);

  // ---------------------------------------------------------
  // TEST 3: clear()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 3: clear()"));
  mip.radar.clear();
  // Read should return invalid immediately after clear before new events arrive
  MiPRadar currentRadar = mip.radar.read();
  t_clear = (currentRadar == MIP_RADAR_INVALID && mip.lastCallResult() == MiP::MIP_ERROR_NO_EVENT);
  
  // ---------------------------------------------------------
  // TEST 4: read() (Wait for valid data)
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 4: read() (Waiting for first valid radar ping...)"));
  uint32_t startWait = millis();
  while (millis() - startWait < 3000) {
    currentRadar = mip.radar.read();
    if (currentRadar != MIP_RADAR_INVALID) {
      t_read = true;
      break;
    }
    delay(50);
  }

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE TO CONSOLE
  // ---------------------------------------------------------
  TEST_PRINTLN();
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" MiP_Radar Exhaustive Test Summary"));
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" Method / Feature                 | Result"));
  TEST_PRINTLN(F("----------------------------------|---------------"));
  
  printTestResult("disable()", t_disable);
  printTestResult("enable()", t_enable);
  printTestResult("isEnabled()", t_isEnabled);
  printTestResult("clear()", t_clear);
  printTestResult("read()", t_read);
  
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F("Radar.ino: Tests Complete."));
  TEST_PRINTLN();
  TEST_PRINTLN(F("Now, try placing your hand in front of MiP's sensors!"));
  TEST_PRINTLN(F("Monitoring radar ranges in real-time..."));
}

/**
 * @brief Arduino setup function.
 *
 * @details Initializes MiP, connects to WiFi, and starts the Telnet debug
 * server. It prints the IP address to the standard USB console so the user
 * knows where to connect.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Radar.ino: Failed connecting to MiP."));
    return;
  }

#if defined(ESP8266) || defined(ESP32)
  uint8_t wifiStatus = mip.wifi.begin(ssid, password, hostname);
  if (wifiStatus != WL_CONNECTED) {
    mip.console.println(F("Radar.ino: Failed connecting to WiFi."));
    connectResult = false;
    return;
  }

  // Start telnet debug server
  debug.begin(hostname, MiP_Debug::INFO);
  
  // Print connection info to the USB Serial monitor before disconnecting it
  mip.console.println(F("Radar.ino: WiFi Connected!"));
  mip.console.print(F("Please connect to Telnet at IP: "));
  mip.console.println(WiFi.localIP());
  mip.console.println(F("You may now disconnect the USB cable and place MiP on the floor."));
#else
  // On AVR, we don't wait for WiFi. Run tests immediately.
  runExhaustiveTests();
  hasRunTests = true;
#endif
}

/**
 * @brief Arduino loop function.
 *
 * @details Services the Telnet server and OTA network tasks. When a Telnet
 * client connects for the first time, it executes runExhaustiveTests().
 * Afterwards, it continuously polls for radar events and wirelessly prints
 * any detected distance changes.
 */
void loop() {
  if (!connectResult) {
    return;
  }

#if defined(ESP8266) || defined(ESP32)
  ArduinoOTA.handle();
  debug.handle();

  // If a user just connected via Telnet, run the automated API tests once
  if (!hasRunTests && debug.isActive(MiP_Debug::INFO)) {
    runExhaustiveTests();
    hasRunTests = true;
  }
#endif

  // If the automated tests are done, stream live radar updates
  bool shouldStream = false;
#if defined(ESP8266) || defined(ESP32)
  shouldStream = (hasRunTests && debug.isActive(MiP_Debug::INFO));
#else
  shouldStream = hasRunTests;
#endif

  if (shouldStream) {
    static MiPRadar lastRadar = MIP_RADAR_INVALID;
    MiPRadar currentRadar = mip.radar.read();

    // Only act when a valid reading is available and it changed since last time.
    if (currentRadar != MIP_RADAR_INVALID && lastRadar != currentRadar) {
      TEST_PRINT(F(" Radar Range = "));
      switch (currentRadar) {
        case MIP_RADAR_NONE: 
          TEST_PRINTLN(F("None")); 
          break;
        case MIP_RADAR_10CM_30CM: 
          TEST_PRINTLN(F("10cm - 30cm (Distant)")); 
          break;
        case MIP_RADAR_0CM_10CM: 
          TEST_PRINTLN(F("0cm - 10cm (Near)")); 
          break;
        default:
          TEST_PRINTLN(F("Unknown"));
          break;
      }
      lastRadar = currentRadar;
    }
  }

  // Yield control briefly to prevent watchdog reset triggers
  delay(10);
}
