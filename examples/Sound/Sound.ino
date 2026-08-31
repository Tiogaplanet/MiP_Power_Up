/**
 * @file Sound.ino
 * @brief Exhaustive test and demonstration of MiP's sound playback and volume
 * control.
 *
 * @details This sketch serves as both a tutorial for the user and an exhaustive
 * test suite for the MiP_Sound class. It systematically exercises every public
 * method available in the class. It verifies success by setting and reading
 * back the speaker volume, playing individual sounds, building a multi-sound
 * sequence, and verifying the class cleanup function.
 *
 * A summary table is printed to the console at the end of the automated tests.
 *
 * Exhaustively tests the following APIs:
 *   - sound.writeVolume(uint8_t volume)
 *   - sound.readVolume()
 *   - sound.play(MiPSoundIndex sound, MiPVolume volume)
 *   - sound.beginList()
 *   - sound.addEntryToList(MiPSoundIndex sound, uint16_t delayTime, MiPVolume
 * volume)
 *   - sound.playList(uint8_t repeatCount)
 *   - sound.end()
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
 * test of every MiP_Sound method, pausing between tests so the user can hear
 * the sound changes. Finally, it outputs a formatted summary table.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Sound.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("Sound.ino: Starting Exhaustive MiP_Sound Tests..."));
  mip.console.println();

  // Test tracking variables
  bool t_writeVolume = false;
  bool t_readVolume = false;
  bool t_play = false;
  bool t_soundList = false;
  bool t_end = false;

  // ---------------------------------------------------------
  // TEST 1: writeVolume() & readVolume()
  // ---------------------------------------------------------
  mip.console.println(F("Test 1: writeVolume(3) & readVolume()"));
  mip.sound.writeVolume(MIP_VOLUME_3);

  uint8_t currentVolume = mip.sound.readVolume();

  t_readVolume = !mip.didLastCallFail();
  t_writeVolume = (!mip.didLastCallFail() && currentVolume == MIP_VOLUME_3);

  mip.console.print(F("  -> Readback Volume: "));
  mip.console.println(currentVolume);
  delay(500);

  // ---------------------------------------------------------
  // TEST 2: play()
  // ---------------------------------------------------------
  mip.console.println(F("Test 2: play() - Playing a single drinking sound at volume 4"));
  mip.sound.play(MIP_SOUND_ACTION_DRINKING, MIP_VOLUME_4);
  t_play = !mip.didLastCallFail();

  // Allow time for the single sound to play completely
  delay(3000);

  // ---------------------------------------------------------
  // TEST 3: Sound List Building and Playback
  // ---------------------------------------------------------
  mip.console.println(F("Test 3: Sound List - Building and playing a 2-sound sequence"));

  mip.sound.beginList();

  // First entry: eating, 1000 ms delay before next entry, volume 4
  mip.sound.addEntryToList(MIP_SOUND_ACTION_EATING, 1000, MIP_VOLUME_4);

  // Second entry: burping, no delay after, louder volume 7
  mip.sound.addEntryToList(MIP_SOUND_ACTION_BURPING, 0, MIP_VOLUME_7);

  // Play the constructed list once (repeat count = 1)
  mip.sound.playList(1);

  t_soundList = !mip.didLastCallFail();

  // Wait long enough for the list to finish
  delay(6000);

  // ---------------------------------------------------------
  // TEST 4: end()
  // ---------------------------------------------------------
  mip.console.println(F("Test 4: end() - Restoring default volume"));
  mip.sound.end();

  // Verify end() successfully set the volume back to MIP_VOLUME_7
  currentVolume = mip.sound.readVolume();
  t_end = (!mip.didLastCallFail() && currentVolume == MIP_VOLUME_7);
  delay(500);

  // ---------------------------------------------------------
  // PRINT SUMMARY TABLE
  // ---------------------------------------------------------
  mip.console.println();
  mip.console.println(F("=================================================="));
  mip.console.println(F(" MiP_Sound Exhaustive Test Summary"));
  mip.console.println(F("=================================================="));
  mip.console.println(F(" Method / Feature                 | Result"));
  mip.console.println(F("----------------------------------|---------------"));

  printTestResult("writeVolume()", t_writeVolume);
  printTestResult("readVolume()", t_readVolume);
  printTestResult("play()", t_play);
  printTestResult("beginList/addEntry/playList", t_soundList);
  printTestResult("end() (Restore Volume)", t_end);

  mip.console.println(F("=================================================="));
  mip.console.println(F("Sound.ino: Tests Complete."));
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
