/**
 * @file EEPROM.ino
 * @brief Exhaustive test and demonstration of MiP's non-volatile EEPROM
 * storage.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_EEPROM class. It systematically exercises the read and
 * write methods available in the class. It reads the current state of offset 0,
 * writes a test byte, verifies the write via readback, writes 0 to clear it,
 * verifies the clear, and then restores the original state.
 *
 * A summary table is printed to the console at the end of the automated tests.
 *
 * Exhaustively tests the following APIs:
 *   - eeprom.read(uint8_t addressOffset)
 *   - eeprom.write(uint8_t addressOffset, uint8_t userData)
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
 * @details Initializes communication with MiP and runs a sequential, exhaustive
 * test of the EEPROM read and write methods. It verifies state preservation and
 * outputs a formatted summary table.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("EEPROM.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("EEPROM.ino: Starting Exhaustive MiP_EEPROM Tests..."));
  mip.console.println();

  // Test tracking variables
  bool t_read = false;
  bool t_write = false;
  bool t_write_zero = false;

  // Track original state to restore later
  uint8_t originalValue = 0;

  // ---------------------------------------------------------
  // TEST 1: read()
  // ---------------------------------------------------------
  mip.console.println(F("Test 1: Read offset 0 (Preserve original state)"));
  originalValue = mip.eeprom.read(0);
  t_read = !mip.didLastCallFail();

  mip.console.print(F("  -> Original Value at Offset 0: 0x"));
  if (originalValue < 0x10) mip.console.print(F("0"));
  mip.console.println(originalValue, HEX);

  delay(500);

  // ---------------------------------------------------------
  // TEST 2: write() & read() (Test byte 0xAA)
  // ---------------------------------------------------------
  mip.console.println(F("Test 2: Write 0xAA to offset 0 and verify readback"));
  mip.eeprom.write(0, 0xAA);

  // Implicitly, write() already verifies via rawRead() internally. But we
  // explicitly read it back here to prove the public read() works after
  // write().
  uint8_t readbackValue = mip.eeprom.read(0);
  t_write = !mip.didLastCallFail() && (readbackValue == 0xAA);

  delay(500);

  // ---------------------------------------------------------
  // TEST 3: write() & read() (Zeroing/Clearing byte 0x00)
  // ---------------------------------------------------------
  mip.console.println(F("Test 3: Write 0x00 to offset 0 (Zeroing) and verify readback"));
  mip.eeprom.write(0, 0x00);

  readbackValue = mip.eeprom.read(0);
  t_write_zero = !mip.didLastCallFail() && (readbackValue == 0x00);

  delay(500);

  // ---------------------------------------------------------
  // CLEANUP: Restore Original State
  // ---------------------------------------------------------
  mip.console.println(F("Cleanup: Restoring original state to offset 0"));
  mip.eeprom.write(0, originalValue);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE
  // ---------------------------------------------------------
  mip.console.println();
  mip.console.println(F("=================================================="));
  mip.console.println(F(" MiP_EEPROM Exhaustive Test Summary"));
  mip.console.println(F("=================================================="));
  mip.console.println(F(" Method / Feature                 | Result"));
  mip.console.println(F("----------------------------------|---------------"));

  printTestResult("read()", t_read);
  printTestResult("write() (Test Byte)", t_write);
  printTestResult("write() (Zeroing)", t_write_zero);

  mip.console.println(F("=================================================="));
  mip.console.println(F("EEPROM.ino: Tests Complete."));
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
