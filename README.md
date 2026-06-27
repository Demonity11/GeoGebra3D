readme_content = """# GeoGebra3D

A high-performance, interactive 3D Geometric & Mathematical Visualization Engine built from scratch using Modern OpenGL and C++17. Designed as an educational CAD-like environment for real-time relational geometry rendering and vector calculation.

## 🚀 Overview

GeoGebra3D allows users to dynamically input, visualize, and manipulate mathematical primitives (Points, Vectors, Lines, and Planes) in a true 3D viewport. The engine interprets geometric entities not just as isolated graphic arrays, but as components of an interconnected relational graph, updating dependents in real-time upon parental modification.

---

## 🛠️ Tech Stack & Dependencies

The project is built entirely targeting the native Windows `x64` architecture, leveraging the following industry-standard low-level graphics and windowing libraries:

* **Language:** C++17
* **Graphics API:** Modern OpenGL (Core Profile).
* **Windowing & Input:** GLFW (Window management and native raw mouse/keyboard callbacks).
* **OpenGL Loading Library:** GLAD.
* **Mathematics:** GLM (OpenGL Mathematics - header-only vector/matrix transformations).
* **User Interface:** Dear ImGui (Immediate Mode GUI for variables and command input processing).

---

## 🏛️ Core Features

* **3D Interactive Viewport:** Full spatial navigation governed by a custom virtual camera system using Euler angles (Yaw/Pitch) map-controlled via raw mouse deltas.
* **Procedural Infinite Grid:** An infinite structural grid and explicit RGB coordinate axes rendered dynamically via heavy fragment-shader procedural mathematical computations per fragment.
* **Dynamic Command Parser:** Real-time terminal string interpreter within the UI capable of evaluating complex multi-argument geometric functions (e.g., instantiation of lines from points, planes from point-normal configurations, and intersections).
* **Relational Primitive Intersections:** Real-time geometric evaluation of line-plane intersections that continuously recalibrate coordinate values over runtime modifications.

---

## 📐 Mathematical Underpinnings

The core computational logic avoids external black-box solvers, performing explicit linear algebraic calculations directly on the CPU.

### Real-Time Line-Plane Intersection
The intersection point $P$ of a parametric line $P(t) = P_L + t \cdot \vec{v}$ and a general implicit plane $\pi: ax + by + cz + d = 0$ with normal vector $\vec{n} = (a, b, c)$ is deduced by injecting the parametric components into the plane equation and isolating the scalar parameter $t$:

$$t = -\frac{\vec{n} \cdot P_L + d}{\vec{n} \cdot \vec{v}}$$

Where:
* $d = -(\vec{n} \cdot P_\pi)$, with $P_\pi$ representing the plane's anchoring point.
* $\vec{v}$ represents the direction vector of the line.
* $\vec{n}$ represents the normal vector of the plane.

The engine intercepts singular conditions (**Edge Cases**): if the dot product in the denominator ($\vec{n} \cdot \vec{v}$) approaches zero ($\pm \epsilon$), the engine detects a parallel state, suppresses undefined floating-point execution, and gracefully flags the entity as invalid without halting the simulation loop.

---

## 🏗️ Architectural Decisions & Low-Level Engineering

### 1. State Isolation via Centralized Namespace Context
To eliminate the architectural technical debt of loose global variables, the entire global state of the engine (including rendering tokens like VAO/VBO handles, vector buffers, and UI parsing structures) is encapsulated into a highly coordinated `Context` namespace.
* **Compilation Decoupling:** Employs explicit `extern` linkage declarations inside headers and physical definitions inside localized compilation units (`.cpp`).
* **Header-only Constants:** Utilizes modern C++17 `inline constexpr` modifiers for structural boundaries (e.g., parent IDs for literal expressions), collapsing the memory footprint to zero and allowing compile-time literal optimization.

### 2. Directed Acyclic Graph (DAG) & Cascading Deletion
Geometric relationships are managed via a strict Directed Acyclic Graph (DAG). Entities track their ancestors through explicit identification registries (`parentIDs`).
* **Cascading Traversal:** Deleting a parent node triggers a deep recursive query across the entire vector container, systematically hunting down and purging orphaned child primitives.
* **Memory Integrity Protection:** Engineered to resolve the classic C++ *Iterator Invalidation* trap during runtime `std::vector` mutations by executing multi-stage evaluation passes paired with a clean **Total Rebuild Pipeline**. The rendering engine subsequently clears vertex streams and streams fresh, contiguous bit blocks to the GPU VRAM via specialized `glBufferData` routines.

### 3. Immediate-Mode UI Stack Stabilization
Dynamic generation of text arrays and fields inside Dear ImGui routinely breaks input handling due to shifting frame states. The engine counters this by invoking ImGui's internal **ID Stack separation mechanism** via the unique `###` token. This locks down persistent structural keyboard focus indices inside the memory layout while leaving outward text labels free to update dynamically.

---

## 🚀 How to Build

The workspace is organized as a native Visual Studio Solution.

### Prerequisites
1. **Visual Studio 2022** (with the *Desktop development with C++* workload installed).
2. Graphic hardware supporting at least **OpenGL 3.3 Core Profile**.

### Step-by-Step Compilation
1. Clone the repository to your local directory: