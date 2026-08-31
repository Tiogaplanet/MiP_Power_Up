/**
 * @file Clap.ino
 * @brief Exhaustive test and demonstration of MiP's clap event APIs.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Clap class. It systematically exercises every public
 * method available in the class, verifying success by writing configuration
 * values and reading them back, as well as testing the event queue's behavior
 * in an empty state.
 *
 * A summary table is printed to the console at the end of the automated tests.
 * After the tests complete, the sketch enters an interactive mode in loop()
 * where the user can test physical clap detection.
 *
 * Exhaustively tests the following APIs:
 *   - clap.enableEvents()
 *   - clap.disableEvents()
 *   - clap.areEventsEnabled()
 *   - clap.writeDelay(uint16_t delay)
 *   - clap.readDelay()
 *   - clap.availableEvents()
 *   - clap.readEvent()
 *   - MiPClapSettings::clear()
 *
 * @author Adam Green (Original Author)
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
 * test of every MiP_Clap method. It verifies disabled/enabled states, tests
 * delay configuration read/writes, tests empty queue responses, and outputs a
 * formatted summary table.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Clap.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("Clap.ino: Starting Exhaustive MiP_Clap Tests..."));
  mip.console.println();

  // Test tracking variables
  bool t_disableEvents = false;
  bool t_writeDelay = false;
  bool t_readDelay = false;
  bool t_enableEvents = false;
  bool t_areEventsEnabled = false;
  bool t_availableEvents = false;
  bool t_readEvent = false;
  bool t_settingsClear = false;

  // ---------------------------------------------------------
  // TEST 1: disableEvents() & areEventsEnabled() (False state)
  // ---------------------------------------------------------
  mip.console.println(F("Test 1: disableEvents() & areEventsEnabled()"));
  mip.clap.disableEvents();
  t_disableEvents = !mip.didLastCallFail();

  bool isEnabled = mip.clap.areEventsEnabled();
  bool t_are_false = !mip.didLastCallFail() && (isEnabled == false);
  delay(500);

  // ---------------------------------------------------------
  // TEST 2: writeDelay() & readDelay()
  // ---------------------------------------------------------
  mip.console.println(F("Test 2: writeDelay() & readDelay()"));
  mip.clap.writeDelay(501);
  t_writeDelay = !mip.didLastCallFail();

  uint16_t delayMs = mip.clap.readDelay();
  t_readDelay = !mip.didLastCallFail() && (delayMs == 501);
  delay(500);

  // ---------------------------------------------------------
  // TEST 3: enableEvents() & areEventsEnabled() (True state)
  // ---------------------------------------------------------
  mip.console.println(F("Test 3: enableEvents() & areEventsEnabled()"));
  mip.clap.enableEvents();
  t_enableEvents = !mip.didLastCallFail();

  isEnabled = mip.clap.areEventsEnabled();
  bool t_are_true = !mip.didLastCallFail() && (isEnabled == true);

  // areEventsEnabled() fully passes if it accurately reported both false and
  // true
  t_areEventsEnabled = (t_are_false && t_are_true);
  delay(500);

  // ---------------------------------------------------------
  // TEST 4: availableEvents() & readEvent() (Empty Queue)
  // ---------------------------------------------------------
  mip.console.println(F("Test 4: Queue functions (Empty State)"));

  // Drain any incidental claps that might have just happened
  while (mip.clap.availableEvents() > 0) { mip.clap.readEvent(); }

  uint8_t avail = mip.clap.availableEvents();
  t_availableEvents = !mip.didLastCallFail() && (avail == 0);

  uint8_t event = mip.clap.readEvent();
  // Expect an error because we tried to read from an empty queue
  t_readEvent = (mip.lastCallResult() == MiP::MIP_ERROR_NO_EVENT && event == 0);

  // ---------------------------------------------------------
  // TEST 5: MiPClapSettings::clear()
  // ---------------------------------------------------------
  mip.console.println(F("Test 5: MiPClapSettings::clear()"));
  MiPClapSettings settings;
  settings.enabled = MIP_CLAP_ENABLED;
  settings.delay = 1000;
  settings.clear();
  t_settingsClear = (settings.enabled == MIP_CLAP_DISABLED && settings.delay == 0);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE
  // ---------------------------------------------------------
  mip.console.println();
  mip.console.println(F("=================================================="));
  mip.console.println(F(" MiP_Clap Exhaustive Test Summary"));
  mip.console.println(F("=================================================="));
  mip.console.println(F(" Method / Feature                 | Result"));
  mip.console.println(F("----------------------------------|---------------"));

  printTestResult("enableEvents()", t_enableEvents);
  printTestResult("disableEvents()", t_disableEvents);
  printTestResult("areEventsEnabled()", t_areEventsEnabled);
  printTestResult("writeDelay(uint16_t)", t_writeDelay);
  printTestResult("readDelay()", t_readDelay);
  printTestResult("availableEvents() (Empty)", t_availableEvents);
  printTestResult("readEvent() (Empty)", t_readEvent);
  printTestResult("MiPClapSettings::clear()", t_settingsClear);

  mip.console.println(F("=================================================="));
  mip.console.println(F("Clap.ino: Tests Complete."));
  mip.console.println();
  mip.console.println(F("Now, try clapping to test the interactive queue!"));
}

/**
 * @brief Arduino loop function.
 *
 * @details Called repeatedly after setup() completes. This implementation
 * polls MiP for pending clap events. While clap.availableEvents() reports one
 * or more events, clap.readEvent() is called to retrieve the clap count for
 * each event and the result is printed to the console.
 */
void loop() {
  if (!connectResult) { return; }

  // Poll for available clap events interactively
  while (mip.clap.availableEvents() > 0) {
    uint8_t clapCount = mip.clap.readEvent();
    mip.console.print(F(" Detected "));
    mip.console.print(clapCount);
    mip.console.println(F(" clap(s)"));
  }

  // Yield control briefly to prevent watchdog reset triggers
  delay(10);
}
