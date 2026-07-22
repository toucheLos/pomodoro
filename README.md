# Pomodoro

A tiny floating Pomodoro timer widget. Written in C++ with Qt6 Widgets for the
UI and [miniaudio](https://github.com/mackron/miniaudio) (a single vendored
header, no extra system dependencies) for sound. Runs on Linux, Windows, and
macOS.

## Features

- **Floating mini widget** — frameless, always-on-top, draggable. Its position
  is remembered between runs.
- **Classic auto-advance flow** — 25 min work / 5 min short break / 15 min long
  break every 4 pomodoros. All durations are customizable.
- **Soothing sounds** — a chime plays when a phase ends, and a distinct sound
  plays when the next phase starts (the "resume" alert). Five built-in voices are
  generated procedurally on first run:
  `chime`, `soft-gong`, `arpeggio`, `birds`, `ding`.
- **Bring your own sounds** — drop any `.wav` / `.mp3` / `.flac` / `.ogg` into
  `~/.config/pomodoro/sounds/` and pick them in Settings (each has a Preview
  button). On Windows/macOS the folder is the platform config location.
- **Controls** — Start/Pause (▶/⏸), Skip to next phase (⏭), Reset (↺),
  Settings (⚙). Only one instance runs at a time.

## Build

Requires CMake ≥ 3.16, a C++17 compiler, and Qt6 Widgets.

On Debian/Ubuntu:

```bash
sudo apt install cmake g++ qt6-base-dev
```

Then:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The binary is at `build/pomodoro`.

## Run

```bash
./build/pomodoro
```

To launch it as `pomodoro` from anywhere, either install it:

```bash
cmake --install build --prefix ~/.local   # installs to ~/.local/bin/pomodoro
```

(ensure `~/.local/bin` is on your `PATH`), or add your own alias to
`build/pomodoro`.

## Configuration

Settings (durations, sounds, volume, auto-advance) are edited in the ⚙ dialog
and stored via Qt's cross-platform settings — on Linux at
`~/.config/pomodoro/pomodoro.conf`. Generated and user sounds live in
`~/.config/pomodoro/sounds/`.

## Layout

```
CMakeLists.txt          Build (Qt6 Widgets + vendored miniaudio)
third_party/miniaudio.h Vendored audio backend (public domain / MIT-0)
src/
  main.cpp              App entry, single-instance guard, first-run setup
  PomodoroEngine.*      Phase state machine + drift-free countdown
  PomodoroWidget.*      The floating frameless widget
  SettingsDialog.*      Durations + sound pickers with Preview
  AudioPlayer.*         miniaudio wrapper (non-blocking playback)
  SoundGenerator.*      Synthesizes the built-in WAV sounds
  Config.*              QSettings-backed configuration
```
# pomodoro
