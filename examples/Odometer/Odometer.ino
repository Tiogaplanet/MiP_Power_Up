/**
 * @file Odometer.ino
 * @brief Exhaustive test and wireless demonstration of MiP's odometer.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Odometer class. Because testing the odometer requires
 * MiP to physically drive (which requires disconnecting the USB cable), this
 * sketch connects to WiFi and outputs all test results wirelessly via a Telnet
 * server using the MiP_Debug class. WiFi credentials are securely loaded from
 * a local secrets.h file.
 *
 * On AVR targets (like the Pro Mini), the sketch falls back to standard USB
 * console output.
 *
 * Once you connect to MiP's IP address via a Telnet client, the sketch will
 * verify that MiP is upright, reset the odometer, drive him forward and
 * backward a short distance, and read back the accumulated distance. Finally,
 * it outputs a formatted summary table.
 *
 * Exhaustively tests the following APIs:
 *   - odometer.reset()
 *   - odometer.read()
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
  const char* hostname = "MiP-Odometer";
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
  TEST_PRINTLN(F("Odometer.ino: Starting Exhaustive MiP_Odometer Tests..."));
  TEST_PRINTLN();

  bool t_reset = false;
  bool t_read_initial = false;
  bool t_read_after = false;

  TEST_PRINTLN(F("Waiting for MiP to be standing upright before driving..."));
  while (!mip.position.isUpright()) {
    delay(100); // Yield CPU time to prevent watchdog resets while waiting
  }
  
  // Give MiP a moment to settle after being stood up
  delay(1000);

  // ---------------------------------------------------------
  // TEST 1: reset()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 1: reset()"));
  mip.odometer.reset();
  t_reset = !mip.didLastCallFail();
  delay(500);

  // ---------------------------------------------------------
  // TEST 2: read() [Initial]
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 2: read() [Initial zero verification]"));
  float initialDistance = mip.odometer.read();
  
  // Implicitly verify that the reset zeroed the odometer
  t_read_initial = !mip.didLastCallFail() && (initialDistance <= 0.5f);
  
  TEST_PRINT(F("  -> Distance since reset: "));
  TEST_PRINT(initialDistance);
  TEST_PRINTLN(F(" cm"));
  delay(500);

  // ---------------------------------------------------------
  // ACTION: Drive to accumulate distance
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Driving forward and backward 15cm to accumulate distance..."));
  
  // Drive forward 15cm
  mip.motion.distanceDrive(MIP_DRIVE_FORWARD, 15, MIP_TURN_RIGHT, 0);
  delay(3000); // Allow time for maneuver to complete
  
  // Drive backward 15cm to return to starting area
  mip.motion.distanceDrive(MIP_DRIVE_BACKWARD, 15, MIP_TURN_RIGHT, 0);
  delay(3000);

  // ---------------------------------------------------------
  // TEST 3: read() [After Drive]
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 3: read() [After Drive accumulation]"));
  float finalDistance = mip.odometer.read();
  
  // Verify that the distance reported increased after driving
  t_read_after = !mip.didLastCallFail() && (finalDistance > initialDistance);

  TEST_PRINT(F("  -> Distance since reset: "));
  TEST_PRINT(finalDistance);
  TEST_PRINTLN(F(" cm"));
  delay(500);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE TO CONSOLE
  // ---------------------------------------------------------
  TEST_PRINTLN();
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" MiP_Odometer Exhaustive Test Summary"));
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" Method / Feature                 | Result"));
  TEST_PRINTLN(F("----------------------------------|---------------"));
  
  printTestResult("reset()", t_reset);
  printTestResult("read() [Initial]", t_read_initial);
  printTestResult("read() [After Drive]", t_read_after);
  
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F("Odometer.ino: Tests Complete."));
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
    mip.console.println(F("Odometer.ino: Failed connecting to MiP."));
    return;
  }

#if defined(ESP8266) || defined(ESP32)
  uint8_t wifiStatus = mip.wifi.begin(ssid, password, hostname);
  if (wifiStatus != WL_CONNECTED) {
    mip.console.println(F("Odometer.ino: Failed connecting to WiFi."));
    connectResult = false;
    return;
  }

  // Start telnet debug server
  debug.begin(hostname, MiP_Debug::INFO);
  
  // Print connection info to the USB Serial monitor before disconnecting it
  mip.console.println(F("Odometer.ino: WiFi Connected!"));
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

  // Yield control briefly to prevent watchdog reset triggers
  delay(50);
}
