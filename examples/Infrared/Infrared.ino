/**
 * @file Infrared.ino
 * @brief Exhaustive test and wireless demonstration of MiP's Infrared capabilities.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Infrared class, consolidating MiP detection mode and 
 * Dongle Code sending/receiving into a single interactive demonstration.
 *
 * Because testing IR effectively requires two MiPs to interact over a physical 
 * distance, this sketch uses the wireless testing framework via MiP_Debug.
 *
 * **Two-MiP Testing Scheme:**
 * Load this exact sketch onto TWO different MiP robots. The sketch automatically 
 * determines its role based on the `SECRET_HOSTNAME` defined in `secrets.h`:
 *   - "MiP-IR-1" acts as the **Receiver**. It broadcasts its detection ID and 
 *     waits to receive and print IR dongle codes and detection events.
 *   - "MiP-IR-2" acts as the **Transmitter**. It runs through the automated 
 *     API testing suite, then transmits dongle codes to MiP 1.
 *
 * Exhaustively tests the following APIs:
 *   - infrared.enableMiPDetectionMode()
 *   - infrared.disableMiPDetectionMode()
 *   - infrared.isMiPDetectionModeEnabled()
 *   - infrared.enableRemoteControl()
 *   - infrared.disableRemoteControl()
 *   - infrared.isRemoteControlEnabled()
 *   - infrared.sendDongleCode()
 *   - infrared.availableCodeEvents()
 *   - infrared.readDongleCode()
 *   - infrared.availableDetectedMiPEvents()
 *   - infrared.readDetectedMiP()
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

  // In secrets.h, define:
  // #define SECRET_SSID "Your_Network"
  // #define SECRET_PASS "Your_Password"
  // #define SECRET_HOSTNAME "MiP-IR-1" // Use "MiP-IR-2" on the second robot
  const char* ssid = SECRET_SSID;
  const char* password = SECRET_PASS;
  const char* hostname = SECRET_HOSTNAME;
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

// Determine role based on hostname
bool isTransmitter = false;

/**
 * @brief Helper function to print a PASS/FAIL row in the summary table.
 */
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

/**
 * @brief Runs the exhaustive API test sequence for the Transmitter MiP.
 */
void runTransmitterTests() {
  TEST_PRINTLN(F("Infrared.ino: Starting Exhaustive MiP_Infrared Tests (Transmitter Mode)..."));
  TEST_PRINTLN();

  bool t_disableDetect = false;
  bool t_enableDetect = false;
  bool t_isDetectEnabled = false;
  bool t_disableRemote = false;
  bool t_enableRemote = false;
  bool t_isRemoteEnabled = false;
  bool t_sendCode = false;

  // ---------------------------------------------------------
  // TEST 1: disableMiPDetectionMode() & isMiPDetectionModeEnabled()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 1: disableMiPDetectionMode()"));
  mip.infrared.disableMiPDetectionMode();
  t_disableDetect = !mip.didLastCallFail();

  bool isDetectEnabled = mip.infrared.isMiPDetectionModeEnabled();
  bool t_detect_false = !mip.didLastCallFail() && (isDetectEnabled == false);
  delay(500);

  // ---------------------------------------------------------
  // TEST 2: enableMiPDetectionMode()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 2: enableMiPDetectionMode(ID: 0x10)"));
  mip.infrared.enableMiPDetectionMode(0x10, 0x78);
  t_enableDetect = !mip.didLastCallFail();

  isDetectEnabled = mip.infrared.isMiPDetectionModeEnabled();
  bool t_detect_true = !mip.didLastCallFail() && (isDetectEnabled == true);
  
  t_isDetectEnabled = (t_detect_false && t_detect_true);
  delay(500);

  // ---------------------------------------------------------
  // TEST 3: disableRemoteControl() & isRemoteControlEnabled()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 3: disableRemoteControl()"));
  mip.infrared.disableRemoteControl();
  t_disableRemote = !mip.didLastCallFail();

  bool isRemoteEnabled = mip.infrared.isRemoteControlEnabled();
  bool t_remote_false = !mip.didLastCallFail() && (isRemoteEnabled == false);
  delay(500);

  // ---------------------------------------------------------
  // TEST 4: enableRemoteControl()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 4: enableRemoteControl()"));
  mip.infrared.enableRemoteControl();
  t_enableRemote = !mip.didLastCallFail();

  isRemoteEnabled = mip.infrared.isRemoteControlEnabled();
  bool t_remote_true = !mip.didLastCallFail() && (isRemoteEnabled == true);
  
  t_isRemoteEnabled = (t_remote_false && t_remote_true);
  delay(500);

  // ---------------------------------------------------------
  // TEST 5: sendDongleCode()
  // ---------------------------------------------------------
  TEST_PRINTLN(F("Test 5: sendDongleCode() (Fire-and-forget tests)"));
  
  TEST_PRINTLN(F("  -> Transmitting 2-byte code: 0x4567"));
  mip.infrared.sendDongleCode(0x4567, 2, 0x78);
  delay(2000);

  TEST_PRINTLN(F("  -> Transmitting 3-byte code: 0x123456"));
  mip.infrared.sendDongleCode(0x123456, 3, 0x78);
  delay(2000);

  TEST_PRINTLN(F("  -> Transmitting 4-byte code: 0xA1B2C3D4 (via struct)"));
  MiPIRDongleCode code4Byte(0xA1B2C3D4, 4);
  mip.infrared.sendDongleCode(code4Byte, 0x78);
  
  t_sendCode = !mip.didLastCallFail();
  delay(2000);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE TO CONSOLE
  // ---------------------------------------------------------
  TEST_PRINTLN();
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" MiP_Infrared Test Summary (Transmitter)"));
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F(" Method / Feature                 | Result"));
  TEST_PRINTLN(F("----------------------------------|---------------"));
  
  printTestResult("disableMiPDetectionMode()", t_disableDetect);
  printTestResult("enableMiPDetectionMode()", t_enableDetect);
  printTestResult("isMiPDetectionModeEnabled()", t_isDetectEnabled);
  printTestResult("disableRemoteControl()", t_disableRemote);
  printTestResult("enableRemoteControl()", t_enableRemote);
  printTestResult("isRemoteControlEnabled()", t_isRemoteEnabled);
  printTestResult("sendDongleCode()", t_sendCode);
  
  TEST_PRINTLN(F("=================================================="));
  TEST_PRINTLN(F("Infrared.ino: Transmit Sequence Complete."));
}

/**
 * @brief Sets up the Receiver MiP.
 */
void setupReceiver() {
  TEST_PRINTLN(F("Infrared.ino: Receiver Mode Active."));
  TEST_PRINTLN(F("Enabling MiP Detection Mode (ID: 0x20)..."));
  
  // Become discoverable by Transmitter
  mip.infrared.enableMiPDetectionMode(0x20, 0x78);
  
  TEST_PRINTLN(F("Listening for IR dongle codes and detection pings..."));
}

void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Infrared.ino: Failed connecting to MiP."));
    return;
  }

#if defined(ESP8266) || defined(ESP32)
  uint8_t wifiStatus = mip.wifi.begin(ssid, password, hostname);
  if (wifiStatus != WL_CONNECTED) {
    mip.console.println(F("Infrared.ino: Failed connecting to WiFi."));
    connectResult = false;
    return;
  }

  // Determine role based on hostname
  if (strcmp(hostname, "MiP-IR-2") == 0) {
    isTransmitter = true;
  }

  debug.begin(hostname, MiP_Debug::INFO);
  
  mip.console.print(F("Infrared.ino: WiFi Connected as "));
  mip.console.println(hostname);
  mip.console.print(F("Please connect to Telnet at IP: "));
  mip.console.println(WiFi.localIP());
#else
  // On AVR, we run immediately. Default to Receiver mode if undefined.
  isTransmitter = false; 
  setupReceiver();
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

  // If a user just connected via Telnet, kick off tests/setup once
  if (!hasRunTests && debug.isActive(MiP_Debug::INFO)) {
    if (isTransmitter) {
      runTransmitterTests();
    } else {
      setupReceiver();
    }
    hasRunTests = true;
  }
#endif

  // If the automated setup/tests are done, process background IR events
  bool isReady = false;
#if defined(ESP8266) || defined(ESP32)
  isReady = (hasRunTests && debug.isActive(MiP_Debug::INFO));
#else
  isReady = hasRunTests;
#endif

  if (isReady) {
    // ---------------------------------------------------------
    // TEST 6: availableCodeEvents() & readDongleCode()
    // ---------------------------------------------------------
    while (mip.infrared.availableCodeEvents() > 0) {
      MiPIRDongleCode irEvent = mip.infrared.readDongleCode();
      if (irEvent.isValid()) {
        TEST_PRINT(F(" [SUCCESS] Received "));
        TEST_PRINT(irEvent.length);
        TEST_PRINT(F("-byte IR Code: 0x"));

        for (int8_t i = irEvent.length - 1; i >= 0; i--) {
          uint8_t byteVal = static_cast<uint8_t>((irEvent.code >> (i * 8)) & 0xFF);
          if (byteVal < 0x10) TEST_PRINT(F("0"));
          TEST_PRINT(byteVal, HEX);
          if (i > 0) TEST_PRINT(F(" "));
        }
        TEST_PRINTLN();
      }
    }

    // ---------------------------------------------------------
    // TEST 7: availableDetectedMiPEvents() & readDetectedMiP()
    // ---------------------------------------------------------
    while (mip.infrared.availableDetectedMiPEvents() > 0) {
      uint8_t detectedId = mip.infrared.readDetectedMiP();
      TEST_PRINT(F(" [SUCCESS] Detected another MiP via IR with ID: 0x"));
      if (detectedId < 0x10) TEST_PRINT(F("0"));
      TEST_PRINTLN(detectedId, HEX);
    }
  }

  delay(20);
}
