/**
 * @file HeadLEDs.ino
 * @brief Exhaustive test and demonstration of MiP's head LED control.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_HeadLEDs class. It systematically exercises every
 * public method available in the class, verifying success by writing LED
 * patterns and reading them back.
 *
 * A summary table is printed to the console at the end of the automated tests.
 *
 * Exhaustively tests the following APIs:
 *   - headLEDs.write(led1, led2, led3, led4)
 *   - headLEDs.read(struct)
 *   - headLEDs.write(struct)
 *   - headLEDs.unverifiedWrite(led1, led2, led3, led4)
 *   - headLEDs.unverifiedWrite(struct)
 *   - MiPHeadLEDs::clear()
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
 * test of every HeadLEDs method, pausing between tests so the user can observe
 * MiP's eye patterns changing. Finally, it outputs a formatted summary table
 * and restores MiP's eyes to a normal fully-on state.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("HeadLEDs.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("HeadLEDs.ino: Starting Exhaustive MiP_HeadLEDs Tests..."));
  mip.console.println();

  // Test tracking variables
  bool t_write_enums = false;
  bool t_read = false;
  bool t_write_struct = false;
  bool t_unverified_enums = false;
  bool t_unverified_struct = false;
  bool t_struct_clear = false;

  MiPHeadLEDs readback;
  MiPHeadLEDs configStruct;

  // ---------------------------------------------------------
  // TEST 1: write(enums)
  // ---------------------------------------------------------
  mip.console.println(F("Test 1: write(enums) - Setting mixed eye pattern"));
  mip.headLEDs.write(MIP_HEAD_LED_ON, MIP_HEAD_LED_OFF, MIP_HEAD_LED_BLINK_SLOW, MIP_HEAD_LED_BLINK_FAST);
  t_write_enums = !mip.didLastCallFail();
  delay(2000);

  // ---------------------------------------------------------
  // TEST 2: read(struct)
  // ---------------------------------------------------------
  mip.console.println(F("Test 2: read(struct) - Verifying mixed pattern"));
  mip.headLEDs.read(readback);
  t_read = !mip.didLastCallFail() && (readback.led1 == MIP_HEAD_LED_ON)
           && (readback.led2 == MIP_HEAD_LED_OFF) && (readback.led3 == MIP_HEAD_LED_BLINK_SLOW)
           && (readback.led4 == MIP_HEAD_LED_BLINK_FAST);
  delay(500);

  // ---------------------------------------------------------
  // TEST 3: write(struct)
  // ---------------------------------------------------------
  mip.console.println(F("Test 3: write(struct) - Setting reverse mixed pattern"));
  configStruct.led1 = MIP_HEAD_LED_BLINK_FAST;
  configStruct.led2 = MIP_HEAD_LED_BLINK_SLOW;
  configStruct.led3 = MIP_HEAD_LED_OFF;
  configStruct.led4 = MIP_HEAD_LED_ON;
  mip.headLEDs.write(configStruct);
  t_write_struct = !mip.didLastCallFail();
  delay(2000);

  // ---------------------------------------------------------
  // TEST 4: unverifiedWrite(enums) & read()
  // ---------------------------------------------------------
  mip.console.println(F("Test 4: unverifiedWrite(enums) - Setting alternating pattern"));
  mip.headLEDs.unverifiedWrite(
    MIP_HEAD_LED_OFF, MIP_HEAD_LED_ON, MIP_HEAD_LED_OFF, MIP_HEAD_LED_ON);
  delay(100);  // Give MiP a moment to process the fire-and-forget command

  mip.headLEDs.read(readback);
  if (!mip.didLastCallFail() && readback.led1 == MIP_HEAD_LED_OFF
      && readback.led2 == MIP_HEAD_LED_ON) {
    t_unverified_enums = true;
  }
  delay(2000);

  // ---------------------------------------------------------
  // TEST 5: MiPHeadLEDs::clear()
  // ---------------------------------------------------------
  mip.console.println(F("Test 5: MiPHeadLEDs::clear()"));
  configStruct.clear();
  if (configStruct.led1 == MIP_HEAD_LED_OFF && configStruct.led2 == MIP_HEAD_LED_OFF
      && configStruct.led3 == MIP_HEAD_LED_OFF && configStruct.led4 == MIP_HEAD_LED_OFF) {
    t_struct_clear = true;
  }

  // ---------------------------------------------------------
  // TEST 6: unverifiedWrite(struct) & read()
  // ---------------------------------------------------------
  mip.console.println(
    F("Test 6: unverifiedWrite(struct) - Setting eyes OFF using cleared struct"));
  mip.headLEDs.unverifiedWrite(configStruct);  // configStruct is all OFF from
                                               // the clear() test
  delay(100);

  mip.headLEDs.read(readback);
  if (!mip.didLastCallFail() && readback.led1 == MIP_HEAD_LED_OFF
      && readback.led4 == MIP_HEAD_LED_OFF) {
    t_unverified_struct = true;
  }
  delay(2000);

  // ---------------------------------------------------------
  // RESTORE DEFAULT STATE
  // ---------------------------------------------------------
  mip.console.println(F("Restoring MiP's eyes to fully ON."));
  mip.headLEDs.write(MIP_HEAD_LED_ON, MIP_HEAD_LED_ON, MIP_HEAD_LED_ON, MIP_HEAD_LED_ON);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE
  // ---------------------------------------------------------
  mip.console.println();
  mip.console.println(F("=================================================="));
  mip.console.println(F(" MiP_HeadLEDs Exhaustive Test Summary"));
  mip.console.println(F("=================================================="));
  mip.console.println(F(" Method / Feature                 | Result"));
  mip.console.println(F("----------------------------------|---------------"));

  printTestResult("write(enums)", t_write_enums);
  printTestResult("read(struct)", t_read);
  printTestResult("write(struct)", t_write_struct);
  printTestResult("unverifiedWrite(enums)", t_unverified_enums);
  printTestResult("MiPHeadLEDs::clear()", t_struct_clear);
  printTestResult("unverifiedWrite(struct)", t_unverified_struct);

  mip.console.println(F("=================================================="));
  mip.console.println(F("HeadLEDs.ino: Tests Complete."));
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
