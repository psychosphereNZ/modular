# IRON MODULAR — VST3 instrument

A JUCE/C++ starter project for a dark industrial-style modular-inspired software synthesizer.

## Features in this first build
- VST3 instrument and standalone target
- MIDI input, 12-voice allocation
- Two oscillators (saw, square, triangle, sine), sub oscillator and noise
- ADSR amplitude envelope
- LFO pitch and filter modulation
- Low-pass filter core, resonance/drive controls
- Delay and reverb
- Parameter state saved with the DAW project
- Dark industrial front panel

## Important scope note
This is a working first-generation synth engine and front-panel prototype, not yet a fully cable-patchable Eurorack environment. The UI labels the patch-bay expansion point. A true drag-and-drop CV/audio patch graph, additional filter modes, independent per-voice stereo filtering, preset browser and oversampling are planned for subsequent iterations.

## Build on Windows 11

### Requirements
1. Visual Studio 2022 with **Desktop development with C++**
2. CMake 3.22 or newer
3. Git
4. Internet connection for the first configure step (CMake downloads JUCE 8.0.6)

### Build
Open **x64 Native Tools Command Prompt for VS 2022**, then:

```bat
cd path\to\IRON_MODULAR
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The VST3 bundle is normally produced under:

```text
build\IRON_MODULAR_artefacts\Release\VST3\IRON MODULAR.vst3
```

Copy the `.vst3` bundle to:

```text
C:\Program Files\Common Files\VST3\
```

Then restart Ableton Live and enable VST3 plug-ins in Preferences > Plug-Ins.

## Ableton Live
- Track type: MIDI track
- Load `IRON MODULAR` from the Plug-ins browser
- Arm the track or draw MIDI notes
- Start with OSC 1 LEVEL at 0.75 and MASTER at 0.8

## Notes
The source is provided for building and extending. The VST3 binary is not precompiled in this package; it must be built on Windows with the tools above.
