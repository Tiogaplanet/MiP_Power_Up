/**
 * @file MPU_Debug.cpp
 * @brief Implements the debug functionality for the MiP library on
 * ESP8266/ESP32 targets.
 *
 * @author Joao Lopes (Original Author)
 * @author Samuel Trassare (Maintainer)
 * @copyright Copyright (C) 2018-2026 Samuel Trassare
 * (https://github.com/Tiogaplanet) Licensed under the MIT License
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * https://opensource.org/licenses/MIT
 */

#if defined(ESP8266) || defined(ESP32)

#include "MPU_Debug.h"
#include "MiP_Power_Up.h"

WiFiServer telnetServer(MiP_Debug::TELNET_PORT);
WiFiClient telnetClient;

void MiP_Debug::begin(const String& hostname, uint8_t startingDebugLevel) {
  telnetServer.begin();
  telnetServer.setNoDelay(true);

  m_bufferPrint.reserve(BUFFER_PRINT);

#ifdef CLIENT_BUFFERING
  m_bufferSend.reserve(MAX_SIZE_SEND);
#endif

  m_hostname = hostname;
  m_clientDebugLevel = startingDebugLevel;
  m_lastDebugLevel = startingDebugLevel;
}

void MiP_Debug::stop() {
  if (telnetClient && telnetClient.connected()) { telnetClient.stop(); }
  telnetServer.stop();
}

void MiP_Debug::handle() {
#ifdef ALPHA_VERSION
  static uint32_t lastTime = millis();
#endif

  if (m_clientDebugLevel == PROFILER) {
    if (millis() > m_levelProfilerDisable) {
      m_clientDebugLevel = m_levelBeforeProfiler;
      if (m_connected) {
        telnetClient.println(F("* Debug level profile is now inactive."));
      }
    }
  }

#ifdef ALPHA_VERSION
  if (m_autoLevelProfiler > 0 && m_clientDebugLevel != PROFILER) {
    uint32_t diff = (millis() - lastTime);

    if (diff >= m_autoLevelProfiler) {
      m_levelBeforeProfiler = m_clientDebugLevel;
      m_clientDebugLevel = PROFILER;
      m_levelProfilerDisable = millis() + 1000;
      if (m_connected) {
        telnetClient.printf(
          "* Debug level profile is now active - time between handles: %u\r\n", diff);
      }
    }
    lastTime = millis();
  }
#endif

  if (telnetServer.hasClient()) {
    if (telnetClient && telnetClient.connected()) {
      WiFiClient newClient = telnetServer.accept();
      if (newClient.remoteIP() == telnetClient.remoteIP()) {
        telnetClient.stop();
        telnetClient = newClient;
      } else {
        newClient.stop();
        return;
      }
    } else {
      telnetClient = telnetServer.accept();
    }

    if (!telnetClient) return;

    telnetClient.setNoDelay(true);
    telnetClient.flush();

    m_bufferPrint = "";
    m_lastTimeCommand = millis();
    m_command = "";
    m_lastCommand = "";
    m_lastTimePrint = millis();

    showHelp();

#ifdef CLIENT_BUFFERING
    m_bufferSend = "";
    m_sizeBufferSend = 0;
    m_lastTimeSend = millis();
#endif

    delay(100);
    while (telnetClient.available()) { telnetClient.read(); }
  }

  m_connected = (telnetClient && telnetClient.connected());

  if (m_connected) {
    char last = ' ';

    while (telnetClient.available()) {
      char character = telnetClient.read();

      if (isCRLF(character)) {
        if (!isCRLF(last)) {
          if (m_command.length() > 0) {
            m_lastCommand = m_command;
            processCommand();
          }
        }
        m_command = "";
      } else if (isPrintable(character)) {
        m_command.concat(character);
      }
      last = character;
    }

#ifdef CLIENT_BUFFERING
    if ((millis() - m_lastTimeSend) >= DELAY_TO_SEND || m_sizeBufferSend >= MAX_SIZE_SEND) {
      telnetClient.print(m_bufferSend);
      m_bufferSend = "";
      m_sizeBufferSend = 0;
      m_lastTimeSend = millis();
    }
#endif

    if (MAX_TIME_INACTIVE > 0) {
      if ((millis() - m_lastTimeCommand) > MAX_TIME_INACTIVE) {
        telnetClient.println(F("* Closing session due to inactivity."));
        telnetClient.stop();
        m_connected = false;
      }
    }
  }
}

void MiP_Debug::setSerialEnabled(bool enable) {
  m_serialEnabled = enable;
  m_showColors = false;
}

void MiP_Debug::setResetCmdEnabled(bool enable) {
  m_resetCommandEnabled = enable;
}

void MiP_Debug::showTime(bool show) {
  m_showTime = show;
}

void MiP_Debug::showProfiler(bool show, uint32_t minTime) {
  m_showProfiler = show;
  m_minTimeShowProfiler = minTime;
}

#ifdef ALPHA_VERSION
void MiP_Debug::autoProfilerLevel(uint32_t millisElapsed) {
  m_autoLevelProfiler = millisElapsed;
}
#endif

void MiP_Debug::showDebugLevel(bool show) {
  m_showDebugLevel = show;
}

void MiP_Debug::showColors(bool show) {
  m_showColors = (!m_serialEnabled) ? show : false;
}

bool MiP_Debug::isActive(uint8_t debugLevel) {
  bool ret = (debugLevel >= m_clientDebugLevel && (m_connected || m_serialEnabled));
  if (ret) { m_lastDebugLevel = debugLevel; }
  return ret;
}

void MiP_Debug::setHelpProjectsCmds(const String& help) {
  m_helpProjectCmds = help;
}

void MiP_Debug::setCallBackProjectCmds(void (*callback)()) {
  m_callbackProjectCmds = callback;
}

size_t MiP_Debug::write(const uint8_t* buffer, size_t size) {
  for (size_t i = 0; i < size; i++) { write(buffer[i]); }
  return size;
}

size_t MiP_Debug::write(uint8_t character) {
  uint32_t elapsed = 0;
  size_t ret = 0;

  if (m_newLine) {
    String show = "";

    if (m_showDebugLevel) {
      if (!m_showColors) {
        switch (m_lastDebugLevel) {
          case PROFILER: show = "P"; break;
          case VERBOSE: show = "v"; break;
          case DEBUG: show = "d"; break;
          case INFO: show = "i"; break;
          case WARNING: show = "w"; break;
          case ERROR: show = "e"; break;
        }
      } else {
        switch (m_lastDebugLevel) {
          case PROFILER: show = "P"; break;
          case VERBOSE: show = "v"; break;
          case DEBUG:
            show = COLOR_BACKGROUND_GREEN;
            show += "d";
            break;
          case INFO:
            show = COLOR_BACKGROUND_WHITE;
            show += "i";
            break;
          case WARNING:
            show = COLOR_BACKGROUND_YELLOW;
            show += "w";
            break;
          case ERROR:
            show = COLOR_BACKGROUND_RED;
            show += "e";
            break;
        }
        if (show.length() > 1) { show += COLOR_RESET; }
      }
    }

    if (m_showTime) {
      if (show != "") show += " ";
      show += "t:";
      show += millis();
      show += "ms";
    }

    if (m_showProfiler) {
      elapsed = (millis() - m_lastTimePrint);
      bool resetColors = false;
      if (show != "") show += " ";
      if (m_showColors) {
        if (elapsed >= 5000) {
          show += COLOR_BACKGROUND_RED;
          resetColors = true;
        } else if (elapsed >= 3000) {
          show += COLOR_BACKGROUND_MAGENTA;
          resetColors = true;
        } else if (elapsed >= 1000) {
          show += COLOR_BACKGROUND_YELLOW;
          resetColors = true;
        } else if (elapsed >= 250) {
          show += COLOR_BACKGROUND_CYAN;
          resetColors = true;
        }
      }
      show += "p:^";
      show += formatNumber(elapsed, 4);
      show += "ms";
      if (resetColors) { show += COLOR_RESET; }
      m_lastTimePrint = millis();
    }

    if (show != "") {
      String send = "(" + show + ") ";
      if (m_connected || m_serialEnabled) { m_bufferPrint = send; }
    }
    m_newLine = false;
  }

  bool doPrint = false;

  if (character == '\n') {
    m_bufferPrint += "\r";
    m_newLine = true;
    doPrint = true;
  } else if (m_bufferPrint.length() == BUFFER_PRINT) {
    doPrint = true;
  }

  m_bufferPrint += (char)character;

  if (doPrint) {
    bool noPrint = false;

    if (m_showProfiler && elapsed < m_minTimeShowProfiler) {
      noPrint = true;
    } else if (m_filterActive) {
      String aux = m_bufferPrint;
      aux.toLowerCase();
      if (aux.indexOf(m_filter) == -1) { noPrint = true; }
    }

    if (!noPrint) {
      if (m_connected) {
#ifndef CLIENT_BUFFERING
        telnetClient.print(m_bufferPrint);
#else
        uint8_t size = m_bufferPrint.length();
        if ((m_sizeBufferSend + size) >= MAX_SIZE_SEND) {
          telnetClient.print(m_bufferSend);
          m_bufferSend = "";
          m_sizeBufferSend = 0;
          m_lastTimeSend = millis();
        }

        m_bufferSend += m_bufferPrint;
        m_sizeBufferSend += size;

        if ((millis() - m_lastTimeSend) >= DELAY_TO_SEND) {
          telnetClient.print(m_bufferSend);
          m_bufferSend = "";
          m_sizeBufferSend = 0;
          m_lastTimeSend = millis();
        }
#endif
      }

      if (m_serialEnabled) { Serial1.print(m_bufferPrint); }
    }

    ret = m_bufferPrint.length();
    m_bufferPrint = "";
  }

  return ret;
}

String MiP_Debug::expand(const String& string) {
  String temp = string;
  temp.replace("\r", "\\r");
  temp.replace("\n", "\\n");
  return temp;
}

void MiP_Debug::showHelp() {
  String help = "";

  help += "*** Welcome to MiP's debug terminal. This is version ";
  help += MIP_POWER_UP_VERSION;  // Pull dynamically from MiP_Power_Up.h
  help += ".\r\n* Hostname: ";
  help += m_hostname;
  help += "\r\n* IP: ";
  help += WiFi.localIP().toString();
  help += "\r\n* MAC address: ";
  help += WiFi.macAddress();
  help += "\r\n* Free heap RAM: ";
  help += ESP.getFreeHeap();
  help += "\r\n******************************************************\r\n";
  help += "* Commands:\r\n";
  help += "    ? or help -> display these help commands\r\n";
  help += "    q -> quit (close this connection)\r\n";
  help += "    m -> display available memory\r\n";
  help += "    v -> set debug level to verbose\r\n";
  help += "    d -> set debug level to debug\r\n";
  help += "    i -> set debug level to info\r\n";
  help += "    w -> set debug level to warning\r\n";
  help += "    e -> set debug level to errors\r\n";
  help += "    l -> show debug level\r\n";
  help += "    t -> show time in milliseconds\r\n";
  help += "    profiler:\r\n";
  help += "      p      -> show time between actual and last message (in millis)\r\n";
  help += "      p min  -> show only if time is this minimal\r\n";
  help += "      P time -> set debug level to profiler\r\n";
#ifdef ALPHA_VERSION
  help += "      A time -> set auto debug level to profiler\r\n";
#endif
  help += "    c -> show colors\r\n";
  help += "    filter:\r\n";
  help += "          filter <string> -> show only debug messages containing this value\r\n";
  help += "          nofilter        -> disable the filter\r\n";
  help += "    cpu80  -> Set the ESP8266 CPU to 80 MHz\r\n";
  help += "    cpu160 -> Set the ESP8266 CPU to 160 MHz\r\n";
  if (m_resetCommandEnabled) {
    help += "    reset -> reset the MiP Power Up\r\n";
  }

  if (m_helpProjectCmds != "" && m_callbackProjectCmds) {
    help += "\r\n    * Project commands:\r\n";
    String show = "\r\n" + m_helpProjectCmds;
    show.replace("\n", "\n    ");
    help += show;
  }

  help += "\r\n* Please type the command and press enter to execute. (? or h for this help)\r\n***\r\n";

  telnetClient.print(help);
}

String MiP_Debug::getLastCommand() const {
  return m_lastCommand;
}

void MiP_Debug::clearLastCommand() {
  m_lastCommand = "";
}

void MiP_Debug::processCommand() {
  telnetClient.print(F("* Debug: Command received: "));
  telnetClient.println(m_command);

  String options = "";
  int pos = m_command.indexOf(' ');
  if (pos > 0) { options = m_command.substring(pos + 1); }

  m_lastTimeCommand = millis();

  if (m_command == "h" || m_command == "?" || m_command == "help") {
    showHelp();
  } else if (m_command == "q") {
    telnetClient.println(F("* Closing telnet connection ..."));
    telnetClient.stop();
  } else if (m_command == "m") {
    telnetClient.print(F("* Free heap RAM: "));
    telnetClient.println(ESP.getFreeHeap());
  } else if (m_command == "cpu80") {
    system_update_cpu_freq(80);
    telnetClient.println(F("ESP8266 CPU changed to 80 MHz"));
  } else if (m_command == "cpu160") {
    system_update_cpu_freq(160);
    telnetClient.println(F("ESP8266 CPU changed to 160 MHz"));
  } else if (m_command == "v") {
    m_clientDebugLevel = VERBOSE;
    telnetClient.println(F("* Debug level set to Verbose"));
  } else if (m_command == "d") {
    m_clientDebugLevel = DEBUG;
    telnetClient.println(F("* Debug level set to Debug"));
  } else if (m_command == "i") {
    m_clientDebugLevel = INFO;
    telnetClient.println(F("* Debug level set to Info"));
  } else if (m_command == "w") {
    m_clientDebugLevel = WARNING;
    telnetClient.println(F("* Debug level set to Warning"));
  } else if (m_command == "e") {
    m_clientDebugLevel = ERROR;
    telnetClient.println(F("* Debug level set to Error"));
  } else if (m_command == "l") {
    m_showDebugLevel = !m_showDebugLevel;
    telnetClient.printf("* Show debug level: %s\r\n", m_showDebugLevel ? "on" : "off");
  } else if (m_command == "t") {
    m_showTime = !m_showTime;
    telnetClient.printf("* Show time: %s\r\n", m_showTime ? "on" : "off");
  } else if (m_command == "p") {
    m_showProfiler = !m_showProfiler;
    m_minTimeShowProfiler = 0;
    telnetClient.printf("* Show profiler: %s\r\n", m_showProfiler ? "on" : "off");
  } else if (m_command.startsWith("p ")) {
    if (options.length() > 0) {
      int32_t aux = options.toInt();
      if (aux > 0) {
        m_showProfiler = true;
        m_minTimeShowProfiler = aux;
        telnetClient.printf("* Show profiler: on (with minimal time: %u)\r\n", m_minTimeShowProfiler);
      }
    }
  } else if (m_command == "P") {
    m_levelBeforeProfiler = m_clientDebugLevel;
    m_clientDebugLevel = PROFILER;

    if (!m_showProfiler) { m_showProfiler = true; }

    uint32_t duration = 1000;
    if (options.length() > 0) {
      int32_t aux = options.toInt();
      if (aux > 0) { duration = aux; }
    }
    m_levelProfilerDisable = millis() + duration;

    telnetClient.printf("* Debug level set to Profiler (disable in %u millis)\r\n", duration);
  } else if (m_command == "A") {
    m_autoLevelProfiler = 1000;
    if (options.length() > 0) {
      int32_t aux = options.toInt();
      if (aux > 0) { m_autoLevelProfiler = aux; }
    }
    telnetClient.printf(
      "* Auto profiler debug level active (time >= %u millis)\r\n", m_autoLevelProfiler);
  } else if (m_command == "c") {
    m_showColors = !m_showColors;
    telnetClient.printf("* Show colors: %s\r\n", m_showColors ? "on" : "off");
  } else if (m_command.startsWith("filter ") && options.length() > 0) {
    setFilter(options);
  } else if (m_command == "nofilter") {
    setNoFilter();
  } else if (m_command == "reset" && m_resetCommandEnabled) {
    telnetClient.println(
      F("* Reset...\r\n* Closing telnet connection...\r\n* Resetting the controller..."));
    telnetClient.stop();
    telnetServer.stop();
    delay(500);
    ESP.restart();
  } else {
    if (m_callbackProjectCmds) { m_callbackProjectCmds(); }
  }
}

void MiP_Debug::setFilter(const String& filter) {
  m_filter = filter;
  m_filter.toLowerCase();
  m_filterActive = true;

  telnetClient.print(F("* Debug: Filter active: "));
  telnetClient.println(m_filter);
}

void MiP_Debug::setNoFilter() {
  m_filter = "";
  m_filterActive = false;
  telnetClient.println(F("* Debug: Filter disabled"));
}

String MiP_Debug::formatNumber(uint32_t value, uint8_t size, char insert) {
  String ret = "";
  uint32_t limit = 10;

  for (uint8_t i = 1; i <= size; i++) {
    if (value < limit) {
      for (uint8_t j = (size - i); j > 0; j--) { ret += insert; }
      break;
    }
    limit *= 10;
  }

  ret += value;
  return ret;
}

bool MiP_Debug::isCRLF(char character) {
  return (character == '\r' || character == '\n');
}

#endif  // defined(ESP8266) || defined(ESP32)
