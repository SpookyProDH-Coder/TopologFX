# TopologFX
**Mathematical Topology Visualizer. By SpookyProDH-Coder**

[![Status](https://img.shields.io/badge/Status-Work--In--Progress-orange)](#)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](#)

## Overview

TopologFX represents abstract mathematical topological spaces into computer graphics.

While euclidean spaces are the industry standard in computer graphics due to its computational simplicity and being straightforward; this engine takes a step ahead and build a geometry based on topology.

*Note: This project is still work in progress.*

## Features

### SceneManager

Acts as the handler of every active entity within a scene.

### EntityInspector

Each registred entity have its intrinsic properties.
This feature allows the user to apply in run-time properties such as:
* Affine Transformations: Adjust position, rotation and scale.
* Combinatiorial Surfaces:
    * Modify the word policy to generate fundamental shapes: Torus, Klein Bottle, Möbius Strip, Projective Plane and Sphere.
    * Control U/V subdivisions induced by the fundamental polygon grid.
* Renderer options:
    * Topological Heatmap: When enabled, colors the surface based on the p-norm distance of each subdivision from the world origin $(0,0,0)$.
    * Orientability Heatmap: When disabled, colors the surface based on their orientability (Red: Oriented; Blue: Not oriented).

### EngineConsole

An embedded developer console to execute commands in runtime
* Toggle the console using the `º` character.
* It features tabulation for command auto-completion.
* Several commands (WIP)

## Tech Stack:
* Languages: C++
* Graphics API: bgfx
* Environments: Linux (Tested on Fedora, Wayland)

## Getting started

### Prerequisites
Ensure you have the following dependencies installed:
* `g++` (C++ Compiler)
* `make` (Building source code tool)

### Installation & Building

1. Clone the repository:
```bash
git clone [https://github.com/SpookyProDH-Coder/TopologFX.git](https://github.com/SpookyProDH-Coder/TopologFX.git)
cd TopologFX
```

2. Run the building script:
```bash
chmod +x run.sh
./run.sh
```

3. Run the resulting executable inside the `engine/` folder:
```bash
./engine.out
```

## Bugs & Errors

### Klein Bottle & Möbius Strip
I haven't found a solution how to properly embed or project them in `TopologyPolicies.h`'s `GeometryEmbedding` structure.
Still, it is a visual bug, no worries.