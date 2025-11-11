#include "VisCheck.h"
#include "Debug.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cctype>
#include <functional>

namespace Vec3Helpers {
    inline Vec3 Subtract(const Vec3& a, const Vec3& b) {
        return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
    }
    
    inline float Dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    
    inline Vec3 Cross(const Vec3& a, const Vec3& b) {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
    
    inline float LengthSquared(const Vec3& v) {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }
}

bool AABB::RayIntersects(const Vec3& rayOrigin, const Vec3& rayDir) const {
    float tmin = std::numeric_limits<float>::lowest();
    float tmax = std::numeric_limits<float>::max();

    const float* rayOriginArr = &rayOrigin.x;
    const float* rayDirArr = &rayDir.x;
    const float* minArr = &min.x;
    const float* maxArr = &max.x;

    for (int i = 0; i < 3; ++i) {
        float invDir = 1.0f / rayDirArr[i];
        float t0 = (minArr[i] - rayOriginArr[i]) * invDir;
        float t1 = (maxArr[i] - rayOriginArr[i]) * invDir;

        if (invDir < 0.0f) std::swap(t0, t1);
        tmin = std::max(tmin, t0);
        tmax = std::min(tmax, t1);
    }

    return tmax >= tmin && tmax >= 0;
}

AABB TriangleCombined::ComputeAABB() const {
    Vec3 min_point, max_point;

    min_point.x = std::min({ v0.x, v1.x, v2.x });
    min_point.y = std::min({ v0.y, v1.y, v2.y });
    min_point.z = std::min({ v0.z, v1.z, v2.z });

    max_point.x = std::max({ v0.x, v1.x, v2.x });
    max_point.y = std::max({ v0.y, v1.y, v2.y });
    max_point.z = std::max({ v0.z, v1.z, v2.z });

    return { min_point, max_point };
}

VisCheck::VisCheck() : geometryLoaded(false) {
}

VisCheck::~VisCheck() {
}

std::unique_ptr<BVHNode> VisCheck::BuildBVH(const std::vector<TriangleCombined>& tris) {
    auto node = std::make_unique<BVHNode>();

    if (tris.empty()) return node;
    
    AABB bounds = tris[0].ComputeAABB();
    for (size_t i = 1; i < tris.size(); ++i) {
        AABB triAABB = tris[i].ComputeAABB();
        bounds.min.x = std::min(bounds.min.x, triAABB.min.x);
        bounds.min.y = std::min(bounds.min.y, triAABB.min.y);
        bounds.min.z = std::min(bounds.min.z, triAABB.min.z);
        bounds.max.x = std::max(bounds.max.x, triAABB.max.x);
        bounds.max.y = std::max(bounds.max.y, triAABB.max.y);
        bounds.max.z = std::max(bounds.max.z, triAABB.max.z);
    }
    node->bounds = bounds;
    
    if (tris.size() <= LEAF_THRESHOLD) {
        node->triangles = tris;
        return node;
    }
    
    Vec3 diff = Vec3Helpers::Subtract(bounds.max, bounds.min);
    int axis = (diff.x > diff.y && diff.x > diff.z) ? 0 : ((diff.y > diff.z) ? 1 : 2);
    
    std::vector<TriangleCombined> sortedTris = tris;
    std::sort(sortedTris.begin(), sortedTris.end(), [axis](const TriangleCombined& a, const TriangleCombined& b) {
        AABB aabbA = a.ComputeAABB();
        AABB aabbB = b.ComputeAABB();
        float centerA, centerB;
        if (axis == 0) {
            centerA = (aabbA.min.x + aabbA.max.x) / 2.0f;
            centerB = (aabbB.min.x + aabbB.max.x) / 2.0f;
        }
        else if (axis == 1) {
            centerA = (aabbA.min.y + aabbA.max.y) / 2.0f;
            centerB = (aabbB.min.y + aabbB.max.y) / 2.0f;
        }
        else {
            centerA = (aabbA.min.z + aabbA.max.z) / 2.0f;
            centerB = (aabbB.min.z + aabbB.max.z) / 2.0f;
        }
        return centerA < centerB;
    });

    size_t mid = sortedTris.size() / 2;
    std::vector<TriangleCombined> leftTris(sortedTris.begin(), sortedTris.begin() + mid);
    std::vector<TriangleCombined> rightTris(sortedTris.begin() + mid, sortedTris.end());

    node->left = BuildBVH(leftTris);
    node->right = BuildBVH(rightTris);

    return node;
}

bool VisCheck::IntersectBVH(const BVHNode* node, const Vec3& rayOrigin, const Vec3& rayDir, float maxDistance, float& hitDistance) const {
    if (!node->bounds.RayIntersects(rayOrigin, rayDir)) {
        return false;
    }

    bool hit = false;
    if (node->IsLeaf()) {
        for (const auto& tri : node->triangles) {
            float t;
            if (RayIntersectsTriangle(rayOrigin, rayDir, tri, t)) {
                if (t < maxDistance && t < hitDistance) {
                    hitDistance = t;
                    hit = true;
                }
            }
        }
    } else {
        if (node->left) {
            hit |= IntersectBVH(node->left.get(), rayOrigin, rayDir, maxDistance, hitDistance);
        }
        if (node->right) {
            hit |= IntersectBVH(node->right.get(), rayOrigin, rayDir, maxDistance, hitDistance);
        }
    }
    return hit;
}

bool VisCheck::RayIntersectsTriangle(const Vec3& rayOrigin, const Vec3& rayDir,
    const TriangleCombined& triangle, float& t) const {
    const float EPSILON = 1e-7f;

    Vec3 edge1 = Vec3Helpers::Subtract(triangle.v1, triangle.v0);
    Vec3 edge2 = Vec3Helpers::Subtract(triangle.v2, triangle.v0);
    Vec3 h = Vec3Helpers::Cross(rayDir, edge2);
    float a = Vec3Helpers::Dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false;

    float f = 1.0f / a;
    Vec3 s = Vec3Helpers::Subtract(rayOrigin, triangle.v0);
    float u = f * Vec3Helpers::Dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    Vec3 q = Vec3Helpers::Cross(s, edge1);
    float v = f * Vec3Helpers::Dot(rayDir, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * Vec3Helpers::Dot(edge2, q);

    return (t > EPSILON);
}

bool VisCheck::LoadGeometry(const std::vector<std::vector<TriangleCombined>>& geometryMeshes) {
    if (geometryMeshes.empty()) {
        DEBUG_LOG_ERROR("[VisCheck] No geometry meshes provided");
        return false;
    }
    
    meshes = geometryMeshes;
    bvhNodes.clear();
    
    for (size_t i = 0; i < meshes.size(); ++i) {
        if (meshes[i].empty()) {
            DEBUG_LOG_WARNING("[VisCheck] Mesh " << i << " is empty, skipping");
            continue;
        }
        
        DEBUG_LOG_INFO("[VisCheck] Building BVH for mesh " << i << " with " << meshes[i].size() << " triangles...");
        bvhNodes.push_back(BuildBVH(meshes[i]));
    }
    
    geometryLoaded = (meshes.size() > 0 && bvhNodes.size() > 0);
    
    if (geometryLoaded) {
        DEBUG_LOG_INFO("[VisCheck] Successfully loaded geometry with " << meshes.size() << " meshes and " << bvhNodes.size() << " BVH trees");
    }
    
    return geometryLoaded;
}

bool VisCheck::LoadFromOptFile(const std::string& filePath) {
    try {
        std::ifstream in(filePath, std::ios::binary);
        if (!in) {
            DEBUG_LOG_ERROR("[VisCheck] Failed to open file: " << filePath);
            return false;
        }
        
        meshes.clear();
        bvhNodes.clear();
        
        size_t numMeshes;
        in.read(reinterpret_cast<char*>(&numMeshes), sizeof(size_t));
        
        if (numMeshes == 0) {
            DEBUG_LOG_WARNING("[VisCheck] File has 0 meshes");
            in.close();
            return false;
        }
        
        DEBUG_LOG_INFO("[VisCheck] Loading " << numMeshes << " meshes from file...");
        
        for (size_t i = 0; i < numMeshes; ++i) {
            size_t numTris;
            in.read(reinterpret_cast<char*>(&numTris), sizeof(size_t));
            
            if (numTris == 0) {
                DEBUG_LOG_WARNING("[VisCheck] Mesh " << i << " has 0 triangles, skipping");
                continue;
            }
            
            std::vector<TriangleCombined> mesh;
            mesh.resize(numTris);
            
            for (size_t j = 0; j < numTris; ++j) {
                in.read(reinterpret_cast<char*>(&mesh[j].v0), sizeof(Vec3));
                in.read(reinterpret_cast<char*>(&mesh[j].v1), sizeof(Vec3));
                in.read(reinterpret_cast<char*>(&mesh[j].v2), sizeof(Vec3));
            }
            
            meshes.push_back(mesh);
            DEBUG_LOG_INFO("[VisCheck] Building BVH for mesh " << i << " with " << numTris << " triangles...");
            bvhNodes.push_back(BuildBVH(mesh));
        }
        
        in.close();
        geometryLoaded = (meshes.size() > 0 && bvhNodes.size() > 0);
        
        if (geometryLoaded) {
            DEBUG_LOG_INFO("[VisCheck] Successfully loaded geometry with " << meshes.size() << " meshes and " << bvhNodes.size() << " BVH trees");
        }
        
        return geometryLoaded;
    } catch (const std::exception& e) {
        DEBUG_LOG_ERROR("[VisCheck] Exception loading file: " << e.what());
        return false;
    } catch (...) {
        DEBUG_LOG_ERROR("[VisCheck] Unknown exception loading file");
        return false;
    }
}

bool VisCheck::LoadOptFile(const std::string& filePath) {
    return LoadFromOptFile(filePath);
}

void VisCheck::SerializeBVHNode(std::ofstream& out, const BVHNode* node) {
    if (!node) {
        bool isNull = true;
        out.write(reinterpret_cast<const char*>(&isNull), sizeof(bool));
        return;
    }
    
    bool isNull = false;
    out.write(reinterpret_cast<const char*>(&isNull), sizeof(bool));
    
    out.write(reinterpret_cast<const char*>(&node->bounds.min), sizeof(Vec3));
    out.write(reinterpret_cast<const char*>(&node->bounds.max), sizeof(Vec3));
    
    bool isLeaf = node->IsLeaf();
    out.write(reinterpret_cast<const char*>(&isLeaf), sizeof(bool));
    
    if (isLeaf) {
        size_t numTris = node->triangles.size();
        out.write(reinterpret_cast<const char*>(&numTris), sizeof(size_t));
        for (const auto& tri : node->triangles) {
            out.write(reinterpret_cast<const char*>(&tri.v0), sizeof(Vec3));
            out.write(reinterpret_cast<const char*>(&tri.v1), sizeof(Vec3));
            out.write(reinterpret_cast<const char*>(&tri.v2), sizeof(Vec3));
        }
    } else {
        SerializeBVHNode(out, node->left.get());
        SerializeBVHNode(out, node->right.get());
    }
}

std::unique_ptr<BVHNode> VisCheck::DeserializeBVHNode(std::ifstream& in) {
    bool isNull;
    in.read(reinterpret_cast<char*>(&isNull), sizeof(bool));
    
    if (isNull) {
        return nullptr;
    }
    
    auto node = std::make_unique<BVHNode>();
    
    in.read(reinterpret_cast<char*>(&node->bounds.min), sizeof(Vec3));
    in.read(reinterpret_cast<char*>(&node->bounds.max), sizeof(Vec3));
    
    bool isLeaf;
    in.read(reinterpret_cast<char*>(&isLeaf), sizeof(bool));
    
    if (isLeaf) {
        size_t numTris;
        in.read(reinterpret_cast<char*>(&numTris), sizeof(size_t));
        node->triangles.resize(numTris);
        for (size_t i = 0; i < numTris; ++i) {
            in.read(reinterpret_cast<char*>(&node->triangles[i].v0), sizeof(Vec3));
            in.read(reinterpret_cast<char*>(&node->triangles[i].v1), sizeof(Vec3));
            in.read(reinterpret_cast<char*>(&node->triangles[i].v2), sizeof(Vec3));
        }
    } else {
        node->left = DeserializeBVHNode(in);
        node->right = DeserializeBVHNode(in);
    }
    
    return node;
}

bool VisCheck::SaveBVHToFile(const std::string& cachePath) {
    return SaveBVHCache(cachePath);
}

bool VisCheck::SaveBVHCache(const std::string& cachePath) {
    try {
        std::ofstream out(cachePath, std::ios::binary);
        if (!out) {
            DEBUG_LOG_ERROR("[VisCheck] Failed to create BVH cache file: " << cachePath);
            return false;
        }
        
        const uint32_t CACHE_VERSION = 1;
        out.write(reinterpret_cast<const char*>(&CACHE_VERSION), sizeof(uint32_t));
        
        size_t numMeshes = meshes.size();
        out.write(reinterpret_cast<const char*>(&numMeshes), sizeof(size_t));
        
        for (const auto& mesh : meshes) {
            size_t numTris = mesh.size();
            out.write(reinterpret_cast<const char*>(&numTris), sizeof(size_t));
        }
        
        for (const auto& bvhRoot : bvhNodes) {
            SerializeBVHNode(out, bvhRoot.get());
        }
        
        out.close();
        return true;
    } catch (const std::exception& e) {
        DEBUG_LOG_ERROR("[VisCheck] Exception saving BVH cache: " << e.what());
        return false;
    } catch (...) {
        DEBUG_LOG_ERROR("[VisCheck] Unknown exception saving BVH cache");
        return false;
    }
}

// Optional: Load BVH cache from file (example implementation)
bool VisCheck::LoadBVHFromFile(const std::string& cachePath) {
    size_t meshCount = 0;
    if (LoadBVHCache(cachePath, meshCount)) {
        if (meshCount == meshes.size()) {
            return true;
        }
    }
    return false;
}

bool VisCheck::LoadBVHCache(const std::string& cachePath, size_t& meshCount) {
    try {
        std::ifstream in(cachePath, std::ios::binary);
        if (!in) {
            DEBUG_LOG_ERROR("[VisCheck] Failed to open BVH cache file: " << cachePath);
            return false;
        }
        
        uint32_t version;
        in.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));
        if (version != 1) {
            DEBUG_LOG_WARNING("[VisCheck] BVH cache version mismatch (expected 1, got " << version << ")");
            in.close();
            return false;
        }
        
        size_t numMeshes;
        in.read(reinterpret_cast<char*>(&numMeshes), sizeof(size_t));
        meshCount = numMeshes;
        
        if (numMeshes == 0) {
            DEBUG_LOG_WARNING("[VisCheck] BVH cache has 0 meshes");
            in.close();
            return false;
        }
        
        std::vector<size_t> triangleCounts;
        triangleCounts.resize(numMeshes);
        for (size_t i = 0; i < numMeshes; ++i) {
            in.read(reinterpret_cast<char*>(&triangleCounts[i]), sizeof(size_t));
        }
        
        bvhNodes.clear();
        for (size_t i = 0; i < numMeshes; ++i) {
            auto bvhRoot = DeserializeBVHNode(in);
            if (!bvhRoot) {
                DEBUG_LOG_ERROR("[VisCheck] Failed to deserialize BVH tree " << i);
                in.close();
                return false;
            }
            bvhNodes.push_back(std::move(bvhRoot));
        }
        
        meshes.clear();
        meshes.resize(numMeshes);
        for (size_t i = 0; i < bvhNodes.size(); ++i) {
            meshes[i].reserve(triangleCounts[i]);
            std::function<void(const BVHNode*)> extractTriangles = [&](const BVHNode* node) {
                if (!node) return;
                if (node->IsLeaf()) {
                    meshes[i].insert(meshes[i].end(), node->triangles.begin(), node->triangles.end());
                } else {
                    extractTriangles(node->left.get());
                    extractTriangles(node->right.get());
                }
            };
            extractTriangles(bvhNodes[i].get());
        }
        
        in.close();
        return true;
    } catch (const std::exception& e) {
        DEBUG_LOG_ERROR("[VisCheck] Exception loading BVH cache: " << e.what());
        return false;
    } catch (...) {
        DEBUG_LOG_ERROR("[VisCheck] Unknown exception loading BVH cache");
        return false;
    }
}


// Check visibility between two points
bool VisCheck::IsVisible(const Vec3& point1, const Vec3& point2) {
    if (!geometryLoaded || bvhNodes.empty()) {
        static bool logged = false;
        if (!logged) {
            DEBUG_LOG_WARNING("[VisCheck] Geometry not loaded or BVH empty, returning false for visibility");
            logged = true;
        }
        return false;
    }
    
    Vec3 rayDir = Vec3Helpers::Subtract(point2, point1);
    float distance = std::sqrt(Vec3Helpers::LengthSquared(rayDir));
    
    if (distance < 0.001f) {
        return true;
    }
    
    rayDir.x /= distance;
    rayDir.y /= distance;
    rayDir.z /= distance;
    
    float hitDistance = std::numeric_limits<float>::max();
    
    for (const auto& bvhRoot : bvhNodes) {
        if (bvhRoot && IntersectBVH(bvhRoot.get(), point1, rayDir, distance, hitDistance)) {
            if (hitDistance < distance) {
                return false;
            }
        }
    }
    
    return true;
}


