libRealSpace
============

Tools to parse and render Origin Systems RealSpace engine assets.

- Full TRE Support.
- Full IFF Support.
- Full PAK Support.

- Render Textures 3D Jets for all Level Of Details
- Render animation layers (individually).
- Render Map Textures.
- Render ingame screen (conversation, transition animation, scenes)
- Fly in mission

- Guess PAK contents:
     - VOC sound effects.
     - XMidi music
     - Animations.
     - IFF
     - Images
     - Map Textures
     - Sub-pack structure

Support MacOS X, Windows and Linux 

Goal :
======

The long term goal is to be able to play Strike Commander with a Virtual Reality Headset
(Oculus Rift).

Dependencies :
==============

- SDL 2: https://www.libsdl.org/
- Dear IMGui: https://github.com/ocornut/imgui
- SDL-Mixer-ext: https://wohlsoft.github.io/SDL-Mixer-X/
- Legacy OpenGL

Eye Candies :
=============

## News :)

![Alt text](pics/gameflow.png)
![Alt text](pics/flying.png)
![Alt text](pics/conversation.png)
![Alt text](pics/external.png)
![Alt text](pics/transistion.png)
![Alt text](pics/virtualcp.png)
![Alt text](pics/object_viewer.png)

## Originals Eye Candies :)

![Alt text](pics/nice_sc_scene.png)
![Alt text](/pics/F-22.png)
![Alt text](/pics/face.png)

Build :
=======

To build the project using CMake and Vcpkg, follow these instructions:

1. Install CMake: 
     - Download and install CMake from the official website: https://cmake.org/download/
     - Follow the installation instructions for your operating system.

2. Install Vcpkg:
     - Download and install Vcpkg from the official GitHub repository: https://github.com/microsoft/vcpkg
     - Follow the installation instructions for your operating system.

3. Install dependencies:
     - Open a terminal or command prompt.
     - Navigate to the Vcpkg directory.
     - Run the following command to install the SDL2 library:
       ```
       vcpkg install sdl2
       ```

4. Configure the project:
     - Open a terminal or command prompt.
     - Navigate to the project directory.
     - Run the following command to configure the project using CMake:
       ```
       cmake -B build -S .
       ```

5. Build the project:
     - Run the following command to build the project using CMake:
       ```
       cmake --build build
       ```

6. Run the project:
     - After the build is successful, you can run the project using the generated executable.

Remember to adjust the paths and commands according to your specific setup.

