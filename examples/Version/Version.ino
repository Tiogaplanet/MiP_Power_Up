/**
 * @file Version.ino
 * @brief Exhaustive test and demonstration of MiP's version reporting.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Version class. It systematically exercises every public
 * method available in the class. It verifies success by reading MiP's software
 * and hardware version details, retrieving the library's version strings and
 * numbers, and confirming the clear functionality of the data structures.
 *
 * A summary table is printed to the console at the end of the automated tests.
 *
 * Exhaustively tests the following APIs:
 *   - version.readSoftware(struct)
 *   - version.readHardware(struct)
 *   - version.readMPUString()
 *   - version.readMPUNumber()
 *   - MiPSoftwareVersion::clear()
 *   - MiPHardwareInfo::clear()
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
  for (int i = 0; i < padding; i++) {
    mip.console.print(F(" "));
  }
  
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
 * test of every MiP_Version method. It outputs the retrieved version numbers 
 * so the user can inspect MiP's details, and finally prints a formatted summary table.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Version.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("Version.ino: Starting Exhaustive MiP_Version Tests..."));
  mip.console.println();

  // Test tracking variables
  bool t_readSoftware = false;
  bool t_readHardware = false;
  bool t_readMPUString = false;
  bool t_readMPUNumber = false;
  bool t_sw_clear = false;
  bool t_hw_clear = false;

  // ---------------------------------------------------------
  // TEST 1: readMPUString() & readMPUNumber()
  // ---------------------------------------------------------
  mip.console.println(F("Test 1: Library Version Information"));
  
  const char* libVersionString = mip.version.readMPUString();
  uint32_t libVersionNumber = mip.version.readMPUNumber();
  
  mip.console.print(F("  -> readMPUString() returned: "));
  mip.console.println(libVersionString);
  mip.console.print(F("  -> readMPUNumber() returned: "));
  mip.console.println(libVersionNumber);

  t_readMPUString = (libVersionString != nullptr && strlen(libVersionString) > 0);
  t_readMPUNumber = (libVersionNumber > 0);
  delay(500);

  // ---------------------------------------------------------
  // TEST 2: readSoftware()
  // ---------------------------------------------------------
  mip.console.println(F("Test 2: readSoftware()"));
  MiPSoftwareVersion softwareVersion;
  mip.version.readSoftware(softwareVersion);
  
  t_readSoftware = !mip.didLastCallFail();

  if (t_readSoftware) {
    mip.console.print(F("  -> MiP Software version: "));
    mip.console.print(softwareVersion.year);
    mip.console.print('-');
    if (softwareVersion.month < 10) mip.console.print('0');
    mip.console.print(softwareVersion.month);
    mip.console.print('-');
    if (softwareVersion.day < 10) mip.console.print('0');
    mip.console.print(softwareVersion.day);
    mip.console.print('.');
    mip.console.println(softwareVersion.uniqueVersion);
  }
  delay(500);

  // ---------------------------------------------------------
  // TEST 3: readHardware()
  // ---------------------------------------------------------
  mip.console.println(F("Test 3: readHardware()"));
  MiPHardwareInfo hardwareInfo;
  mip.version.readHardware(hardwareInfo);
  
  t_readHardware = !mip.didLastCallFail();

  if (t_readHardware) {
    mip.console.println(F("  -> MiP Hardware info:"));
    mip.console.print(F("     Voice chip version: "));
    mip.console.println(hardwareInfo.voiceChip);
    mip.console.print(F("     Hardware version: "));
    mip.console.println(hardwareInfo.hardware);
  }
  delay(500);

  // ---------------------------------------------------------
  // TEST 4: Data Struct clear() functions
  // ---------------------------------------------------------
  mip.console.println(F("Test 4: Struct clear() functions"));
  
  softwareVersion.clear();
  t_sw_clear = (softwareVersion.year == 0 && softwareVersion.month == 0 && 
                softwareVersion.day == 0 && softwareVersion.uniqueVersion == 0);
                
  hardwareInfo.clear();
  t_hw_clear = (hardwareInfo.voiceChip == 0 && hardwareInfo.hardware == 0);
  delay(500);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE
  // ---------------------------------------------------------
  mip.console.println();
  mip.console.println(F("=================================================="));
  mip.console.println(F(" MiP_Version Exhaustive Test Summary"));
  mip.console.println(F("=================================================="));
  mip.console.println(F(" Method / Feature                 | Result"));
  mip.console.println(F("----------------------------------|---------------"));
  
  printTestResult("readSoftware(struct)", t_readSoftware);
  printTestResult("readHardware(struct)", t_readHardware);
  printTestResult("readMPUString()", t_readMPUString);
  printTestResult("readMPUNumber()", t_readMPUNumber);
  printTestResult("MiPSoftwareVersion::clear()", t_sw_clear);
  printTestResult("MiPHardwareInfo::clear()", t_hw_clear);
  
  mip.console.println(F("=================================================="));
  mip.console.println(F("Version.ino: Tests Complete."));
}

/**
 * @brief Arduino loop function.
 *
 * @details This example performs all actions in setup() and does not require
 * repeated work in loop().
 */
void loop() {
  if (!connectResult) {
    return;
  }
}
