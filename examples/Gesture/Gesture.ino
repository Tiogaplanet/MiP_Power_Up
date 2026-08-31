/**
 * @file Gesture.ino
 * @brief Exhaustive test and wireless demonstration of MiP's gesture detection.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Gesture class. Because testing gestures requires MiP
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
 * disabling gesture mode, checking state, and querying the event queue. It then
 * prints an exhaustive summary table. Afterwards, it enters an interactive mode, 
 * printing MiP's detected hand gestures in real-time.
 *
 * Exhaustively tests the following APIs:
 *   - gesture.enable()
 *   - gesture.disable()
 *   - gesture.isEnabled()
 *   - gesture.availableEvents()
 *   - gesture.readEvent()
 *   - gesture.areGestureAndRadarModesDisabled()
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
  const char* hostname = "MiP-Gesture";
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
  TEST_PRINTLN(F("Gesture.ino: Starting Exhaustive MiP_Gesture Tests..."));
  TEST_PRINTLN();

  bool t_disable = false;
  bool t_areBothDisabled = false;
  bool t_enable = false;
  bool t_isEnabled = false;
  bool t_availableEvents = false;
  bool t_readEvent = false;

  TEST_PRINTLN(F("Waiting for MiP to be standing upright to test gestures..."));
  while (!mip.position.isUpright()) {
    delay(100); // Yield CPU time to prevent watchdog resets while waiting
  }
  
  // Give MiP a moment to settle after being stood up
  delay(1000);

  // ---------------------------------------------------------
  // TEST 1: disable() & areGestureAndRadarModesDisabled()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 1: disable() & areGestureAndRadarModesDisabled()"));
  mip.gesture.disable();
  t_disable = !mip.didLastCallFail();

  bool bothDisabled = mip.gesture.areGestureAndRadarModesDisabled();
  t_areBothDisabled = !mip.didLastCallFail() && bothDisabled;
  delay(500);

  // ---------------------------------------------------------
  // TEST 2: enable() & isEnabled()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 2: enable() & isEnabled()"));
  mip.gesture.enable();
  t_enable = !mip.didLastCallFail();

  bool isGestEnabled = mip.gesture.isEnabled();
  t_isEnabled = !mip.didLastCallFail() && isGestEnabled;
  delay(500);

  // ---------------------------------------------------------
  // TEST 3: availableEvents() & readEvent() (Empty Queue Test)
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 3: Queue functions (Empty State)"));
  
  // Drain any incidental gestures that might have triggered
  while (mip.gesture.availableEvents() > 0) {
    mip.gesture.readEvent();
  }
  
  uint8_t avail = mip.gesture.availableEvents();
  t_availableEvents = !mip.didLastCallFail() && (avail == 0);

  MiPGesture event = mip.gesture.readEvent();
  // Expect an error because we tried to read from an empty queue
  t_readEvent = (mip.lastCallResult() == MiP::MIP_ERROR_NO_EVENT && event == MIP_GESTURE_INVALID);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE TO CONSOLE
  // ---------------------------------------------------------
  TEST_PRINTLN();
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" MiP_Gesture Exhaustive Test Summary"));
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" Method / Feature                 | Result"));
  TEST_PRINTLN(F("----------------------------------|---------------"));
  
  printTestResult("disable()", t_disable);
  printTestResult("areGestureAndRadarModesDisabled()", t_areBothDisabled);
  printTestResult("enable()", t_enable);
  printTestResult("isEnabled()", t_isEnabled);
  printTestResult("availableEvents() (Empty)", t_availableEvents);
  printTestResult("readEvent() (Empty)", t_readEvent);
  
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F("Gesture.ino: Tests Complete."));
  TEST_PRINTLN();
  TEST_PRINTLN(F("Now, try swiping your hand in front of MiP's sensors!"));
  TEST_PRINTLN(F("Monitoring hand gestures in real-time..."));
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
    mip.console.println(F("Gesture.ino: Failed connecting to MiP."));
    return;
  }

#if defined(ESP8266) || defined(ESP32)
  uint8_t wifiStatus = mip.wifi.begin(ssid, password, hostname);
  if (wifiStatus != WL_CONNECTED) {
    mip.console.println(F("Gesture.ino: Failed connecting to WiFi."));
    connectResult = false;
    return;
  }

  // Start telnet debug server
  debug.begin(hostname, MiP_Debug::INFO);
  
  // Print connection info to the USB Serial monitor before disconnecting it
  mip.console.println(F("Gesture.ino: WiFi Connected!"));
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
 * Afterwards, it continuously polls for gesture events and wirelessly prints
 * any detected hand motions.
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

  // If the automated tests are done, stream live gesture updates
  bool shouldStream = false;
#if defined(ESP8266) || defined(ESP32)
  shouldStream = (hasRunTests && debug.isActive(MiP_Debug::INFO));
#else
  shouldStream = hasRunTests;
#endif

  if (shouldStream) {
    while (mip.gesture.availableEvents() > 0) {
      MiPGesture gesture = mip.gesture.readEvent();
      TEST_PRINT(F(" Detected "));
      switch (gesture) {
        case MIP_GESTURE_LEFT:
          TEST_PRINTLN(F("Left gesture!"));
          break;
        case MIP_GESTURE_RIGHT:
          TEST_PRINTLN(F("Right gesture!"));
          break;
        case MIP_GESTURE_CENTER_SWEEP_LEFT:
          TEST_PRINTLN(F("Center Sweep Left gesture!"));
          break;
        case MIP_GESTURE_CENTER_SWEEP_RIGHT:
          TEST_PRINTLN(F("Center Sweep Right gesture!"));
          break;
        case MIP_GESTURE_CENTER_HOLD:
          TEST_PRINTLN(F("Center Hold gesture!"));
          break;
        case MIP_GESTURE_FORWARD:
          TEST_PRINTLN(F("Forward gesture!"));
          break;
        case MIP_GESTURE_BACKWARD:
          TEST_PRINTLN(F("Backward gesture!"));
          break;
        case MIP_GESTURE_INVALID:
          TEST_PRINTLN(F("INVALID gesture!"));
          break;
      }
    }
  }

  // Yield control briefly to prevent watchdog reset triggers
  delay(10);
}
