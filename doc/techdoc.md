# libRealSpace Technical Documentation

## 1. Introduction

**libRealSpace** is a cross-platform open-source project aimed at recreating the RealSpace engine originally developed by Origin Systems for games such as Strike Commander and Wing Commander. The project's primary goal is to parse and render the original game assets, implement flight mechanics, and ultimately enable gameplay in modern environments with potential Virtual Reality support.

### 1.1 Project Overview

The libRealSpace project provides:
- Complete parsing capabilities for TRE, PAK, and IFF file formats
- 3D rendering of models with multiple levels of detail (LOD)
- Environment and terrain rendering
- Cockpit and instrument simulation
- Flight physics and control systems
- Mission scripting and gameflow logic
- User interface and menu systems

### 1.2 Target Platforms

The project supports:
- Windows
- Linux
- macOS

## 2. System Architecture

### 2.1 High-Level Architecture

The libRealSpace project is structured with a modular architecture consisting of five main components:

1. **Commons Layer**: Core utilities and data structures
2. **Engine Layer**: Rendering, audio, and input systems
3. **RealSpace Layer**: Game asset parsing and management
4. **SC Layer**: Strike Commander specific implementations
5. **TO Layer**: Tools and utilities

```
                  +---------------+
                  |  SC / TO Apps |
                  +---------------+
                          |
                          v
+----------+    +-------------------+    +---------+
| Engine   | <- | RealSpace Systems | <- | Commons |
+----------+    +-------------------+    +---------+
```

### 2.2 Directory Structure

```
librealspace/
├── src/
│   ├── commons/       # Utility classes (parsers, math, matrices)
│   ├── engine/        # Core engine components (rendering, audio)
│   ├── realspace/     # Game-specific logic and asset management
│   ├── sc/            # Strike Commander specific implementations
│   └── to/            # Tools and utilities
├── doc/               # Documentation files
├── scripts/           # Python tools for asset analysis
└── pics/              # Images for documentation
```

## 3. Core Systems

### 3.1 Asset Management System

The Asset Management System is responsible for loading, parsing, and managing game assets from original game files.

#### 3.1.1 Key Components

- **AssetManager**: Central manager for accessing game assets
  - Handles ISO9660 file systems from CD images
  - Implements caching for efficient file access
  - Provides file data extraction and manipulation

- **TreArchive**: Parser for TRE archives which store collections of game files
  - Extracts and provides access to entries within TRE files
  - Maps entry names to data locations

- **PakArchive**: Parser for PAK files, often contained within TRE archives
  - Handles sub-archives with their own entry systems
  - Supports various file formats within PAK files

- **IFF Parsers**: Two approaches for parsing Interchange File Format files
  - **IffLexer**: Traditional parsing for complete file parsing
  - **IFFSaxLexer**: Event-based "SAX-like" parsing with callbacks for efficient processing

#### 3.1.2 File Formats

The system supports various file formats used in the original games:

- **TRE**: Primary archive format for game assets
- **PAK**: Secondary archive format often contained within TRE files
- **IFF**: Interchange File Format for structured data
- **VOC**: Creative Voice format for audio data
- **XMI**: XMIDI format for music
- **SHP**: Shape format for UI elements and sprites

### 3.2 Rendering System

The Rendering System (`SCRenderer`) handles the 3D visualization of game assets, terrain, and interface elements.

#### 3.2.1 Key Features

- **Model Rendering**: Renders 3D models with multiple levels of detail
  - Support for textures and materials
  - Implements lighting and shading models
  - Handles animation data

- **Terrain Rendering**: Renders landscape and environment
  - Height-map based terrain system
  - Texture mapping for surface details
  - Support for water, desert, and other terrain types

- **Cockpit Rendering**: Simulates aircraft cockpits
  - Instrument panels with working displays
  - Multi-Function Displays (MFDs)
  - Heads-Up Display (HUD)

- **Special Effects**: Various visual effects
  - Fog and atmospheric effects
  - Smoke and explosion effects
  - Lighting and glint effects

#### 3.2.2 Technical Implementation

The renderer uses legacy OpenGL for compatibility across platforms, with vertex buffers and basic shading techniques. Key methods include:

- `renderWorldSolid()`: Renders the terrain with lighting and fog
- `displayModel()`: Renders 3D models with textures
- `renderMissionObjects()`: Renders mission-specific objects

### 3.3 Gameflow System

The Gameflow System (`RSGameFlow`) manages the overall game progression, mission structure, and player state.

#### 3.3.1 Components

- **Mission System**: Handles mission structure and objectives
  - Mission parsing and initialization
  - Objective tracking and completion
  - Event triggers

- **Scene System**: Manages different game screens and UI elements
  - Menu screens
  - Object viewers
  - Transition animations

- **State Management**: Tracks game and player state
  - Player statistics and inventory
  - Mission progress
  - Financial data (cash, overhead costs)

#### 3.3.2 Data Structures

The gameflow system uses various data structures to represent game elements:

- `GAMEFLOW_SCEN`: Represents game scenes with sprites and requirements
- `MISS`: Contains mission data including information, scenes, and effects
- `WING`: Stores wing data with pilots and information
- `CHNG`: Contains state change data for pilots, cash, and weapons

### 3.4 Mission System

The Mission System (`RSMission`) handles the parsing, management, and execution of game missions.

#### 3.4.1 Key Components

- **Mission Parsing**: Reads and interprets mission files
  - Extracts mission parameters
  - Loads and positions objects
  - Sets up scripts and triggers

- **Script System**: Executes mission scripts
  - Interprets opcode-based scripts
  - Handles mission progression
  - Manages AI behavior

- **Area Management**: Handles mission areas and navigation
  - Defines playable areas
  - Manages waypoints and objectives
  - Controls area-based triggers

#### 3.4.2 Data Structures

- `MISN`: Core mission data structure
- `MISN_PART`: Represents mission participants (aircraft, buildings)
- `SPOT`: Defines important locations within missions
- `PROG`: Contains script program opcodes and arguments

### 3.5 Cockpit System

The Cockpit System (`RSCockpit`) simulates and renders aircraft cockpits and their instruments.

#### 3.5.1 Key Features

- **Multi-Function Displays**: Simulates various cockpit displays
  - Weapons display with selection and status
  - Radar display with target tracking
  - Communications interface
  - Damage display with aircraft status

- **Instrument Panels**: Recreates flight instruments
  - Altimeter
  - Airspeed indicator
  - Attitude indicator
  - Warning indicators

- **Heads-Up Display**: Projects flight data onto the windscreen
  - Flight path vector
  - Target tracking
  - Weapons status

#### 3.5.2 Implementation

The cockpit system uses IFF parsing to reconstruct the cockpit elements:

- `parseMONI_MFDS_WEAP()`: Parses weapon display data
- `parseMONI_MFDS_AARD()`: Parses air radar display data
- `parseMONI_INST_ALTI()`: Parses altimeter instrument data

## 4. File Formats

### 4.1 TRE (TreasuRE) Archives

TRE archives are the primary containers for game assets, featuring:
- Directory structure with named entries
- Offset-based entry system for direct access
- Compression for efficiency

### 4.2 PAK Archives

PAK archives are secondary containers, often nested within TRE files:
- Simple header with entry count
- Fixed-size entries with type and size information

### 4.3 IFF (Interchange File Format)

IFF is a structured hierarchical format used for most game data:
- Chunk-based format with 4-character type identifiers
- Nested structure for complex data organization
- Length-prefixed chunks for easy parsing

### 4.4 Asset-Specific Formats

The project supports various specialized formats:
- **RSEntity**: 3D models with multiple LOD levels
- **RSImage**: Texture and image data
- **RSArea**: Terrain and environment data
- **RSMusic**: Music and sound effect data

## 5. Key Classes and Components

### 5.1 Core Engine Classes

- **SCRenderer**: Main rendering system
- **Camera**: View and projection management
- **GameEngine**: Core game loop and state management
- **RSScreen**: Window and display management
- **RSMixer**: Audio playback and management

### 5.2 Asset Management Classes

- **AssetManager**: Primary interface for asset loading
- **TreArchive**: Parser for TRE archives
- **PakArchive**: Parser for PAK archives
- **IFFSaxLexer**: Event-based IFF parser
- **ByteStream**: Helper for binary data parsing

### 5.3 Game System Classes

- **RSGameFlow**: Game progression and state
- **RSMission**: Mission management
- **RSArea**: Terrain and environment
- **RSCockpit**: Cockpit simulation
- **RSWorld**: World data and parameters

### 5.4 Math and Utility Classes

- **Matrix**: Matrix operations for 3D transformations
- **Quaternion**: Rotation representation
- **Vector3D**: Three-dimensional vector
- **LZBuffer**: Compression utilities

## 6. Development Workflow

### 6.1 Build System

The project uses CMake for cross-platform building with the following presets:
- **windows**: Visual Studio 2022 for Windows
- **linux**: GCC 13.2.0 for Linux
- **linux_clang**: Clang 18.1.3 for Linux

Dependencies are managed through vcpkg, with key libraries:
- SDL2
- SDL2_mixer_ext
- Dear ImGui
- OpenGL

### 6.2 Coding Standards

The project follows C++17 standards with:
- Class-based encapsulation for key components
- SAX-style parsing for efficient asset loading
- Callback-based event handling

### 6.3 Testing

Testing is done through:
- Direct visualization of parsed assets
- Python scripts for asset analysis
- Runtime validation of parsing results

## 7. Core Algorithms

### 7.1 Asset Parsing

The project implements sophisticated parsing algorithms for game assets:
- IFF lexers for structured data
- Binary stream parsing for raw data
- Event-based parsing for large files

### 7.2 3D Rendering

Rendering algorithms include:
- Level of Detail (LOD) management for models
- Fog and atmospheric effects
- Texture mapping and coordinate transformation

### 7.3 Flight Model

The flight model simulates:
- Aerodynamic forces and physics
- Aircraft control systems
- Mission-specific behavior parameters

## 8. Future Development

### 8.1 Planned Features

Based on the codebase examination:
- Enhanced VR support
- Advanced AI flight models
- Expanded mission system
- Improved rendering with modern OpenGL/Vulkan

### 8.2 Known Limitations

Current limitations include:
- Incomplete mission scripting support
- Limited AI behavior implementation
- Partial support for some asset types

## 9. Appendices

### 9.1 Glossary

- **TRE**: TreasuRE archive format
- **PAK**: Package archive format
- **IFF**: Interchange File Format
- **LOD**: Level of Detail
- **MFD**: Multi-Function Display
- **HUD**: Heads-Up Display

### 9.2 References

- Original RealSpace engine documentation
- Strike Commander technical specifications
- Wing Commander technical specifications

### 9.3 Contributors

The project is a community effort to recreate and preserve the RealSpace engine for historical and educational purposes.

---

This technical documentation provides a comprehensive overview of the libRealSpace project's architecture, components, and implementation. It serves as a guide for developers working with the codebase and for understanding the inner workings of the RealSpace engine recreation.