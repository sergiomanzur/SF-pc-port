# Syphon Filter Android Port

This directory contains the Android port for **Syphon Filter PC Recompilation**.

## Features
- **100% Feature Parity with PC**:
  - All original and enhanced gameplay mechanics, weapon systems, AI, and mission objectives.
  - Complete ACD (Agent Communication Device) in-game pause menu, mission status, maps, and weapons overview.
  - Retail cheats support (All Weapons & Infinite Ammo, Super 9mm One-Shot Kills, Stage Select, Weak Enemies, Movie Theater).
  - High-resolution character dossiers & mission archives.
  - Multi-language support (English & Russian VIT translation).
- **Modern Display & Graphics Settings**:
  - Adaptive Widescreen rendering and original 4:3 mode.
  - Frame rate options: 30 FPS (original PSX cadence), 60 FPS, 120 FPS, and Uncapped.
  - V-Sync toggle.
  - Texture filtering: Bilinear, Trilinear, Anisotropic, Nearest.
  - Volumetric lighting & atmospheric fog effects.
  - Custom enhanced high-resolution mission skyboxes.
- **Touch & Controller Input**:
  - Customizable On-Screen Virtual Touch Controller (D-Pad, Dual Analog Sticks, Action buttons $\Delta, \bigcirc, \times, \square$, Shoulder triggers L1/L2/R1/R2, Select & Start).
  - Haptic touch feedback.
  - Native physical Bluetooth & USB game controller support (PlayStation DualShock/DualSense, Xbox, generic HID gamepads).

## Building the Android APK

### Prerequisites
- Android Studio Iguana (2023.2.1) or newer / Command Line Tools.
- Android NDK (r25c or newer).
- CMake 3.24+.
- Java JDK 17+.

### Command Line Build
`ash
cd android
./gradlew assembleRelease
`
The resulting APK will be generated at:
ndroid/app/build/outputs/apk/release/app-release-unsigned.apk

### Loading Game ROM
Syphon Filter requires the original **Syphon Filter USA v1.1 (SCUS-94240)** disc image in BIN/CUE format.
On first launch, tap **Select CUE / BIN Disc Image** to browse and select your ROM via the Android Storage Access Framework.
