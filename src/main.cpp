#include "VisCheck.h"
#include "Debug.h"
#include <iostream>
#include <vector>

int main() {
    VisCheck visCheck;
    
    // Create some test geometry
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
    
    // Wall
    mesh.push_back(TriangleCombined(
        Vec3(-100.0f, 0.0f, 500.0f),
        Vec3(100.0f, 0.0f, 500.0f),
        Vec3(100.0f, 1000.0f, 500.0f)
    ));
    mesh.push_back(TriangleCombined(
        Vec3(-100.0f, 0.0f, 500.0f),
        Vec3(100.0f, 1000.0f, 500.0f),
        Vec3(-100.0f, 1000.0f, 500.0f)
    ));
    
    meshes.push_back(mesh);
    
    if (!visCheck.LoadGeometry(meshes)) {
        std::cout << "Failed to load geometry" << std::endl;
        return 1;
    }
    
    // Test visibility
    Vec3 p1(0.0f, 100.0f, 0.0f);
    Vec3 p2(0.0f, 100.0f, 200.0f);
    
    bool visible = visCheck.IsVisible(p1, p2);
    std::cout << "Test 1: " << (visible ? "VISIBLE" : "BLOCKED") << std::endl;
    
    Vec3 p3(0.0f, 100.0f, 0.0f);
    Vec3 p4(0.0f, 100.0f, 1000.0f);
    
    visible = visCheck.IsVisible(p3, p4);
    std::cout << "Test 2: " << (visible ? "VISIBLE" : "BLOCKED") << std::endl;
    
    return 0;
}

