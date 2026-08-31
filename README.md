# MiP Power Up Library 🤖⚡

[![Arduino Compile](https://github.com/Tiogaplanet/MPU_D1_mini_lib/actions/workflows/arduino-compile.yml/badge.svg)](https://github.com/Tiogaplanet/MiP_Power_Up/actions)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)


A powerful, unified Arduino IDE library for controlling the **WowWee MiP**. 

This library abstracts MiP's low-level binary UART protocol into an easy-to-use C++ interface. It allows you to command MiP to drive, balance, play audio, animate his LEDs, and read his internal sensors (such as weight, radar, and orientation). 

Originally based on the excellent reverse-engineering work by Adam Green, this library has been completely refactored to support modern microcontrollers, unified debugging, and advanced networking features.

---

## 🚀 Supported Architectures

This library dynamically adapts to your hardware, compiling the exact features you need while hiding the underlying hardware complexity.

### 1. ESP8266 (e.g., Wemos D1 mini)
* **The Smart IoT Robot:** Unlocks exclusive network features!
* **Wi-Fi & OTA:** Connect MiP to your Wi-Fi network and flash new firmware Over-The-Air (OTA) without plugging in a USB cable.
* **Telnet Debugging:** Stream live logs and interact with MiP wirelessly via a built-in Telnet server.
* **Persistent Memory:** Read and write files to the onboard LittleFS flash memory.

### 2. AVR ATmega328P (e.g., 5V Arduino Pro Mini)
* **The Lightweight Classic:** Highly optimized for tight memory constraints.
* **Auto-Multiplexing:** The Pro Mini only has one hardware Serial port. This library automatically manages a digital pin (Pin 4) to toggle a hardware multiplexer, seamlessly sharing the single UART line between your PC console and MiP.

---

## 🛠️ Hardware Setup

To talk to MiP, you need to connect your microcontroller's hardware UART to MiP's internal UART test pads. 

**Important Voltage Note:** MiP's internal logic operates at roughly 3.0V. You should use a **3.3V microcontroller** (like the ESP8266, ESP32, or a 3.3V/8MHz Pro Mini) to communicate with him safely. 
* Connect **TX** on your microcontroller to **RX** on MiP.
* Connect **RX** on your microcontroller to **TX** on MiP.
* Connect **GND** on your microcontroller to **GND** on MiP.

*(For detailed teardown and soldering instructions, see the original [Hackaday project guides](https://hackaday.io/project/10001-mip-power-up)).*

---

## 🌟 Key Features

* 🏃 **Motion & Balance:** Command MiP to drive, turn, fall over, stand back up, or adjust his center of gravity.
* 💡 **LED Control:** Change his chest LED to any RGB color and trigger eye/head LED blinking animations.
* 🎵 **Audio Playback:** Trigger built-in sound effects, voice clips, and manage speaker volume.
* 📡 **Sensor Reading:** Read his proximity radar, payload weight sensor, internal odometer, and orientation state.
* 👏 **Event Detection:** Listen for claps, physical shakes, and hand-swipe gestures.
* 💻 **Universal Console:** Use `mip.console.println()` to easily print debug messages back to your PC, regardless of which board architecture you are using!

---

## 💻 "Hello MiP" Example

Here is a minimal example of how easy it is to wake MiP up, turn his chest green, and have him drive forward:

```cpp
#include <MiP_Power_Up.h>

MiP mip;

void setup() {
  // Connect to MiP's hardware UART
  if (!mip.begin()) {
    mip.console.println(F("Failed to connect to MiP!"));
    return;
  }
  
  mip.console.println(F("Successfully connected to MiP."));

  // Turn chest LED solid green
  mip.chestLED.write(0x00, 0xFF, 0x00);
  
  // Play a happy sound
  mip.sound.play(MIP_SOUND_MIP_YEEESSS);
  
  // Drive forward at half speed for 2 seconds
  mip.motion.driveForward(15, 2000);
}

void loop() {
  // Your amazing robot logic goes here!
}
```

---

## 📚 Documentation & API Reference

Comprehensive documentation for every class, function, enum, and constant is available on the GitHub Wiki!

👉 **[Read the Full API Reference & Wiki Here](https://github.com/Tiogaplanet/MiP_Power_Up/wiki)**

We provide **20+ example sketches** in the `examples/` folder. We highly recommend starting with `BareMinimumWiFi.ino` (for ESP8266) or `Status.ino` to understand the basics.

---

## 🤝 Contributing & Community

We love pull requests! Whether you are fixing a bug, adding ESP32 support, or writing new examples, your contributions are welcome. 
* Please read our [Contributing Guidelines](CONTRIBUTING.md) before submitting a PR.
* We expect all community members to follow our [Code of Conduct](CODE_OF_CONDUCT.md).

---

## 📝 Credits & License

* **Adam Green:** Original reverse engineering of the MiP UART protocol and creation of the base Arduino library.
* **Joao Lopes:** Original author of the `MiPDebug` telnet server architecture.
* **Samuel Trassare:** Maintainer, multi-architecture unification, and advanced API refactoring.
* **WowWee Group Limited:** Creators of the MiP robot. (This library is unofficial and not affiliated with WowWee).

**Licenses:** 
The primary library is licensed under the **Apache License 2.0**. The `MPU_Debug` subsystem is licensed under the **MIT License**. See [`LICENSE.txt`](LICENSE.txt) for full details.