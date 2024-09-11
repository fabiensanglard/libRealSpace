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
- Flying
- GameFlow logic
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

3. Configure the project:
     - Open a terminal or command prompt.
     - Navigate to the project directory.
     - Run the following command to configure the project using CMake:
       ```bash
       cmake --preset=windows
       ```
     - current preset are :
          - windows, to build for windows with Visual Studio 17 2022
          - linux, to build for linux with GCC 13.2.0 x86_64-linux-gnu
          - linux_clang to build for linux with Clang 18.1.3 x86_64-pc-linux-gnu


5. Build the project:
     - Run the following command to build the project using CMake:
       ```
       cmake --build build
       ```

6. Run the project:
     - After the build is successful, you can run the project using the generated executable.

Remember to adjust the paths and commands according to your specific setup.

