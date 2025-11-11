# VisCheck Standalone

Ray-traced visibility checking using BVH (Bounding Volume Hierarchy) acceleration. Pure algorithm implementation with no file loading logic - you provide the geometry data.

## What It Does

Checks if two 3D points have line of sight by casting a ray through triangle geometry. Uses BVH trees for fast spatial queries.

## Requirements

- Visual Studio 2022 (or compatible C++17 compiler)
- Windows 10/11 (or Linux/macOS with GCC/Clang)
- C++17 standard library support

## Quick Start

```cpp
#include "VisCheck.h"

int main() {
    VisCheck visCheck;
    
    // 1. Load your geometry
    std::vector<std::vector<TriangleCombined>> meshes;
    // ... load triangles from your file format ...
    visCheck.LoadGeometry(meshes);
    
    // 2. Check visibility
    Vec3 point1(0.0f, 10.0f, 0.0f);
    Vec3 point2(200.0f, 10.0f, 0.0f);
    bool visible = visCheck.IsVisible(point1, point2);
    
    return 0;
}
```

See `main.cpp` for a complete working example.

## API

### Core Methods

**LoadGeometry(meshes)** - Load triangle data. Call this first.

**IsVisible(point1, point2)** - Check if two points have line of sight. Returns true if visible, false if blocked.

**IsGeometryLoaded()** - Check if geometry is loaded.

### Optional Methods

**LoadFromOptFile(path)** - Load from .opt file format (example implementation).

**SaveBVHToFile(path)** - Save BVH cache for faster loading.

**LoadBVHFromFile(path)** - Load BVH cache.

## Building

### Visual Studio 2022 (Recommended)

1. Open `VisCheckStandalone.sln` in Visual Studio 2022
2. Select configuration (Debug or Release) and platform (x64)
3. Build the solution (F7 or Build → Build Solution)
4. Run the executable from `x64/Debug/` or `x64/Release/`

### Command Line (MSVC)

```cmd
cl /std:c++17 /EHsc /O2 *.cpp /Fe:vischeck.exe
```

### Command Line (GCC/Clang)

```bash
g++ -std=c++17 -O2 *.cpp -o vischeck
```

## Project Structure

```
VisCheckStandalone/
├── VisCheckStandalone.sln          # Visual Studio solution
├── VisCheckStandalone.vcxproj      # Visual Studio project
├── main.cpp                        # Example usage
├── VisCheck.h/cpp                  # Core algorithm
├── Types.h                         # Vec3 definition
├── Debug.h                         # Logging macros
├── Parser.h/cpp                    # Optional .vphys parser
├── OptimizedGeometry.h/cpp         # Optional .opt format handler
├── README.md                       # This file
└── USAGE.md                        # Detailed usage guide
```

## How It Works

1. You provide triangle meshes via `LoadGeometry()`
2. BVH trees are built automatically for fast queries
3. `IsVisible()` casts a ray and checks for triangle intersections
4. Uses Möller-Trumbore algorithm for ray-triangle intersection

## Implementation

You must implement your own file loading. Convert your geometry to `std::vector<std::vector<TriangleCombined>>` format where each inner vector is a mesh containing triangles.

Each triangle is defined by three Vec3 vertices (v0, v1, v2).

See `main.cpp` for a complete example.

## Usage

For detailed usage instructions, see [USAGE.md](USAGE.md).

Basic usage:
1. Implement your own file loading to read triangles
2. Convert to `std::vector<std::vector<TriangleCombined>>` format
3. Call `visCheck.LoadGeometry(meshes)`
4. Use `visCheck.IsVisible(point1, point2)` to check line of sight

## Credits

This implementation is based on concepts from:
- [cs2-map-parser](https://github.com/AtomicBool/cs2-map-parser) - CS2 map parsing and visibility checking implementation

## License

MIT License - see [LICENSE](LICENSE) file for details.
