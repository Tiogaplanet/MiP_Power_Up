/**
 * @file Core.ino
 * @brief Exhaustive test and wireless demonstration of the core MiP lifecycle APIs.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the main MiP orchestrator class. Because the final test puts 
 * MiP to sleep (which causes him to drop to the floor), this sketch connects 
 * to WiFi and outputs all test results wirelessly via a Telnet server using 
 * the MiP_Debug class, preventing USB cable snags. WiFi credentials are 
 * securely loaded from a local secrets.h file.
 *
 * On AVR targets (like the Pro Mini), the sketch falls back to standard USB
 * console output.
 *
 * Once you connect to MiP's IP address via a Telnet client, the sketch will
 * verify the connection, test error-handling states, demonstrate disconnecting
 * and reconnecting, and finally put MiP into a deep sleep mode.
 *
 * Exhaustively tests the following core APIs:
 *   - begin()
 *   - isInitialized()
 *   - getBaudRate()
 *   - didLastCallFail()
 *   - lastCallResult()
 *   - printLastCallResult()
 *   - end()
 *   - sleep()
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
  const char* hostname = "MiP-Core";
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
  TEST_PRINTLN(F("Core.ino: Starting Exhaustive MiP Lifecycle Tests..."));
  TEST_PRINTLN();

  bool t_begin = connectResult;
  bool t_isInitialized = false;
  bool t_getBaudRate = false;
  bool t_errorHandling = false;
  bool t_end = false;
  bool t_sleep = false;

  // ---------------------------------------------------------
  // TEST 1: isInitialized() & getBaudRate()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 1: isInitialized() & getBaudRate()"));
  t_isInitialized = mip.isInitialized();
  uint32_t baud = mip.getBaudRate();
  t_getBaudRate = (baud == 115200 || baud == 9600);
  
  TEST_PRINT(F("  -> Connection Initialized: "));
  TEST_PRINTLN(t_isInitialized ? F("True") : F("False"));
  TEST_PRINT(F("  -> Negotiated Baud Rate: "));
  TEST_PRINTLN(baud);
  delay(1000);

  // ---------------------------------------------------------
  // TEST 2: Error Handling APIs
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 2: Error Handling (didLastCallFail, lastCallResult)"));
  TEST_PRINTLN(F("  -> Inducing an intentional API error by reading an empty queue..."));
  
  // Drain queue and try to read to trigger MIP_ERROR_NO_EVENT
  while (mip.clap.availableEvents() > 0) mip.clap.readEvent();
  mip.clap.readEvent(); 
  
  t_errorHandling = mip.didLastCallFail() && (mip.lastCallResult() == MiP::MIP_ERROR_NO_EVENT);
  
  // Note: printLastCallResult() automatically routes to the USB Hardware Serial debug port.
  mip.printLastCallResult(); 
  delay(1000);

  // ---------------------------------------------------------
  // TEST 3: end()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 3: end()"));
  TEST_PRINTLN(F("  -> Disconnecting from MiP. Chest LED should turn blue."));
  mip.end();
  t_end = !mip.isInitialized();
  delay(3000); // Wait long enough for the user to observe the disconnect

  // Reconnect for the final test
  TEST_PRINTLN(F("  -> Reconnecting for the final test..."));
  bool reconnected = mip.begin();
  if (!reconnected) {
    TEST_PRINTLN(F("  -> Failed to reconnect!"));
  }
  delay(1000);

  // ---------------------------------------------------------
  // TEST 4: sleep()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 4: sleep()"));
  if (reconnected) {
    TEST_PRINTLN(F("  -> Putting MiP to sleep. Watch him fall!"));
    mip.sleep();
    t_sleep = true;
  }
  delay(1500);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE TO CONSOLE
  // ---------------------------------------------------------
  TEST_PRINTLN();
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" MiP Core API Exhaustive Test Summary"));
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" Method / Feature                 | Result"));
  TEST_PRINTLN(F("----------------------------------|---------------"));
  
  printTestResult("begin()", t_begin);
  printTestResult("isInitialized()", t_isInitialized);
  printTestResult("getBaudRate()", t_getBaudRate);
  printTestResult("Error Handling API", t_errorHandling);
  printTestResult("end()", t_end);
  printTestResult("sleep()", t_sleep);
  
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F("Core.ino: Tests Complete."));
  TEST_PRINTLN(F("Note: MiP requires a physical power cycle before accepting new connections."));
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
    mip.console.println(F("Core.ino: Failed connecting to MiP."));
    return;
  }

#if defined(ESP8266) || defined(ESP32)
  uint8_t wifiStatus = mip.wifi.begin(ssid, password, hostname);
  if (wifiStatus != WL_CONNECTED) {
    mip.console.println(F("Core.ino: Failed connecting to WiFi."));
    connectResult = false;
    return;
  }

  // Start telnet debug server
  debug.begin(hostname, MiP_Debug::INFO);
  
  // Print connection info to the USB Serial monitor before disconnecting it
  mip.console.println(F("Core.ino: WiFi Connected!"));
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
