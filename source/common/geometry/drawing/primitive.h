#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <string>

//What kind of drawable object is this?
enum class PrimitiveType : uint8_t {
    Point,
    Polyline,
    Polygon
};

// Minimal base class: ID + type + user-facing name
class Primitive {
public:
    Primitive(uint32_t id, PrimitiveType type)
            : id_(id), type_(type) {}

    virtual ~Primitive() = default;

    uint32_t getID() const { return id_; }
    PrimitiveType getType() const { return type_; }

    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

private:
    uint32_t id_;
    PrimitiveType type_;
    std::string name_;
};

// Concrete primitives
// Storing positions as unit vectors on the sphere: glm::vec3 (x,y,z)
// Storing color as RGBA: glm::vec4 (r,g,b,a) in [0,1]

// Point: single vertex + optional size of the point
class PointPrimitive final : public Primitive {
public:
    explicit PointPrimitive(uint32_t id)
        : Primitive(id, PrimitiveType::Point) {}

    glm::vec3 p {0.f, 0.f, 1.f};
    glm::vec4 color {1.f, 1.f, 1.f, 1.f};
    float size = 0.007f; //optional, ask Nathan if he thinks we need this later
};

//Polyline: list of vertices + line width
class PolylinePrimitive final : public Primitive {
public:
    explicit PolylinePrimitive(uint32_t id)
        : Primitive(id, PrimitiveType::Polyline) {}

    std::vector<glm::vec3> verts;
    glm::vec4 color {1.f, 1.f, 1.f, 1.f};
    float width = 0.007f;
    bool closed = false; // optional, default false, ask Nathan if he thinks we need this later
};

//Polygon: list of vertices (implicitly closed)
class PolygonPrimitive final : public Primitive {
public:
    explicit PolygonPrimitive(uint32_t id)
        : Primitive(id, PrimitiveType::Polygon) {}

    std::vector<glm::vec3> verts;
    glm::vec4 color {1.f, 1.f, 1.f, 1.f};
};