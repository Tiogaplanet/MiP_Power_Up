/**
 * @file Mode.ino
 * @brief Exhaustive test and demonstration of MiP's operating modes.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Mode class. It systematically exercises every public
 * method available in the class. It verifies success by setting MiP into a
 * specific game/app mode, and then reading back the status to confirm the
 * mode change was accepted.
 *
 * A summary table is printed to the console at the end of the automated tests.
 *
 * Exhaustively tests the following APIs:
 *   - mode.enableCage()  / mode.isCageEnabled()
 *   - mode.enableDance() / mode.isDanceEnabled()
 *   - mode.enableStack() / mode.isStackEnabled()
 *   - mode.enableTrick() / mode.isTrickEnabled()
 *   - mode.enableRoam()  / mode.isRoamEnabled()
 *   - mode.enableApp()   / mode.isAppEnabled()
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
 * test of every MiP_Mode method. It leaves a short delay between tests so the
 * user can hear MiP's audio cues as he changes modes. Finally, it outputs a
 * formatted summary table and restores MiP to App Mode (the default).
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Mode.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("Mode.ino: Starting Exhaustive MiP_Mode Tests..."));
  mip.console.println();

  // Reset MiP's volume to default so the user can hear him cycling through the
  // modes.
  mip.sound.writeVolume(MIP_VOLUME_7);
  delay(500);

  // Test tracking variables
  bool t_cage = false;
  bool t_dance = false;
  bool t_stack = false;
  bool t_trick = false;
  bool t_roam = false;
  bool t_app = false;

  // ---------------------------------------------------------
  // TEST 1: Cage Mode
  // ---------------------------------------------------------
  mip.console.println(F("Test 1: enableCage() & isCageEnabled()"));
  mip.mode.enableCage();
  bool setSuccess = !mip.didLastCallFail();
  bool checkSuccess = mip.mode.isCageEnabled() && !mip.didLastCallFail();
  t_cage = (setSuccess && checkSuccess);
  delay(3000);

  // ---------------------------------------------------------
  // TEST 2: Dance Mode
  // ---------------------------------------------------------
  mip.console.println(F("Test 2: enableDance() & isDanceEnabled()"));
  mip.mode.enableDance();
  setSuccess = !mip.didLastCallFail();
  checkSuccess = mip.mode.isDanceEnabled() && !mip.didLastCallFail();
  t_dance = (setSuccess && checkSuccess);
  delay(3000);

  // ---------------------------------------------------------
  // TEST 3: Stack Mode
  // ---------------------------------------------------------
  mip.console.println(F("Test 3: enableStack() & isStackEnabled()"));
  mip.mode.enableStack();
  setSuccess = !mip.didLastCallFail();
  checkSuccess = mip.mode.isStackEnabled() && !mip.didLastCallFail();
  t_stack = (setSuccess && checkSuccess);
  delay(3000);

  // ---------------------------------------------------------
  // TEST 4: Trick Mode
  // ---------------------------------------------------------
  mip.console.println(F("Test 4: enableTrick() & isTrickEnabled()"));
  mip.mode.enableTrick();
  setSuccess = !mip.didLastCallFail();
  checkSuccess = mip.mode.isTrickEnabled() && !mip.didLastCallFail();
  t_trick = (setSuccess && checkSuccess);
  delay(3000);

  // ---------------------------------------------------------
  // TEST 5: Roam Mode
  // ---------------------------------------------------------
  mip.console.println(F("Test 5: enableRoam() & isRoamEnabled()"));
  mip.mode.enableRoam();
  setSuccess = !mip.didLastCallFail();
  checkSuccess = mip.mode.isRoamEnabled() && !mip.didLastCallFail();
  t_roam = (setSuccess && checkSuccess);
  delay(3000);

  // ---------------------------------------------------------
  // TEST 6: App Mode (Default State)
  // ---------------------------------------------------------
  mip.console.println(F("Test 6: enableApp() & isAppEnabled()"));
  mip.mode.enableApp();
  setSuccess = !mip.didLastCallFail();
  checkSuccess = mip.mode.isAppEnabled() && !mip.didLastCallFail();
  t_app = (setSuccess && checkSuccess);
  delay(1000);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE
  // ---------------------------------------------------------
  mip.console.println();
  mip.console.println(F("=================================================="));
  mip.console.println(F(" MiP_Mode Exhaustive Test Summary"));
  mip.console.println(F("=================================================="));
  mip.console.println(F(" Method / Feature                 | Result"));
  mip.console.println(F("----------------------------------|---------------"));

  printTestResult("Cage Mode", t_cage);
  printTestResult("Dance Mode", t_dance);
  printTestResult("Stack Mode", t_stack);
  printTestResult("Trick Mode", t_trick);
  printTestResult("Roam Mode", t_roam);
  printTestResult("App Mode", t_app);

  mip.console.println(F("=================================================="));
  mip.console.println(F("Mode.ino: Tests Complete."));
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
