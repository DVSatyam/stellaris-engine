# Stellaris Engine

A real-time N-body gravitational simulation engine built using C++ and OpenGL.

---

## Overview

Stellaris Engine simulates the gravitational interaction between multiple bodies in real time. The project focuses on orbital motion, numerical stability, and interactive visualization of large particle systems.

The engine currently supports binary black hole systems with orbiting particle disks, real-time controls, barycenter tracking, particle trails, and adjustable simulation speed.

---

## Features

- Real-time N-body gravitational simulation
- Velocity Verlet integration
- Gravitational softening
- OpenGL-based rendering
- Binary black hole preset
- Circumbinary particle disk simulation
- Real-time particle trails
- Dynamic glow rendering
- Barycenter tracking mode
- Pause / resume simulation
- Runtime simulation speed control
- Randomized system generation
- Real-time kinetic and potential energy display

---

## Physics Model

The engine uses Newtonian gravity for force calculations.

Force Equation:

F = (G * m1 * m2) / r²

Where:
- G = gravitational constant
- m1, m2 = masses of bodies
- r = distance between bodies

### Gravitational Softening

To avoid extremely large forces at very small distances:

F = (G * m1 * m2) / (r² + ε²)

Where:
- ε = softening factor

This improves stability and prevents force singularities.

---

## Integration Method

The simulation uses the Velocity Verlet integration method for updating motion.

Position Update:

x(t + dt) = x(t) + v(t)dt + 0.5a(t)dt²

Velocity Update:

v(t + dt) = v(t) + ((a(t) + a(t + dt)) / 2)dt

Advantages:
- Better energy conservation
- Improved orbital stability
- Reduced numerical drift

---

## Controls

| Key | Action |
|------|------|
| W A S D | Move camera |
| Up / Down Arrow | Zoom in / out |
| B | Toggle barycenter follow |
| P | Pause simulation |
| R | Reset simulation |
| [ ] | Decrease / increase simulation speed |

---

## Technologies Used

- C++
- OpenGL
- GLFW

---

## Current Limitations

- O(N²) force calculations
- CPU-only physics calculations
- No collision merging
- No Barnes-Hut optimization
- Limited UI system

---

## Future Plans

- Barnes-Hut optimization
- GPU acceleration
- Collision handling
- Improved rendering effects
- Galaxy generation presets
- Better UI and simulation controls
- 3D camera system
- Save/load simulation states

---

## Build Requirements

- C++17 Compiler
- OpenGL
- GLFW
- CMake

---

## Build Instructions

```bash
git clone <repository-url>
cd stellaris-engine

mkdir build
cd build

cmake ..
make
./stellaris-engine