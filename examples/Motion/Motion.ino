/**
 * @file Motion.ino
 * @brief Exhaustive test and wireless demonstration of MiP's motion control.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Motion class, consolidating several individual motion
 * demonstrations into one complete sequence. 
 *
 * Because testing motion requires MiP to physically drive and fall over (which 
 * is restricted if a USB cable is attached), this sketch connects to WiFi and 
 * outputs all test results wirelessly via a Telnet server using the MiP_Debug 
 * class. WiFi credentials are securely loaded from a local secrets.h file.
 *
 * On AVR targets (like the Pro Mini), the sketch falls back to standard USB
 * console output.
 *
 * Once you connect to MiP's IP address via a Telnet client, the sketch will
 * verify that MiP is upright, run through a sequence of short drives and turns
 * designed to keep him in a small area, demonstrate stopping, and finally 
 * command him to fall down and get back up.
 *
 * Exhaustively tests the following APIs:
 *   - motion.driveForward()
 *   - motion.driveBackward()
 *   - motion.continuousDrive()
 *   - motion.stop()
 *   - motion.distanceDrive()
 *   - motion.turnLeft()
 *   - motion.turnRight()
 *   - motion.fallForward()
 *   - motion.fallBackward()
 *   - motion.getUp()
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
  const char* hostname = "MiP-Motion";
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
  TEST_PRINTLN(F("Motion.ino: Starting Exhaustive MiP_Motion Tests..."));
  TEST_PRINTLN();

  bool t_driveForward = false;
  bool t_driveBackward = false;
  bool t_continuousDrive = false;
  bool t_stop = false;
  bool t_distanceDrive = false;
  bool t_turnLeft = false;
  bool t_turnRight = false;
  bool t_fallForward = false;
  bool t_getUpFront = false;
  bool t_fallBackward = false;
  bool t_getUpBack = false;

  TEST_PRINTLN(F("Waiting for MiP to be standing upright to begin motion sequence..."));
  while (!mip.position.isUpright()) {
    delay(100); // Yield CPU time to prevent watchdog resets while waiting
  }
  
  // Give MiP a moment to settle
  delay(1500);

  // ---------------------------------------------------------
  // TEST 1: driveForward() & driveBackward()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 1: driveForward() & driveBackward()"));
  mip.motion.driveForward(15, 1000);
  t_driveForward = !mip.didLastCallFail();
  delay(2000); // Allow motion to complete

  mip.motion.driveBackward(15, 1000);
  t_driveBackward = !mip.didLastCallFail();
  delay(2000);

  // ---------------------------------------------------------
  // TEST 2: continuousDrive() & stop()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 2: continuousDrive() & stop()"));
  // Drive forward slowly while turning left
  mip.motion.continuousDrive(12, -12);
  t_continuousDrive = !mip.didLastCallFail();
  
  delay(1500); // Let him spin for a moment

  mip.motion.stop();
  t_stop = !mip.didLastCallFail();
  delay(1000);

  // ---------------------------------------------------------
  // TEST 3: distanceDrive()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 3: distanceDrive()"));
  // Drive forward 10cm, then turn right 90 degrees
  mip.motion.distanceDrive(MIP_DRIVE_FORWARD, 10, MIP_TURN_RIGHT, 90);
  t_distanceDrive = !mip.didLastCallFail();
  delay(3000); // Allow combined maneuver to complete

  // ---------------------------------------------------------
  // TEST 4: turnLeft() & turnRight()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 4: turnLeft() & turnRight()"));
  mip.motion.turnLeft(90, 15);
  t_turnLeft = !mip.didLastCallFail();
  delay(2000);

  mip.motion.turnRight(90, 15);
  t_turnRight = !mip.didLastCallFail();
  delay(2000);

  // ---------------------------------------------------------
  // TEST 5: fallForward() & getUp()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 5: fallForward() & getUp(FRONT)"));
  mip.motion.fallForward();
  t_fallForward = !mip.didLastCallFail();
  
  delay(2000); // Give him time to hit the floor

  mip.motion.getUp(MIP_GETUP_FROM_FRONT);
  t_getUpFront = !mip.didLastCallFail();
  
  // Wait for him to balance again
  while (!mip.position.isUpright()) {
    delay(100);
  }
  delay(1500);

  // ---------------------------------------------------------
  // TEST 6: fallBackward() & getUp()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 6: fallBackward() & getUp(BACK)"));
  mip.motion.fallBackward();
  t_fallBackward = !mip.didLastCallFail();
  
  delay(2000); // Give him time to hit the floor

  mip.motion.getUp(MIP_GETUP_FROM_BACK);
  t_getUpBack = !mip.didLastCallFail();
  
  // Wait for him to balance again
  while (!mip.position.isUpright()) {
    delay(100);
  }
  delay(1500);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE TO CONSOLE
  // ---------------------------------------------------------
  TEST_PRINTLN();
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" MiP_Motion Exhaustive Test Summary"));
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" Method / Feature                 | Result"));
  TEST_PRINTLN(F("----------------------------------|---------------"));
  
  printTestResult("driveForward()", t_driveForward);
  printTestResult("driveBackward()", t_driveBackward);
  printTestResult("continuousDrive()", t_continuousDrive);
  printTestResult("stop()", t_stop);
  printTestResult("distanceDrive()", t_distanceDrive);
  printTestResult("turnLeft()", t_turnLeft);
  printTestResult("turnRight()", t_turnRight);
  printTestResult("fallForward()", t_fallForward);
  printTestResult("getUp(MIP_GETUP_FROM_FRONT)", t_getUpFront);
  printTestResult("fallBackward()", t_fallBackward);
  printTestResult("getUp(MIP_GETUP_FROM_BACK)", t_getUpBack);
  
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F("Motion.ino: Tests Complete."));
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
    mip.console.println(F("Motion.ino: Failed connecting to MiP."));
    return;
  }

#if defined(ESP8266) || defined(ESP32)
  uint8_t wifiStatus = mip.wifi.begin(ssid, password, hostname);
  if (wifiStatus != WL_CONNECTED) {
    mip.console.println(F("Motion.ino: Failed connecting to WiFi."));
    connectResult = false;
    return;
  }

  // Start telnet debug server
  debug.begin(hostname, MiP_Debug::INFO);
  
  // Print connection info to the USB Serial monitor before disconnecting it
  mip.console.println(F("Motion.ino: WiFi Connected!"));
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
