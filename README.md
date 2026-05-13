Stellaris Engine

A real time N-body simulation engine focused on gravitational dynamics, numerical stability, and interactive visualization.

⸻

Overview

Stellaris Engine is a physics-based simulation project designed to model the interaction of multiple bodies under gravity. The engine currently supports Newtonian gravitational simulation using the Velocity Verlet integration method, combined with gravitational softening for improved numerical stability.

The simulation is rendered in real time using OpenGL, allowing interactive visualization of orbital mechanics, clustering, and emergent gravitational behavior.

⸻

Features

Current Features

* Real-time N-body gravitational simulation
* Newtonian gravity implementation
* Velocity Verlet integration
* Gravitational softening
* OpenGL rendering pipeline
* Dynamic body updates
* Real-time orbital visualization
* Configurable simulation parameters
* Stable timestep-based physics updates

⸻

Physics Model

Newtonian Gravity

The engine computes gravitational interaction between all bodies using Newton’s law of universal gravitation.

The force magnitude between two bodies is:

F = G \\frac{m_1 m_2}{r^2}

Where:

* G = gravitational constant
* m1, m2 = masses of interacting bodies
* r = distance between bodies

⸻

Gravitational Softening

To avoid singularities and extremely large forces at very small distances, the engine uses gravitational softening.

The softened force equation becomes:

F = G \\frac{m_1 m_2}{r^2 + \\epsilon^2}

Where:

* ε = softening parameter

Advantages

* Improved numerical stability
* Better collision handling
* Reduced force singularities
* Smoother large-scale simulations

⸻

Velocity Verlet Integration

The simulation currently uses the Velocity Verlet integration method, which provides:

* Better energy conservation
* Improved orbital stability
* Reduced numerical drift
* Higher accuracy than Euler integration

Position Update

x(t + \\Delta t) = x(t) + v(t)\\Delta t + \\frac{1}{2}a(t)\\Delta t^2

Velocity Update

v(t + \\Delta t) = v(t) + \\frac{a(t) + a(t + \\Delta t)}{2}\\Delta t

This makes the engine suitable for long-duration orbital simulations.

⸻

Rendering

The engine uses OpenGL for rendering.

Current Rendering Capabilities

* Real-time body visualization
* Frame-based updates
* GPU-accelerated rendering pipeline
* Scalable rendering architecture

⸻

Architecture

Core Components

Physics Engine

Handles:

* Force calculations
* Position updates
* Velocity integration
* Time stepping
* Softening calculations

Renderer

Handles:

* OpenGL rendering
* Camera transformations
* Frame updates
* Body visualization

Simulation Controller

Handles:

* Simulation loop
* Parameter control
* Body management
* Physics synchronization

⸻

Technologies Used

C++
OpenGL
GLSL
Physics Simulation
Numerical Integration

⸻

Current Limitations

* CPU-based force calculations
* O(N²) interaction complexity
* No collision merging yet
* No Barnes-Hut optimization
* Limited UI controls
* No GPU compute acceleration

⸻

Future Roadmap

Physics Improvements

* Barnes-Hut optimization
* Fast multipole methods
* Adaptive timestep support
* Relativistic corrections
* Collision detection and merging
* Elastic/inelastic collision handling
* Multi-scale simulation support

⸻

Rendering Improvements

* Particle trails
* HDR rendering
* Bloom effects
* Instanced rendering
* 3D camera controls
* Skybox integration
* Starfield rendering
* Post-processing pipeline

⸻

Performance Improvements

* GPU compute shaders
* CUDA/OpenCL acceleration
* Multithreaded physics updates
* SIMD optimization
* Spatial partitioning

⸻

Simulation Features

* Galaxy generation
* Black hole simulations
* Binary star systems
* Planetary system presets
* Procedural initial conditions
* Save/load simulation states
* Interactive simulation editor

⸻

UI / UX Features

* Real-time parameter controls
* Simulation statistics panel
* Pause/resume controls
* Camera orbit controls
* Debug visualization tools
* Trajectory prediction overlays

⸻

Example Applications

* Orbital mechanics visualization
* Galaxy formation experiments
* Educational physics demonstrations
* Numerical methods experimentation
* Real-time astrophysics visualization

⸻

Build Instructions

Requirements

C++17 Compiler
OpenGL
GLFW / SDL
GLEW / GLAD
CMake

⸻

Build

git clone <repository-url>
cd stellaris-engine
mkdir build
cd build
cmake ..
make

⸻

Running

./stellaris-engine

⸻

Project Goals

The long-term goal of Stellaris Engine is to evolve into a high-performance astrophysics sandbox capable of simulating large-scale gravitational systems with realistic physics and modern GPU-accelerated rendering.

⸻

License

MIT License

⸻

Author

Developed as a computational physics and graphics project exploring:

* N-body dynamics
* Numerical methods
* Real-time rendering
* Simulation architecture
* Computational astrophysics