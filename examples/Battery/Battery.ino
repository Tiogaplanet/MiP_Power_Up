/**
 * @file Battery.ino
 * @brief Exhaustive test and demonstration of MiP's battery monitoring.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Battery class. It exercises the single public method
 * available in the class and verifies that a plausible voltage is returned
 * without generating any internal API errors.
 *
 * A summary table is printed to the console at the end of the test.
 *
 * Exhaustively tests the following APIs:
 *   - battery.readVoltage()
 *
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <MiP_Power_Up.h>

/**
 * @brief Global MiP instance used to communicate with MiP.
 */
MiP mip;

/**
 * @brief Stores the result of the MiP initialization attempt.
 */
bool connectResult;

/**
 * @brief Helper function to print a PASS/FAIL row in the summary table.
 *
 * @param testName The name of the method or feature tested.
 * @param passed True if the test passed, false if it failed.
 */
void printTestResult(const char* testName, bool passed) {
  mip.console.print(F(" "));
  mip.console.print(testName);

  // Calculate padding to align the results column (32 characters wide)
  int padding = 33 - strlen(testName);
  for (int i = 0; i < padding; i++) { mip.console.print(F(" ")); }

  mip.console.print(F("| "));
  if (passed) {
    mip.console.println(F("PASS"));
  } else {
    mip.console.println(F("FAIL"));
  }
}

/**
 * @brief Arduino setup function.
 *
 * @details Initializes communication with MiP, retrieves his current battery
 * voltage, verifies the reading is valid, and outputs a formatted summary
 * table.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Battery.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("Battery.ino: Starting Exhaustive MiP_Battery Tests..."));
  mip.console.println();

  // Test tracking variable
  bool t_readVoltage = false;

  // ---------------------------------------------------------
  // TEST 1: readVoltage()
  // ---------------------------------------------------------
  mip.console.println(F("Test 1: Reading cached battery voltage"));

  // Give MiP a moment to ensure background status events have updated the cache
  delay(100);

  float currentVoltage = mip.battery.readVoltage();

  mip.console.print(F("  -> MiP reported battery voltage: "));
  mip.console.print(currentVoltage, 2);
  mip.console.println(F("V"));

  // Check if the call succeeded and returned a plausible non-zero voltage
  if (!mip.didLastCallFail() && currentVoltage > 0.0f) { t_readVoltage = true; }

  delay(500);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE
  // ---------------------------------------------------------
  mip.console.println();
  mip.console.println(F("=================================================="));
  mip.console.println(F(" MiP_Battery Exhaustive Test Summary"));
  mip.console.println(F("=================================================="));
  mip.console.println(F(" Method / Feature                 | Result"));
  mip.console.println(F("----------------------------------|---------------"));

  printTestResult("readVoltage()", t_readVoltage);

  mip.console.println(F("=================================================="));
  mip.console.println(F("Battery.ino: Done."));
}

/**
 * @brief Arduino loop function.
 *
 * @details This example performs all actions in setup() and does not require
 * repeated work in loop().
 */
void loop() {
  if (!connectResult) { return; }
}
