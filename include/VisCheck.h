#pragma once
#include "Types.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <limits>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

struct AABB {
    Vec3 min;
    Vec3 max;

    bool RayIntersects(const Vec3& rayOrigin, const Vec3& rayDir) const;
};

struct TriangleCombined {
    Vec3 v0, v1, v2;

    TriangleCombined() = default;
    TriangleCombined(const Vec3& v0_, const Vec3& v1_, const Vec3& v2_)
        : v0(v0_), v1(v1_), v2(v2_) {}

    AABB ComputeAABB() const;
};

struct BVHNode {
    AABB bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    std::vector<TriangleCombined> triangles;

    bool IsLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

class VisCheck {
private:
    static constexpr size_t LEAF_THRESHOLD = 4;
    
    std::vector<std::vector<TriangleCombined>> meshes;
    std::vector<std::unique_ptr<BVHNode>> bvhNodes;
    bool geometryLoaded;
    
    std::unique_ptr<BVHNode> BuildBVH(const std::vector<TriangleCombined>& tris);
    bool IntersectBVH(const BVHNode* node, const Vec3& rayOrigin, const Vec3& rayDir, float maxDistance, float& hitDistance) const;
    bool RayIntersectsTriangle(const Vec3& rayOrigin, const Vec3& rayDir,
        const TriangleCombined& triangle, float& t) const;
    
    bool LoadOptFile(const std::string& filePath);
    bool SaveBVHCache(const std::string& cachePath);
    bool LoadBVHCache(const std::string& cachePath, size_t& meshCount);
    void SerializeBVHNode(std::ofstream& out, const BVHNode* node);
    std::unique_ptr<BVHNode> DeserializeBVHNode(std::ifstream& in);

public:
    VisCheck();
    ~VisCheck();
    
    bool LoadGeometry(const std::vector<std::vector<TriangleCombined>>& geometryMeshes);
    bool LoadFromOptFile(const std::string& filePath);
    bool SaveBVHToFile(const std::string& cachePath);
    bool LoadBVHFromFile(const std::string& cachePath);
    bool IsVisible(const Vec3& point1, const Vec3& point2);
    bool IsGeometryLoaded() const { return geometryLoaded; }
};

