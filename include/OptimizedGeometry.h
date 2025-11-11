#pragma once
#include <string>
#include <vector>

// Forward declaration
struct TriangleCombined;

// OptimizedGeometry class for loading and saving .opt files
// Standalone version - no game dependencies
class OptimizedGeometry {
public:
    // Meshes loaded from file (vector of triangle lists)
    std::vector<std::vector<TriangleCombined>> meshes;

    // Load optimized geometry from .opt file
    bool LoadFromFile(const std::string& optimizedFile);

    // Create optimized file from raw .vphys file
    // Uses Parser to parse the .vphys file, then saves as .opt
    bool CreateOptimizedFile(const std::string& rawFile, const std::string& optimizedFile);
};

