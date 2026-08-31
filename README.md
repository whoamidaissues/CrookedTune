# CrookedTune

CrookedTune is an educational real-time pitch-correction audio plug-in built with
JUCE and C++20. It detects the fundamental frequency of a monophonic voice,
quantizes it to a selected musical key and scale, and applies the calculated
pitch correction with an adjustable response.

The project currently builds as VST3 and Standalone on supported desktop
platforms. An Audio Unit target is also built on macOS.

## Features

- monophonic pitch detection with frequency, note, cents, and confidence display;
- chromatic, major, and natural minor pitch quantization in all 12 keys;
- adjustable correction Strength and Retune time;
- real-time pitch shifting using Signalsmith Stretch;
- formant-preserving pitch processing;
- latency-compensated Dry/Wet Mix and correction Bypass;
- soft-saturation Drive and output Gain;
- parameter automation and state restoration through JUCE APVTS;
- resizable graphical interface.

## Audio pipeline

```text
Input -> Pitch Detector -> Quantizer -> Retune Controller
  |                                      |
  |                                      v
  +-> delayed dry signal          Signalsmith Pitch Shifter
             |                           |
             +--------- Dry/Wet Mix <----+
                              |
                       Distortion -> Gain -> Output
```

Pitch detection analyzes the clean input before pitch shifting, distortion, and
gain. The dry path is delayed by the pitch shifter's latency before mixing, which
keeps the dry and processed signals synchronized.

## Requirements

- Git;
- CMake 3.24 or newer;
- a compiler with C++20 support;
- macOS: Xcode command-line tools or Xcode;
- Windows: Visual Studio 2022 with **Desktop development with C++**.

JUCE, Signalsmith Stretch, and Signalsmith Linear are included as Git
submodules. Clone the repository with its submodules:

```bash
git clone --recurse-submodules https://github.com/whoamidaisissues/CrookedTune.git
cd CrookedTune
```

If the repository was cloned without them, run:

```bash
git submodule update --init --recursive
```

## Building on macOS

Configure and build the Release version:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
```

Build artifacts are created in:

```text
build-release/CrookedTune_artefacts/Release/
```

The VST3 plug-in can be copied to:

```text
~/Library/Audio/Plug-Ins/VST3/
```

The Audio Unit component can be copied to:

```text
~/Library/Audio/Plug-Ins/Components/
```

The macOS build has been tested in FL Studio. It is configured as a Universal
Binary for Apple Silicon and Intel Macs.

## Building on Windows

Open PowerShell or the Visual Studio Developer PowerShell in the repository and
run:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The VST3 artifact should be created under:

```text
build\CrookedTune_artefacts\Release\VST3\CrookedTune.vst3
```

Install it in the standard system VST3 directory:

```text
C:\Program Files\Common Files\VST3
```

Then rescan plug-ins in the DAW. The Windows configuration is provided by the
cross-platform JUCE/CMake project, but it has not yet been tested on a Windows
machine.

## Controls

- **Key** and **Scale** select the allowed target notes.
- **Strength** controls how much of the calculated correction is applied.
- **Retune** controls how quickly the correction reaches its target.
- **Mix** blends the latency-aligned original and pitch-corrected signals.
- **Bypass** bypasses pitch correction while preserving its compensated latency.
- **Drive** adds soft saturation after the Dry/Wet stage.
- **Gain** controls the final output level.

## Project structure

```text
src/PluginProcessor.*       plug-in lifecycle, parameters, and DSP orchestration
src/PluginEditor.*          graphical interface
src/DSP/                    audio-processing modules
src/Music/                  pitch conversion, quantization, and retune logic
external/                   Signalsmith Git submodules
JUCE/                       JUCE Git submodule
```

## Project status

This repository contains the first working MVP created as an educational DSP and
audio plug-in project. The current macOS Release build has been tested with both
oscillator signals and recorded male vocals. Future work may include the
CrookedTune Scene granular layer, alternative vocal pitch-shifting algorithms,
presets, and an updated visual design.

## Third-party software

This project uses JUCE, Signalsmith Stretch, and Signalsmith Linear. Their source
code remains in separate Git submodules and is subject to each project's own
license. Review those licenses before redistributing or publishing binary builds.
