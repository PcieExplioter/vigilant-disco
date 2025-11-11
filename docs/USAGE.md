# Usage Guide

Complete guide for using VisCheck in your project.

## Table of Contents

1. [Basic Usage](#basic-usage)
2. [Loading Geometry](#loading-geometry)
3. [Checking Visibility](#checking-visibility)
4. [File Format Examples](#file-format-examples)
5. [Performance Tips](#performance-tips)
6. [Troubleshooting](#troubleshooting)

## Basic Usage

### Step 1: Include the Header

```cpp
#include "VisCheck.h"
```

### Step 2: Create VisCheck Instance

```cpp
VisCheck visCheck;
```

### Step 3: Load Geometry

You must provide triangle data. See [Loading Geometry](#loading-geometry) section.

### Step 4: Check Visibility

```cpp
Vec3 point1(100.0f, 50.0f, 200.0f);
Vec3 point2(500.0f, 50.0f, 300.0f);

bool visible = visCheck.IsVisible(point1, point2);
if (visible) {
    // Line of sight is clear
} else {
    // Blocked by geometry
}
```

## Loading Geometry

### Method 1: Direct Loading (Recommended)

Load triangles directly from your own file format:

```cpp
std::vector<std::vector<TriangleCombined>> meshes;

// Your file loading code here
// For each mesh in your file:
std::vector<TriangleCombined> mesh;

// For each triangle in the mesh:
TriangleCombined tri;
tri.v0 = Vec3(vertex1.x, vertex1.y, vertex1.z);
tri.v1 = Vec3(vertex2.x, vertex2.y, vertex2.z);
tri.v2 = Vec3(vertex3.x, vertex3.y, vertex3.z);
mesh.push_back(tri);

meshes.push_back(mesh);

// Load into VisCheck
visCheck.LoadGeometry(meshes);
```

### Method 2: Using .opt File Format

If you have files in the .opt format:

```cpp
visCheck.LoadFromOptFile("path/to/geometry.opt");
```

The .opt format is a simple binary format:
- First 8 bytes: number of meshes (size_t)
- For each mesh:
  - 8 bytes: number of triangles (size_t)
  - For each triangle: 3 Vec3 structures (36 bytes total)

## Checking Visibility

### Basic Visibility Check

```cpp
Vec3 origin(0.0f, 100.0f, 0.0f);
Vec3 target(1000.0f, 100.0f, 0.0f);

bool visible = visCheck.IsVisible(origin, target);
```

### Multiple Checks

For checking multiple points efficiently:

```cpp
std::vector<Vec3> targets = { /* ... */ };
Vec3 camera(100.0f, 50.0f, 200.0f);

for (const auto& target : targets) {
    if (visCheck.IsVisible(camera, target)) {
        // Target is visible
    }
}
```

### Important Notes

- Points must be in the same coordinate system as your geometry
- The ray is cast from point1 to point2
- Returns false if geometry is not loaded
- Returns true if points are the same (distance < 0.001)

## File Format Examples

### Example: Loading from OBJ File

```cpp
std::vector<Vec3> vertices;
std::vector<std::vector<TriangleCombined>> meshes;
std::vector<TriangleCombined> mesh;

// Parse OBJ file (pseudocode)
// while reading file:
//     if line starts with "v ":
//         parse vertex and add to vertices
//     if line starts with "f ":
//         parse face indices and create triangle

// After parsing:
for (auto& face : faces) {
    TriangleCombined tri;
    tri.v0 = vertices[face.v0_index];
    tri.v1 = vertices[face.v1_index];
    tri.v2 = vertices[face.v2_index];
    mesh.push_back(tri);
}
meshes.push_back(mesh);

visCheck.LoadGeometry(meshes);
```

### Example: Loading from Custom Binary Format

```cpp
std::ifstream file("map.dat", std::ios::binary);
std::vector<std::vector<TriangleCombined>> meshes;

uint32_t numMeshes;
file.read(reinterpret_cast<char*>(&numMeshes), sizeof(uint32_t));

for (uint32_t i = 0; i < numMeshes; ++i) {
    uint32_t numTris;
    file.read(reinterpret_cast<char*>(&numTris), sizeof(uint32_t));
    
    std::vector<TriangleCombined> mesh;
    for (uint32_t j = 0; j < numTris; ++j) {
        TriangleCombined tri;
        file.read(reinterpret_cast<char*>(&tri.v0), sizeof(Vec3));
        file.read(reinterpret_cast<char*>(&tri.v1), sizeof(Vec3));
        file.read(reinterpret_cast<char*>(&tri.v2), sizeof(Vec3));
        mesh.push_back(tri);
    }
    meshes.push_back(mesh);
}

visCheck.LoadGeometry(meshes);
```

### Example: Procedural Geometry

```cpp
std::vector<std::vector<TriangleCombined>> meshes;
std::vector<TriangleCombined> groundMesh;

// Create ground plane
groundMesh.push_back(TriangleCombined(
    Vec3(-1000.0f, 0.0f, -1000.0f),
    Vec3(1000.0f, 0.0f, -1000.0f),
    Vec3(1000.0f, 0.0f, 1000.0f)
));
groundMesh.push_back(TriangleCombined(
    Vec3(-1000.0f, 0.0f, -1000.0f),
    Vec3(1000.0f, 0.0f, 1000.0f),
    Vec3(-1000.0f, 0.0f, 1000.0f)
));

meshes.push_back(groundMesh);
visCheck.LoadGeometry(meshes);
```

## Performance Tips

### BVH Caching

Building BVH trees can be slow for large meshes. Cache the BVH to disk:

```cpp
// First time: build and save
visCheck.LoadGeometry(meshes);
visCheck.SaveBVHToFile("cache.bvh");

// Later: load from cache (still need meshes for verification)
visCheck.LoadGeometry(meshes);
visCheck.LoadBVHFromFile("cache.bvh");
```

### Batch Visibility Checks

If checking many points from the same origin:

```cpp
Vec3 origin(100.0f, 50.0f, 200.0f);
std::vector<Vec3> targets = { /* ... */ };

// Check all at once
for (const auto& target : targets) {
    bool visible = visCheck.IsVisible(origin, target);
    // Process result
}
```

### Mesh Organization

Organize triangles into logical meshes. Each mesh gets its own BVH tree, which can improve performance for large scenes.

## Troubleshooting

### Geometry Not Loading

**Problem**: `LoadGeometry()` returns false

**Solutions**:
- Check that meshes vector is not empty
- Verify triangles have valid vertex data
- Ensure Vec3 values are finite (not NaN or Inf)

### Always Returns Blocked

**Problem**: `IsVisible()` always returns false

**Solutions**:
- Verify geometry is loaded: `visCheck.IsGeometryLoaded()`
- Check coordinate system matches between points and geometry
- Ensure points are not inside geometry (ray starts inside a triangle)

### Always Returns Visible

**Problem**: `IsVisible()` always returns true

**Solutions**:
- Verify geometry actually contains triangles
- Check triangle winding order (should be consistent)
- Ensure geometry is in the correct coordinate space

### Performance Issues

**Problem**: Visibility checks are slow

**Solutions**:
- Use BVH caching to avoid rebuilding trees
- Reduce number of triangles if possible
- Organize geometry into multiple meshes
- Consider spatial partitioning for very large scenes

### Coordinate System Mismatch

**Problem**: Results don't match expectations

**Solutions**:
- Verify coordinate system (left-handed vs right-handed)
- Check axis orientation (Y-up vs Z-up)
- Ensure units match (meters vs centimeters)
- Test with known geometry first

## Complete Example

```cpp
#include "VisCheck.h"
#include <iostream>
#include <vector>

int main() {
    VisCheck visCheck;
    
    // Create sample geometry
    std::vector<std::vector<TriangleCombined>> meshes;
    std::vector<TriangleCombined> mesh;
    
    // Ground plane
    mesh.push_back(TriangleCombined(
        Vec3(-1000.0f, 0.0f, -1000.0f),
        Vec3(1000.0f, 0.0f, -1000.0f),
        Vec3(1000.0f, 0.0f, 1000.0f)
    ));
    mesh.push_back(TriangleCombined(
        Vec3(-1000.0f, 0.0f, -1000.0f),
        Vec3(1000.0f, 0.0f, 1000.0f),
        Vec3(-1000.0f, 0.0f, 1000.0f)
    ));
    
    meshes.push_back(mesh);
    
    // Load geometry
    if (!visCheck.LoadGeometry(meshes)) {
        std::cerr << "Failed to load geometry" << std::endl;
        return 1;
    }
    
    // Test visibility
    Vec3 point1(0.0f, 100.0f, 0.0f);
    Vec3 point2(0.0f, 100.0f, 500.0f);
    
    bool visible = visCheck.IsVisible(point1, point2);
    std::cout << "Visibility: " << (visible ? "VISIBLE" : "BLOCKED") << std::endl;
    
    return 0;
}
```

## Advanced Usage

### Custom File Format Integration

Integrate with your existing asset pipeline:

1. Add a function to convert your format to `TriangleCombined`
2. Call `LoadGeometry()` with the converted data
3. Optionally cache BVH for faster subsequent loads

### Multi-Threading

VisCheck instances are not thread-safe. For multi-threaded use:

- Create separate VisCheck instances per thread
- Load the same geometry into each instance
- Each thread can check visibility independently

### Memory Management

- BVH trees are stored in memory
- Large meshes will use significant memory
- Consider unloading geometry when not needed
- BVH cache files are typically smaller than raw geometry

## Credits

This implementation is based on concepts from:
- [cs2-map-parser](https://github.com/AtomicBool/cs2-map-parser) - CS2 map parsing and visibility checking implementation

