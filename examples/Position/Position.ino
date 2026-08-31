/**
 * @file Position.ino
 * @brief Exhaustive test and wireless demonstration of MiP's position reporting.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Position class. 
 *
 * On ESP8266/ESP32 targets (like the D1 mini):
 * Testing position requires physically moving MiP (which requires disconnecting
 * the USB cable). This sketch connects to WiFi and outputs all test results
 * wirelessly via a Telnet server using the MiP_Debug class. WiFi credentials
 * are securely loaded from a local secrets.h file.
 *
 * On AVR targets (like the Pro Mini):
 * The sketch falls back to standard USB console output.
 *
 * Exhaustively tests the following APIs:
 *   - position.read()
 *   - position.isOnBack()
 *   - position.isFaceDown()
 *   - position.isUpright()
 *   - position.isPickedUp()
 *   - position.isHandStanding()
 *   - position.isFaceDownOnTray()
 *   - position.isOnBackWithKickstand()
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
  const char* hostname = "MiP-Position";
  MiP_Debug debug;
#endif

MiP mip;
bool connectResult;
bool hasRunTests = false;
MiPPosition lastPosition = static_cast<MiPPosition>(-1);

// Helper macro to route printing to either Telnet (ESP) or USB Console (AVR)
#if defined(ESP8266) || defined(ESP32)
  #define TEST_PRINT(...)    debug.print(__VA_ARGS__)
  #define TEST_PRINTLN(...)  debug.println(__VA_ARGS__)
#else
  #define TEST_PRINT(...)    mip.console.print(__VA_ARGS__)
  #define TEST_PRINTLN(...)  mip.console.println(__VA_ARGS__)
#endif

void printTestResult(const char* testName, bool passed) {
  TEST_PRINT(F(" "));
  TEST_PRINT(testName);
  
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

void runExhaustiveTests() {
  TEST_PRINTLN(F("Position.ino: Starting Exhaustive MiP_Position Tests..."));
  TEST_PRINTLN();

  bool t_read = false;
  bool t_isOnBack = false;
  bool t_isFaceDown = false;
  bool t_isUpright = false;
  bool t_isPickedUp = false;
  bool t_isHandStanding = false;
  bool t_isFaceDownOnTray = false;
  bool t_isOnBackWithKickstand = false;

  TEST_PRINTLN(F("Testing position.read() and boolean state APIs..."));
  
  mip.position.read();
  t_read = !mip.didLastCallFail();

  mip.position.isOnBack();
  t_isOnBack = !mip.didLastCallFail();

  mip.position.isFaceDown();
  t_isFaceDown = !mip.didLastCallFail();

  mip.position.isUpright();
  t_isUpright = !mip.didLastCallFail();

  mip.position.isPickedUp();
  t_isPickedUp = !mip.didLastCallFail();

  mip.position.isHandStanding();
  t_isHandStanding = !mip.didLastCallFail();

  mip.position.isFaceDownOnTray();
  t_isFaceDownOnTray = !mip.didLastCallFail();

  mip.position.isOnBackWithKickstand();
  t_isOnBackWithKickstand = !mip.didLastCallFail();

  TEST_PRINTLN();
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" MiP_Position Exhaustive Test Summary"));
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" Method / Feature                 | Result"));
  TEST_PRINTLN(F("----------------------------------|---------------"));
  
  printTestResult("read()", t_read);
  printTestResult("isOnBack()", t_isOnBack);
  printTestResult("isFaceDown()", t_isFaceDown);
  printTestResult("isUpright()", t_isUpright);
  printTestResult("isPickedUp()", t_isPickedUp);
  printTestResult("isHandStanding()", t_isHandStanding);
  printTestResult("isFaceDownOnTray()", t_isFaceDownOnTray);
  printTestResult("isOnBackWithKickstand()", t_isOnBackWithKickstand);
  
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F("Position.ino: Tests Complete."));
  TEST_PRINTLN();
  TEST_PRINTLN(F("Now, try picking MiP up or placing him on his back!"));
  TEST_PRINTLN(F("Monitoring physical position in real-time..."));
}

void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Position.ino: Failed connecting to MiP."));
    return;
  }

#if defined(ESP8266) || defined(ESP32)
  uint8_t wifiStatus = mip.wifi.begin(ssid, password, hostname);
  if (wifiStatus != WL_CONNECTED) {
    mip.console.println(F("Position.ino: Failed connecting to WiFi."));
    connectResult = false;
    return;
  }

  debug.begin(hostname, MiP_Debug::INFO);
  
  mip.console.println(F("Position.ino: WiFi Connected!"));
  mip.console.print(F("Please connect to Telnet at IP: "));
  mip.console.println(WiFi.localIP());
  mip.console.println(F("You may now disconnect the USB cable and move MiP freely."));
#else
  // On AVR, we don't wait for WiFi. Run tests immediately.
  runExhaustiveTests();
  hasRunTests = true;
#endif
}

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

  // If the automated tests are done, stream live position updates
  bool shouldStream = false;
#if defined(ESP8266) || defined(ESP32)
  shouldStream = (hasRunTests && debug.isActive(MiP_Debug::INFO));
#else
  shouldStream = hasRunTests;
#endif

  if (shouldStream) {
    MiPPosition currentPosition = mip.position.read();

    if (currentPosition != lastPosition) {
      if (mip.position.isOnBack()) { TEST_PRINTLN(F(" State Change: MiP is On Back")); }
      if (mip.position.isFaceDown()) { TEST_PRINTLN(F(" State Change: MiP is Face Down")); }
      if (mip.position.isUpright()) { TEST_PRINTLN(F(" State Change: MiP is Upright")); }
      if (mip.position.isPickedUp()) { TEST_PRINTLN(F(" State Change: MiP was Picked Up")); }
      if (mip.position.isHandStanding()) { TEST_PRINTLN(F(" State Change: MiP is Hand Standing")); }
      if (mip.position.isFaceDownOnTray()) { TEST_PRINTLN(F(" State Change: MiP is Face Down on Tray")); }
      if (mip.position.isOnBackWithKickstand()) { TEST_PRINTLN(F(" State Change: MiP is On Back With Kickstand")); }

      lastPosition = currentPosition;
    }
  }

  delay(50);
}
