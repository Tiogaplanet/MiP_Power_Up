/**
 * @file Shake.ino
 * @brief Example sketch demonstrating MiP shake detection.
 *
 * @details This simple example initializes communication with MiP and
 * continuously polls for shake events. When MiP detects a shake, the sketch
 * prints a notification to mip.console. The sketch demonstrates basic use of the
 * MiP API for initialization and the shake-detection query:
 *   - begin()
 *   - shake.read()
 *
 * Typical usage:
 *   - Load this sketch onto the MPU-D1 mini connected to MiP Power Up.
 *   - Open mip.console to observe "Shake detected!" messages when MiP is shaken.
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
 *
 * @details Use this object to call MiP API functions such as begin() and
 * shake.read(). Keeping the instance at file scope makes it available
 * throughout setup() and loop().
 */
MiP mip;

/**
 * @brief Tracks whether the initial connection to MiP succeeded.
 *
 * @details Stored so other parts of the sketch can check connection state
 * if extended.
 */
bool connectResult;

/**
 * @brief Arduino setup function.
 *
 * @details Initializes the MiP connection by calling mip.begin(). If the
 * connection fails, an error message is printed to mip.console and the sketch
 * returns early. On success, a short description is printed to mip.console to
 * indicate the sketch is ready to detect shakes.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("Shake.ino: Failed connecting to MiP."));
    return;
  }

  mip.console.println(F("Shake.ino: Detect shakes."));
}

/**
 * @brief Arduino loop function.
 *
 * @details Continuously polls MiP for shake events using shake.read().
 * When a shake is detected, the sketch prints "Shake detected!" to mip.console.
 * This loop is intentionally minimal to keep the example focused on the
 * shake-detection API.
 */
void loop() {
  // Exit immediately if connecting to MiP failed during setup()
  if (!connectResult) { return; }

  if (mip.shake.read()) { mip.console.println(F(" Shake detected!")); }

  // Yield CPU control briefly to keep background tasks responsive
  delay(10);
}
