/**
 * @file Debug.ino
 * @brief Example sketch demonstrating telnet-based debug output for MiP.
 *
 * @details
 * This sketch shows how to initialize a MiP connection over WiFi on an ESP8266,
 * enable a telnet debug service (MiP_Debug), and emit debug messages at 
 * different verbosity levels. It also demonstrates enabling an OTA handler so
 * the device can be reprogrammed over the network while the telnet debug
 * service is running.
 *
 * Behavior summary:
 *   - Connect to WiFi using mip.wifi.begin(ssid, password, hostname).
 *   - Start the telnet debug server with debug.begin(hostname).
 *   - Periodically print example messages at multiple debug levels using the
 *     mDebug* macros (mDebugV, mDebugD, mDebugI, mDebugW, mDebugE).
 *   - Call ArduinoOTA.handle() in loop() to allow OTA updates.
 *   - Call debug.handle() in loop() to service telnet connections and commands.
 *
 * The example enables the telnet "reset" command via
 * debug.setResetCmdEnabled(true).
 *
 * API usage demonstrated:
 *   - MiP::wifi.begin(ssid, password, hostname)
 *   - MiP_Debug::begin(hostname)
 *   - MiP_Debug::setResetCmdEnabled()
 *   - MiP_Debug::handle()
 *   - ArduinoOTA::handle()
 *   - mDebug, mDebugV, mDebugD, mDebugI, mDebugW, mDebugE macros
 *
 * Notes:
 *   - Replace the ssid and password constants with your network credentials.
 *   - The hostname string is used for both the MiP connection and the telnet
 *     debug service; choose a unique name for each device on your network.
 *
 * @author Samuel Trassare (Original Author)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <MiP_Power_Up.h>
#include <MPU_Debug.h>

/**
 * @brief WiFi SSID to join.
 *
 * @note Replace the placeholder with your network SSID before uploading.
 */
const char *ssid = "..............";

/**
 * @brief Wi‑Fi password for the SSID.
 *
 * @note Replace the placeholder with your network password before uploading.
 */
const char *password = "..............";

/**
 * @brief Hostname used for MiP connection and telnet debug service.
 *
 * @details This name appears in the network and is used by the telnet debug
 * server to identify the device. Choose a unique hostname for each device.
 */
const char *hostname = "MiP-Debugger";

/**
 * @brief Global MiP instance used to communicate with MiP.
 *
 * @details The mip object is used to establish the network connection and
 * to integrate MiP-specific functionality with the telnet debug service.
 */
MiP mip;

/**
 * @brief Tracks whether the initial connection to MiP succeeded.
 */
bool connectResult;

/**
 * @brief Telnet debug helper instance.
 *
 * @details MiP_Debug provides a telnet server that receives debug messages and
 * accepts simple commands (for example, a reset command when enabled).
 */
MiP_Debug debug;

/**
 * @brief Timestamp of the last periodic debug emission in milliseconds.
 *
 * @details Used to implement a non-blocking periodic message emission in
 * loop().
 */
uint32_t lastTimeCheck = 0;

/**
 * @brief Period (milliseconds) between periodic debug message bursts.
 *
 * @details The example uses a relatively long period to give the user time to
 * connect a telnet client and observe the messages.
 */
const uint32_t period = 30000;

/**
 * @brief One-shot flag to emit an initial set of example messages once.
 *
 * @details When true the sketch prints a set of example messages at various
 * debug levels to demonstrate the telnet debug facility. After the first burst
 * this flag is cleared so the messages are not repeated.
 */
bool runOnce = true;

/**
 * @brief Arduino setup function.
 *
 * @details
 * - Attempts to initialize communication with MiP via mip.begin().
 * - Attempts to connect to WiFi using mip.wifi.begin(ssid, password,
 * hostname).
 * - If either connection fails, prints an error to mip.console and returns early.
 * - Starts the telnet debug server via debug.begin(hostname).
 * - Enables the telnet reset command with debug.setResetCmdEnabled(true).
 * - Prints the device IP address and a short banner to mip.console.
 * - Initializes lastTimeCheck for the periodic loop behavior.
 */
void setup() {
  connectResult = mip.begin();
  if (!connectResult) {
    mip.console.println(F("TelnetDebug.ino: Failed connecting to MiP."));
    return;
  }

  uint8_t wifiStatus = mip.wifi.begin(ssid, password, hostname);
  if (wifiStatus != WL_CONNECTED) {
    mip.console.println(F("TelnetDebug.ino: Failed connecting to WiFi."));
    connectResult = false;
    return;
  }

  // Start telnet debug server and enable the reset command.
  debug.begin(hostname);
  debug.setResetCmdEnabled(true);

  mip.console.println(F("TelnetDebug.ino: Explore the different telnet debug levels."));
  mip.console.println();
  mip.console.print(F(" IP address: "));
  mip.console.println(WiFi.localIP());
  mip.console.println(F(" Use serial debugging in setup()."));

  lastTimeCheck = millis();
}

/**
 * @brief Arduino loop function.
 *
 * @details
 * - Calls ArduinoOTA.handle() to allow over-the-air programming while running.
 * - Periodically emits example debug messages at multiple verbosity levels
 *   using the mDebug* macros. The first period emits several formatted
 *   example lines to demonstrate formatting and level filtering.
 * - Calls debug.handle() to service telnet connections and process telnet
 *   commands (including the optional reset command).
 *
 * The periodic emission uses a non-blocking timing check so the loop remains
 * responsive to telnet and OTA events.
 */
void loop() {
  // Exit immediately if connecting to MiP or WiFi failed during setup()
  if (!connectResult) { return; }

  // Required for OTA programming to function while the sketch runs.
  ArduinoOTA.handle();

  uint32_t now = millis();

  // Emit example messages periodically so a telnet client can observe them.
  if (now - lastTimeCheck >= period) {
    if (runOnce) {
      // Demonstrate formatted output and that mDebug messages always print to
      // telnet.
      mDebug("The telnet debug utility is very helpful. It can selectively "
             "print messages of different levels.\n");
      mDebug("Messages at the mDebug level always print to telnet.\n");
      mDebug("All debugging messages can use formatted %s.\n", "output");

      // Emit one example of each debug level.
      mDebugV("This is a verbose message.\n");
      mDebugD("This is a debug message.\n");
      mDebugI("This is an informational message.\n");
      mDebugW("This is a warning message.\n");
      mDebugE("This is an error message.\n");

      runOnce = false;
    }

    // Repeated periodic messages at each level to illustrate filtering.
    mDebugV("* This is a message of debug level VERBOSE\n");
    mDebugD("* This is a message of debug level DEBUG\n");
    mDebugI("* This is a message of debug level INFO\n");
    mDebugW("* This is a message of debug level WARNING\n");
    mDebugE("* This is a message of debug level ERROR\n");

    lastTimeCheck = now;
  }

  // Service the telnet debug server (must be called frequently).
  debug.handle();
}
