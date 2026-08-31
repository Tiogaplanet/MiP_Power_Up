/**
 * @file ChestLED.ino
 * @brief Exhaustive test and demonstration of MiP's chest LED operations.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_ChestLED class. It systematically exercises every
 * public method available in the class, verifying success either via the
 * library's built-in verification or through manual read-backs.
 *
 * A summary table is printed to the console at the end of the test.
 *
 * Exhaustively tests the following APIs:
 *   - chestLED.write(r, g, b)
 *   - chestLED.write(r, g, b, onTime, offTime)
 *   - chestLED.write(struct)
 *   - chestLED.unverifiedWrite(r, g, b)
 *   - chestLED.unverifiedWrite(r, g, b, onTime, offTime)
 *   - chestLED.unverifiedWrite(struct)
 *   - chestLED.read()
 *   - MiPChestLED::clear()
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
 * test of every ChestLED method, pausing between tests so the user can observe
 * the LED changes. Finally, it outputs a formatted summary table.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("ChestLED.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("ChestLED.ino: Starting Exhaustive MiP_ChestLED Tests..."));
  mip.console.println();

  // Test tracking variables
  bool t_write_rgb = false;
  bool t_write_rgb_flash = false;
  bool t_write_struct = false;
  bool t_unverified_rgb = false;
  bool t_unverified_rgb_flash = false;
  bool t_unverified_struct = false;
  bool t_read = false;
  bool t_struct_clear = false;

  MiPChestLED readback;

  // ---------------------------------------------------------
  // TEST 1: write(r, g, b)
  // ---------------------------------------------------------
  mip.console.println(F("Test 1: Verified solid color (Magenta)"));
  mip.chestLED.write(0xFF, 0x00, 0xFF);
  t_write_rgb = !mip.didLastCallFail();
  delay(1500);

  // ---------------------------------------------------------
  // TEST 2: write(r, g, b, onTime, offTime)
  // ---------------------------------------------------------
  mip.console.println(F("Test 2: Verified flashing color (Red)"));
  mip.chestLED.write(0xFF, 0x00, 0x00, 500, 500);
  t_write_rgb_flash = !mip.didLastCallFail();
  delay(3000);

  // ---------------------------------------------------------
  // TEST 3: write(const MiPChestLED&)
  // ---------------------------------------------------------
  mip.console.println(F("Test 3: Verified struct configuration (Blue)"));
  MiPChestLED configStruct;
  configStruct.red = 0x00;
  configStruct.green = 0x00;
  configStruct.blue = 0xFC;  // Hardware truncates bottom 2 bits, use 0xFC
  configStruct.onTime = 0;
  configStruct.offTime = 0;

  mip.chestLED.write(configStruct);
  t_write_struct = !mip.didLastCallFail();
  delay(1500);

  // ---------------------------------------------------------
  // TEST 4: unverifiedWrite(r, g, b) & read()
  // ---------------------------------------------------------
  mip.console.println(F("Test 4: Unverified solid color (Yellow) & read()"));
  mip.chestLED.unverifiedWrite(0xFF, 0xFF, 0x00);
  delay(100);  // Give MiP a moment to process the fire-and-forget command

  mip.chestLED.read(readback);
  t_read = !mip.didLastCallFail();

  // Verify it actually worked by checking our manual readback
  if (t_read && readback.red == 0xFF && readback.green == 0xFF && readback.blue == 0x00) {
    t_unverified_rgb = true;
  }
  delay(1500);

  // ---------------------------------------------------------
  // TEST 5: unverifiedWrite(r, g, b, onTime, offTime)
  // ---------------------------------------------------------
  mip.console.println(F("Test 5: Unverified flashing color (Cyan)"));
  mip.chestLED.unverifiedWrite(0x00, 0xFF, 0xFC, 200, 200);
  delay(100);

  mip.chestLED.read(readback);
  if (!mip.didLastCallFail() && readback.green == 0xFF && readback.onTime >= 180) {
    t_unverified_rgb_flash = true;
  }
  delay(3000);

  // ---------------------------------------------------------
  // TEST 6: unverifiedWrite(const MiPChestLED&) & MiPChestLED::clear()
  // ---------------------------------------------------------
  mip.console.println(F("Test 6: Unverified struct config (Green) & struct clear()"));

  // Test clear() functionality on the struct
  configStruct.clear();
  if (configStruct.red == 0 && configStruct.green == 0 && configStruct.onTime == 0) {
    t_struct_clear = true;
  }

  // Load new values to test the unverified struct write
  configStruct.green = 0xFF;
  mip.chestLED.unverifiedWrite(configStruct);
  delay(100);

  mip.chestLED.read(readback);
  if (!mip.didLastCallFail() && readback.green == 0xFF && readback.red == 0x00) {
    t_unverified_struct = true;
  }
  delay(1500);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE
  // ---------------------------------------------------------
  mip.console.println();
  mip.console.println(F("=================================================="));
  mip.console.println(F(" MiP_ChestLED Exhaustive Test Summary"));
  mip.console.println(F("=================================================="));
  mip.console.println(F(" Method / Feature                 | Result"));
  mip.console.println(F("----------------------------------|---------------"));

  printTestResult("write(r, g, b)", t_write_rgb);
  printTestResult("write(r, g, b, on, off)", t_write_rgb_flash);
  printTestResult("write(struct)", t_write_struct);
  printTestResult("unverifiedWrite(r, g, b)", t_unverified_rgb);
  printTestResult("unverifiedWrite(r, g, b, on, off)", t_unverified_rgb_flash);
  printTestResult("unverifiedWrite(struct)", t_unverified_struct);
  printTestResult("read(struct)", t_read);
  printTestResult("MiPChestLED::clear()", t_struct_clear);

  mip.console.println(F("=================================================="));
  mip.console.println(F("ChestLED.ino: Done."));
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
