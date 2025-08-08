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

The libRealSpace project is structured with a modular architecture consisting of six main components:

1. **Commons Layer**: Core utilities and data structures (math, parsing, compression)
2. **Engine Layer**: Rendering, audio, input systems, and core game loop
3. **RealSpace Layer**: Game asset parsing, management, and game systems
4. **Strike Commander Layer**: Strike Commander specific implementations
5. **Executables Layer**: Application entry points (game, tools, debugger)
6. **Tactical Operation Layer**: Utility applications for asset analysis and debugging

```
    +----------------+    +----------------+    +----------------+
    |   SC Client    |    |     TO Client  |    |    Debugger    |
    +----------------+    +----------------+    +----------------+
                  |                |                       |
                  v                v                       v
    +-------------------------------------------------------------------+
    |                        Strike Commander Layer                    |
    +-------------------------------------------------------------------+
                                    |
                                    v
    +-------------------------------------------------------------------+
    |                         RealSpace Layer                          |
    |  (Asset Management, Game Systems, Mission, Cockpit, Gameflow)    |
    +-------------------------------------------------------------------+
                                    |
                                    v
    +-------------------------------------------------------------------+
    |                          Engine Layer                            |
    |     (Rendering, Audio, Input, Camera, Screen Management)         |
    +-------------------------------------------------------------------+
                                    |
                                    v
    +-------------------------------------------------------------------+
    |                          Commons Layer                           |
    |    (Math, Parsing, Compression, Utilities, Data Structures)     |
    +-------------------------------------------------------------------+
```

### 2.2 Directory Structure

```
librealspace/
├── src/
│   ├── commons/           # Core utilities and data structures
│   │   ├── ByteStream     # Binary data stream handling
│   │   ├── IFFSaxLexer    # Event-based IFF parsing
│   │   ├── LZBuffer       # Compression utilities
│   │   ├── Matrix         # 3D transformation matrices
│   │   ├── Quaternion     # Rotation representation
│   │   └── RLEBuffer      # Run-length encoding utilities
│   ├── engine/            # Core engine components
│   │   ├── SCRenderer     # 3D rendering system
│   │   ├── GameEngine     # Main game loop and state
│   │   ├── Camera         # View and projection management
│   │   ├── RSScreen       # Display and window management
│   │   ├── RSMixer        # Audio system
│   │   └── Texture        # Texture management
│   ├── realspace/         # Game systems and asset management
│   │   ├── AssetManager   # Central asset loading system
│   │   ├── TreArchive     # TRE file format parser
│   │   ├── PakArchive     # PAK file format parser
│   │   ├── RSEntity       # 3D models and entities
│   │   ├── RSImage        # Image and texture assets
│   │   ├── RSGameflow     # Game progression system
│   │   ├── RSMission      # Mission management
│   │   ├── RSCockpit      # Cockpit simulation
│   │   ├── RSArea         # Terrain and environment
│   │   ├── RSWorld        # World parameters
│   │   ├── RSSmokeSet     # Smoke and particle effects
│   │   └── RSFont         # Font rendering system
│   ├── strike_commander/  # Strike Commander specific code
│   │   ├── SCStrike       # Main Strike Commander class
│   │   ├── SCMission      # SC-specific mission handling
│   │   ├── SCGameFlow     # SC game flow implementation
│   │   ├── SCCockpit      # SC cockpit implementation
│   │   ├── SCSimplePlane  # Basic aircraft simulation
│   │   ├── SCJdynPlane    # Advanced flight dynamics
│   │   └── SCVectorPlane  # Vector-based plane model
│   └── executables/       # Application entry points
│       ├── sc/            # Strike Commander game executable
│       ├── to/            # Tactical Operation
│       └── debugger/      # Debug interface with ImGui
├── doc/                   # Documentation and technical specs
├── scripts/               # Python analysis and parsing tools
└── pics/                  # Documentation images
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

#### 3.1.2 File Formats and Asset Types

The system supports various file formats and asset types:

**Archive Formats:**
- **TRE**: Primary archive format for game assets
- **PAK**: Secondary archive format often contained within TRE files

**Media Formats:**
- **IFF**: Interchange File Format for structured data
- **VOC**: Creative Voice format for audio data
- **XMI**: XMIDI format for music
- **SHP**: Shape format for UI elements and sprites

**Game Asset Classes:**
- **RSEntity**: 3D models with multiple levels of detail (LOD)
- **RSImage**: Texture and image data with palette support
- **RSImageSet**: Collections of related images
- **RSArea**: Terrain and environment data
- **RSWorld**: Global world parameters and settings
- **RSSmokeSet**: Smoke and particle effect definitions
- **RSFont**: Font rendering and text display
- **RSMission**: Mission data and scripting
- **RSGameFlow**: Game progression and state management

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

### 3.6 Debug and Development System

The project includes comprehensive debugging and development tools through the **DebugStrike** system.

#### 3.6.1 Debug Interface Features

- **Real-time Radar Display**: Interactive top-down view of mission actors
  - Actor position tracking and visualization
  - Clickable elements for detailed inspection
  - Pan and zoom capabilities for navigation
  - Support for both aircraft and ground objects

- **Simulation Information Panel**: Real-time flight data monitoring
  - Aircraft state visualization
  - Performance metrics display
  - Configuration parameter adjustment

- **Entity Inspector**: Detailed examination of game objects
  - Actor properties and states
  - 3D model information and LOD details
  - Animation and effect data

- **Mission Analysis Tools**: Mission debugging capabilities
  - Mission parameter inspection
  - Script execution monitoring
  - Area and waypoint visualization

#### 3.6.2 ImGui Integration

The debug system uses **Dear ImGui** for its interface, providing:
- Real-time parameter adjustment
- Interactive 3D visualizations
- Performance profiling displays
- Asset browser and inspector
- Memory usage monitoring

#### 3.6.3 Development Workflow Support

- **Hot-reloading**: Dynamic asset reloading during development
- **Performance Profiling**: Frame time and rendering statistics
- **Asset Validation**: Automatic checking of parsed assets
- **Debug Logging**: Comprehensive logging system for troubleshooting

### 3.7 Cockpit System

The Cockpit System (`RSCockpit` and `SCCockpit`) simulates and renders aircraft cockpits and their instruments.

#### 3.7.1 Key Features

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

#### 3.7.2 Implementation

The cockpit system uses IFF parsing to reconstruct the cockpit elements:

- `parseMONI_MFDS_WEAP()`: Parses weapon display data
- `parseMONI_MFDS_AARD()`: Parses air radar display data
- `parseMONI_INST_ALTI()`: Parses altimeter instrument data

### 3.8 Aircraft Simulation System

The project implements multiple aircraft simulation models for different complexity levels:

#### 3.8.1 Simulation Models

- **SCSimplePlane**: Basic aircraft model for simple flight mechanics
  - Basic physics simulation
  - Simple control response
  - Suitable for testing and development

- **SCJdynPlane**: Advanced flight dynamics using JDYN data
  - Realistic aerodynamic modeling
  - Advanced flight characteristics
  - Uses original game's flight data tables

- **SCVectorPlane**: Vector-based aircraft simulation
  - Mathematical flight model
  - Precise control systems
  - Advanced physics calculations

#### 3.8.2 Flight Systems

- **Throttle Control**: Engine power management
- **Flight Control Surfaces**: Aileron, elevator, rudder control
- **Landing Gear**: Retractable gear system with physics
- **Weapon Systems**: Integrated weapons management
- **Damage Modeling**: Aircraft damage and performance degradation

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

- **GameEngine**: Core game loop and state management
  - Main application loop
  - State transitions
  - Resource management

- **SCRenderer**: Main rendering system
  - 3D model rendering with LOD support
  - Terrain and environment rendering
  - Special effects and particle systems

- **Camera**: View and projection management
  - 3D camera positioning and orientation
  - Projection matrix calculations
  - View frustum management

- **RSScreen**: Window and display management
  - SDL2-based window creation
  - Display mode management
  - Input event handling

- **RSMixer**: Audio playback and management
  - Multi-channel audio mixing
  - Music and sound effect playback
  - 3D positional audio support

- **Texture**: Texture loading and management
  - Texture atlas support
  - Automatic mipmap generation
  - Memory-efficient texture streaming

### 5.2 Asset Management Classes

- **AssetManager**: Primary interface for asset loading
  - Unified asset access API
  - Caching and memory management
  - Multi-format support

- **TreArchive**: Parser for TRE archives
  - Directory-based file access
  - Efficient entry lookup
  - Nested archive support

- **PakArchive**: Parser for PAK archives
  - Sequential file access
  - Type-based file identification
  - Compression support

- **IFFSaxLexer**: Event-based IFF parser
  - Memory-efficient parsing
  - Callback-based processing
  - Support for large files

- **ByteStream**: Helper for binary data parsing
  - Endian-aware data reading
  - Automatic buffer management
  - Error handling and validation

### 5.3 Game System Classes

- **RSGameFlow**: Game progression and state management
  - Mission sequencing
  - Player statistics tracking
  - Save/load functionality
  - Campaign progression

- **RSMission**: Mission management and execution
  - Mission script interpretation
  - Object placement and management
  - Trigger system
  - AI behavior coordination

- **RSArea**: Terrain and environment rendering
  - Height-map based terrain
  - Texture blending and mapping
  - Environmental effects
  - Collision detection

- **RSCockpit**: Cockpit simulation base class
  - Instrument panel management
  - MFD (Multi-Function Display) system
  - HUD (Heads-Up Display) rendering

- **RSWorld**: World data and global parameters
  - Global game settings
  - World-wide constants
  - Environmental parameters

- **RSSmokeSet**: Particle and effect system
  - Smoke effects
  - Explosion animations
  - Environmental particles

### 5.4 Strike Commander Specific Classes

- **SCStrike**: Main Strike Commander game class
  - Game loop implementation
  - Mission management
  - Player interaction handling

- **SCGameFlow**: SC-specific game flow implementation
  - Campaign structure
  - Mission briefings
  - Story progression

- **SCMission**: Strike Commander mission system
  - SC-specific mission parsing
  - Custom mission events
  - Performance evaluation

- **SCCockpit**: Strike Commander cockpit implementation
  - Authentic SC cockpit layout
  - Original instrument behavior
  - Historical accuracy

- **SCSimplePlane/SCJdynPlane/SCVectorPlane**: Aircraft simulation models
  - Multiple complexity levels
  - Realistic flight physics
  - Weapon system integration

### 5.5 Development and Debug Classes

- **DebugStrike**: Comprehensive debugging interface
  - Real-time mission monitoring
  - Interactive asset inspection
  - Performance profiling tools
  - ImGui-based user interface

### 5.6 Math and Utility Classes

- **Matrix**: 3D transformation matrices
  - 4x4 matrix operations
  - Model, view, and projection transformations
  - Optimized matrix multiplication

- **Quaternion**: Rotation representation and operations
  - Smooth rotation interpolation
  - Gimbal lock avoidance
  - Conversion to/from Euler angles

- **Vector3D**: Three-dimensional vector operations
  - Dot and cross products
  - Vector normalization
  - Distance calculations

- **ByteStream**: Binary data stream processing
  - Endian-aware reading/writing
  - Automatic buffer management
  - Type-safe data access

- **LZBuffer**: Compression and decompression utilities
  - LZ-based compression algorithms
  - Memory-efficient processing
  - Error handling and validation

- **RLEBuffer**: Run-length encoding utilities
  - Efficient compression for image data
  - Fast decompression algorithms
  - Support for various data types

- **IFFSaxLexer**: Event-driven IFF parsing
  - Callback-based processing
  - Memory-efficient for large files
  - Hierarchical data structure support

## 6. Development Workflow

### 6.1 Build System

The project uses **CMake** for cross-platform building with the following configuration:

**Build Presets:**
- **windows**: Visual Studio 2022 for Windows development
- **linux**: GCC 13.2.0 for Linux systems
- **linux_clang**: Clang 18.1.3 for Linux with Clang

**Dependencies** (managed through vcpkg):
- **SDL2**: Cross-platform window management and input
- **SDL2_mixer_ext**: Advanced audio mixing and playback
- **Dear ImGui**: Immediate mode GUI for debugging interface
- **OpenGL**: 3D graphics rendering (legacy OpenGL for compatibility)

**Project Structure:**
- **C++17 Standard**: Modern C++ features with broad compatibility
- **Modular Architecture**: Clear separation between engine, game, and utility layers
- **Cross-platform Support**: Windows, Linux, and macOS compatibility

### 6.2 Coding Standards

The project follows **C++17** standards with modern best practices:

**Code Organization:**
- **Class-based encapsulation**: Object-oriented design for major components
- **RAII principles**: Resource management through constructors/destructors
- **Exception safety**: Proper error handling and resource cleanup

**Parsing Architecture:**
- **SAX-style parsing**: Event-driven parsing for efficient memory usage
- **Callback-based event handling**: Flexible and extensible processing
- **Stream-based I/O**: Efficient binary data processing

**Memory Management:**
- **Smart pointers**: Modern C++ memory management
- **RAII for resources**: Automatic cleanup of system resources
- **Efficient caching**: Optimized asset loading and storage

**Performance Considerations:**
- **LOD systems**: Level-of-detail for 3D models and terrain
- **Efficient rendering**: Optimized OpenGL usage
- **Asset streaming**: On-demand loading of game resources

### 6.3 Testing and Quality Assurance

**Testing Methodology:**
- **Visual Asset Validation**: Direct rendering and inspection of parsed assets
- **Interactive Debugging**: Real-time debugging through ImGui interface
- **Python Analysis Tools**: Automated asset analysis and validation scripts
- **Runtime Validation**: Continuous validation during asset parsing

**Debug Tools:**
- **DebugStrike Interface**: Comprehensive debugging environment
- **Asset Inspector**: Real-time asset examination and validation
- **Performance Profiler**: Frame timing and resource usage monitoring
- **Memory Tracker**: Memory allocation and leak detection

**Quality Assurance:**
- **Cross-platform Testing**: Validation across Windows, Linux, and macOS
- **Asset Compatibility**: Verification with original game assets
- **Performance Benchmarking**: Frame rate and memory usage optimization

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

### 8.1 Current Capabilities

**Asset System:**
- Complete TRE/PAK archive parsing
- IFF format support with hierarchical data structures
- Multi-format image and texture loading
- 3D model loading with LOD support
- Audio format support (VOC, XMI)

**Rendering System:**
- 3D model rendering with textures
- Terrain rendering with height maps
- Cockpit and instrument simulation
- Particle and smoke effects
- Multi-camera support

**Game Systems:**
- Mission loading and parsing
- Gameflow and progression tracking
- Aircraft simulation with multiple models
- Weapon systems integration
- Real-time debugging interface

**Development Tools:**
- Comprehensive debug interface
- Asset inspection and validation
- Performance profiling
- Interactive mission analysis
### 8.2 Planned Features

**Enhanced Rendering:**
- Modern OpenGL/Vulkan support for improved performance
- Advanced shader systems for realistic lighting
- Post-processing effects and modern visual enhancements
- Enhanced particle systems and environmental effects

**VR Support:**
- Virtual Reality cockpit implementation
- 6DOF head tracking for immersive flight experience
- VR-optimized user interface elements
- Hand tracking for cockpit interaction

**Advanced Game Systems:**
- Complete mission scripting system implementation
- Enhanced AI flight models with realistic behavior

### 8.3 Known Limitations

**Current Implementation Constraints:**
- **Legacy OpenGL**: Using older OpenGL for compatibility, limiting modern features
- **Incomplete Mission Scripting**: Some mission opcodes and behaviors not fully implemented
- **Limited AI Complexity**: Basic AI behaviors, advanced formation flying in development
- **Asset Format Coverage**: Some less common asset types not yet fully supported

**Performance Considerations:**
- **Single-threaded Rendering**: Current rendering system is primarily single-threaded
- **Memory Usage**: Asset caching could be optimized for large worlds
- **LOD System**: Level-of-detail transitions could be smoother

**Platform-specific Issues:**
- **Audio System**: Some audio formats have platform-specific limitations
- **Input Handling**: Advanced joystick support varies by platform
- **File System**: Case-sensitive systems require careful path handling

## 9. Appendices

### 9.1 Glossary

**File Formats:**
- **TRE**: TreasuRE archive format - Primary container for game assets
- **PAK**: Package archive format - Secondary container within TRE files
- **IFF**: Interchange File Format - Structured hierarchical data format
- **VOC**: Creative Voice format - Audio file format
- **XMI**: XMIDI format - Music file format

**Technical Terms:**
- **LOD**: Level of Detail - System for rendering objects at different complexities
- **MFD**: Multi-Function Display - Cockpit display system
- **HUD**: Heads-Up Display - Transparent information overlay
- **JDYN**: Flight dynamics data format from original game
- **SAX**: Simple API for XML (adapted for IFF parsing) - Event-driven parsing

**Game Elements:**
- **Actor**: Any object in the game world (aircraft, buildings, etc.)
- **Entity**: 3D model representation of game objects
- **Area**: Terrain and environment definition
- **Mission**: Playable scenario with objectives
- **Gameflow**: Overall game progression and state management

**Development Tools:**
- **ImGui**: Dear ImGui - Immediate mode GUI library
- **vcpkg**: Microsoft's C++ package manager
- **CMake**: Cross-platform build system generator

### 9.2 References

**Technical Documentation:**
- Original RealSpace engine documentation and reverse engineering
- Strike Commander technical specifications and file format analysis
- Wing Commander technical specifications and asset structure
- OpenGL legacy rendering techniques and best practices
- SDL2 documentation for cross-platform development

**Academic References:**
- Flight simulation and aerodynamics modeling
- 3D graphics programming and rendering techniques
- Game engine architecture and design patterns
- Real-time systems and performance optimization

**Community Resources:**
- Wing Commander CIC (Combat Information Center) community archives
- Origin Systems historical documentation
- Retro gaming preservation community resources

### 9.3 Contributors and Acknowledgments

**Project Leadership:**
- The project is a community-driven effort to recreate and preserve the RealSpace engine

**Technical Contributions:**
- Reverse engineering of original game file formats
- 3D rendering system implementation
- Flight simulation and physics modeling
- Debug tools and development environment

**Community Support:**
- Wing Commander and Strike Commander gaming communities
- Retro gaming preservation enthusiasts
- Open source contributors and testers

**Historical Preservation:**
- This project serves educational and historical preservation purposes
- Maintains compatibility with original game assets
- Documents the technical achievements of Origin Systems' RealSpace engine

**Special Thanks:**
- Origin Systems for creating the original RealSpace engine
- The Wing Commander CIC community for documentation and support
- All contributors who have helped with reverse engineering and development

---

This technical documentation provides a comprehensive overview of the libRealSpace project's architecture, components, and implementation. It serves as a guide for developers working with the codebase and for understanding the inner workings of the RealSpace engine recreation.