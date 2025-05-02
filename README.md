# 🛰 Mission Control

A real-time rover monitoring dashboard built with **Qt (C++)**.
Dark-themed, glowing trail, live telemetry, and a Python simulator that makes
the rover wander around an arena on its own.

```
┌─────────────────────────────────────────────────────┐
│  🛰  MISSION CONTROL                    ● CONNECTED  │
├──────────────────────────┬──────────────────────────┤
│                          │  TELEMETRY               │
│   [ 2D MAP — glowing ]   │  STATUS    EXPLORING     │
│                          │  HEADING   037.5 °       │
│    cyan trail…           │  UPTIME    00:04:22      │
│         △ rover          │  BATTERY   ████████  87% │
│                          │  SPEED     █████░░   3.1 │
│                          ├──────────────────────────┤
│                          │  EVENT LOG               │
│                          │  12:01 ⚠️ Obstacle…      │
│                          │  12:00 ✅ Waypoint hit    │
├──────────────────────────┴──────────────────────────┤
│  ▶ START / AUTO   ⏸ STOP              🔴 ABORT       │
└─────────────────────────────────────────────────────┘
```

---

## Prerequisites

| Tool | Version | Install |
|------|---------|---------|
| Qt   | 5.15 or 6.x | https://www.qt.io/download-open-source |
| CMake | ≥ 3.16 | https://cmake.org/download |
| Python | ≥ 3.8 | https://www.python.org |
| C++ compiler | GCC / Clang / MSVC | via your OS package manager |

> **Linux (Ubuntu/Debian)**
> ```bash
> sudo apt install qt6-base-dev cmake g++ python3
> ```
> **macOS (Homebrew)**
> ```bash
> brew install qt cmake python3
> ```
> **Windows** — Install Qt via the online installer, then use Qt Creator or
> a Visual Studio Developer Command Prompt.

---

## Step 1 — Clone / unzip the project

```
mission-control/
├── CMakeLists.txt
├── README.md
├── simulator/
│   └── rover_simulator.py
└── src/
    ├── main.cpp
    ├── battery/
    │   ├── mapwidget.h
    │   └── mapwidget.cpp
    ├── signals/
    │   ├── roverworker.h
    │   └── roverworker.cpp
    ├── triangle/
    │   ├── mainwindow.h
    │   └── mainwindow.cpp
    └── window/
        └── roverstate.h
```

---

## Step 2 — Build the Qt application

```bash
# From the project root
mkdir build && cd build
cmake ..
cmake --build . --parallel
```

The binary will be at `build/MissionControl` (Linux/macOS)
or `build\Debug\MissionControl.exe` (Windows).

---

## Step 3 — Start the rover simulator

Open a **new terminal** (keep it open while using the app):

```bash
cd simulator
python3 rover_simulator.py
```

You should see:
```
  🛰   Rover Simulator
  ────────────────────────────────────
  Listening on  127.0.0.1:9000
  Waiting for Mission Control to connect…
```

---

## Step 4 — Run Mission Control

**macOS**
```bash
open build/MissionControl.app
```

**Linux**
```bash
./build/MissionControl
```

**Windows**
```bash
build\Debug\MissionControl.exe
```

The app will connect automatically. You'll see the rover start wandering
on the map and the EVENT LOG filling up.

---

## Commands

| Button | Effect |
|--------|--------|
| **▶ START / AUTO** | Rover resumes wandering autonomously |
| **⏸ STOP** | Rover halts in place |
| **🔴 ABORT** | Emergency stop — rover freezes |

---

## Project structure explained

```
src/
  main.cpp                  Entry point: dark palette + Fusion style → MainWindow
  window/
    roverstate.h            Plain data struct — the rover's telemetry snapshot
  signals/
    roverworker.h/cpp       Background thread: TCP socket → parse JSON → emit signal
  battery/
    mapwidget.h/cpp         Custom QWidget: QPainter draws grid, trail, rover body
  triangle/
    mainwindow.h/cpp        Main UI: telemetry bars, event log, buttons, layout
simulator/
  rover_simulator.py        Python TCP server — wanders, bounces, fires events
```


