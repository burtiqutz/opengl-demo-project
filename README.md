# OpenGL Demo Project

A C++ OpenGL rendering mini-engine developed on Linux (Pop!_OS). This project demonstrates modern OpenGL concepts including 3D model loading, texture mapping, skyboxes, and camera movement. 
<img width="1280" height="694" alt="image" src="https://github.com/user-attachments/assets/e953cecf-b9c7-4cb0-96e4-e27f20d50fdd" />
<img width="1280" height="694" alt="image" src="https://github.com/user-attachments/assets/5fce2154-04f6-455b-97b4-94db0c5cdf63" />
<img width="1280" height="694" alt="image" src="https://github.com/user-attachments/assets/075fc81d-309a-49a7-a297-48be9752d414" />
<img width="1280" height="694" alt="image" src="https://github.com/user-attachments/assets/a13fd555-78b8-481d-87db-a0177804a869" />

## Credits & Coursework

This project was developed as part of the Graphics Processing course at the Technical University of Cluj-Napoca.  
Boilerplate Code: The implementations for object loading (tiny_obj_loader) and texture loading (stb_image) were adapted from provided course materials and boilerplate code to handle asset parsing efficiently.

## Features

* **Modern OpenGL**: Utilizes the programmable shader pipeline (GLSL).
* **Advanced Lighting**: Implements the **Blinn-Phong** lighting model for realistic ambient, diffuse, and specular reflections.
* **Shadow Mapping**: Real-time dynamic shadows rendering using depth map techniques.
* **Collision System**: Simple AABB collision system enabled per scene object.
* **3D Model Loading**: Support for loading `.obj` files using `tiny_obj_loader`.
* **Textures**: Image loading and texture mapping using `stb_image`.
* **Camera System**: First-person fly camera for navigating the scene.
* **Skybox**: Cubemap implementation for immersive backgrounds.

## Prerequisites

This project was developed using **CLion** on **Pop!_OS** (Ubuntu-based). To build it, you will need the following dependencies installed on your system:

* **C++ Compiler** (GCC/G++)
* **CMake** (Version 3.10+)
* **OpenGL Drivers**
* **GLFW3** (Windowing and Input)
* **GLEW** (OpenGL Extension Wrangler)
* **GLM** (OpenGL Mathematics)

### Installing Dependencies (Pop!_OS / Ubuntu)

Open your terminal and run the following commands to install the necessary libraries:
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libgl1-mesa-dev libglfw3-dev libglew-dev libglm-dev
```
## Building & setup
Option 1: Using CLion (Recommended)

  1. Clone this repository or download the source code
  2. Open CLion.
  3. Select File > Open and choose the project folder opengl-demo-project.
  4. CLion will automatically detect the CMakeLists.txt file and load the project configuration.
  5. Click the Build icon (Hammer) to compile.
  6. Click the Run icon (Play) to start the application.

Option 2: Using Terminal (CMake)

  1. Clone the repository (SSH):
     ```Bash
     git clone git@github.com:burtiqutz/opengl-demo-project.git
     cd opengl-demo-project
      ```
  2. Create a build directory and compile:
     ```Bash
        mkdir build
        cd build
        cmake ..
        make
  3. Run the application
     ```Bash
     ./opengl_demo_project
## Controls
| Key | Action |
| :---: | :--- |
| <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> | Camera movement |
| <kbd>Mouse</kbd> | Look around |
| <kbd>Shift</kbd> | Sprint / Fast movement |
| <kbd>C</kbd> | Toggle Cinematic Mode |
| <kbd>T</kbd> | Cycle Display Modes (Solid &rarr; Wireframe &rarr; Polygonal) |
| <kbd>G</kbd> | Toggle Sun Light (On/Off) |
| <kbd>P</kbd> | Toggle Point Lights (Lanterns) |
| <kbd>M</kbd> | Toggle Snowfall |
## Project structure
- **src/**: Main C++ source files (main.cpp, Window.cpp, etc.).
- **shaders/**: GLSL Vertex and Fragment shaders.
- **models/**: 3D assets (e.g., .obj files).
- **skybox/**: Texture images for the skybox cubemap.
- **CMakeLists.txt**: Build configuration.
- **External Libraries Included**:
  - `stb_image`: Image loading.
  - `tiny_obj_loader`: OBJ model parsing.
## License
This project is open source and free to use anywhere.
