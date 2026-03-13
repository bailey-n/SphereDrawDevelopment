#pragma once

#include <fstream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>

#include "project.h"
#include "primitive_id_manager.h"

using json = nlohmann::json;

// Helper for timestamping
static std::string utc_now_iso8601() {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    char buf[32];
    std::snprintf(buf, sizeof(buf),
            "%04d-%02d-%02dT%02d:%02d:%02dZ",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    return std::string(buf);
}


// glm <-> json helpers

inline json vec3_to_json(const glm::vec3& v){
    return json::array({v.x, v.y, v.z});
}
inline glm::vec3 json_to_vec3(const json& j){
    return glm::vec3(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>());
}

inline json vec4_to_json(const glm::vec4& c){
    return json::array({c.r, c.g, c.b, c.a});
}
inline glm::vec4 json_to_vec4(const json& j){
    return glm::vec4(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>(), j.at(3).get<float>());
}

// Primitive <-> string
inline std::string primTypeToString(PrimitiveType t){
    switch(t){
        case PrimitiveType::Point: return "point";
        case PrimitiveType::Polyline: return "polyline";
        case PrimitiveType::Polygon: return "polygon";
        default: return "unknown";
    }
}
inline PrimitiveType primTypeFromString(const std::string& s){
    if(s == "point") return PrimitiveType::Point;
    if(s == "polyline") return PrimitiveType::Polyline;
    if(s == "polygon") return PrimitiveType::Polygon;
    throw std::runtime_error("Unkown primitive type: " + s);
}

//Primitive -> json
inline json primitiveToJson(const Primitive& p){
    json j;
    j["id"] = p.getID();
    j["type"] = primTypeToString(p.getType());
    j["name"] = p.getName();

    //For each derived type, write the fields that recreate it
    if (p.getType() == PrimitiveType::Point){
        auto& pt = dynamic_cast<const PointPrimitive&>(p);
        j["color_rgba"] = vec4_to_json(pt.color);
        j["size"] = pt.size;
        j["p"] = vec3_to_json(pt.p);
    } else if (p.getType() == PrimitiveType::Polyline){
        auto& line = dynamic_cast<const PolylinePrimitive&>(p);
        j["color_rgba"] = vec4_to_json(line.color);
        j["width"] = line.width;
        j["closed"] = line.closed;
        j["verts"] = json::array();
        for (const auto& v : line.verts) j["verts"].push_back(vec3_to_json(v));
    } else if (p.getType() == PrimitiveType::Polygon){
        auto& poly = dynamic_cast<const PolygonPrimitive&>(p);
        j["color_rgba"] = vec4_to_json(poly.color);
        j["verts"] = json::array();
        for (const auto& v : poly.verts) j["verts"].push_back(vec3_to_json(v));
    } else{
        throw std::runtime_error("primitiveToJson: Unsupported primitive type");
    }

    return j;
}

// json -> Primitive (factory)
inline std::unique_ptr<Primitive> primitiveFromJson(const json& j){
    uint32_t id = j.at("id").get<uint32_t>();
    PrimitiveType t = primTypeFromString(j.at("type").get<std::string>());

    if (t == PrimitiveType::Point){
        auto p = std::make_unique<PointPrimitive>(id);
        p->setName(j.value("name", std::string("")));
        p->color = json_to_vec4(j.at("color_rgba"));
        p->size = j.value("size", 0.007f);
        p->p = json_to_vec3(j.at("p"));
        return p;
    }

    if (t == PrimitiveType::Polyline){
        auto p = std::make_unique<PolylinePrimitive>(id);
        p->setName(j.value("name", std::string("")));
        p->color = json_to_vec4(j.at("color_rgba"));
        p->width = j.value("width", 0.007f);
        p->closed = j.value("closed", false);
        for (const auto& vj : j.at("verts")) p->verts.push_back(json_to_vec3(vj));
        return p;
    }

    if (t == PrimitiveType::Polygon){
        auto p = std::make_unique<PolygonPrimitive>(id);
        p->setName(j.value("name", std::string("")));
        p->color = json_to_vec4(j.at("color_rgba"));
        for (const auto& vj : j.at("verts")) p->verts.push_back(json_to_vec3(vj));
        return p;
    }

    throw std::runtime_error("primitiveFromJson: Unsupported primitive type");
}

// Layer <-> json
inline json layerToJson(const Layer& layer){
    json j;
    j["id"] = layer.id;
    j["name"] = layer.name;
    j["visible"] = layer.visible;
    j["locked"] = layer.locked;
    j["primitive_ids"] = layer.primitiveIDs;
    return j;
}

inline Layer layerFromJson(const json& j){
    Layer layer;
    layer.id = j.at("id").get<uint32_t>();
    layer.name = j.value("name", std::string("Layer"));
    layer.visible = j.value("visible", true);
    layer.locked = j.value("locked", false);
    if(j.contains("primitive_ids")){
        layer.primitiveIDs = j.at("primitive_ids").get<std::vector<uint32_t>>();
    }
    return layer;
}

//Project <-> file
inline void SaveProjectToFile(Project& project, const std::string& filepath){
    // Set created UTC once on first save, then set modified on every save
    if (project.createdUtc.empty()) {
        project.createdUtc = utc_now_iso8601();
    }
    project.modifiedUtc = utc_now_iso8601();

    json j;
    j["format"] = "spheredraw_project";
    j["format_version"] = project.formatVersion;
    j["created_utc"] = project.createdUtc;
    j["modified_utc"] = project.modifiedUtc;

    //Layers define ordering
    j["layers"] = json::array();
    for(const auto& layer : project.layers){
        j["layers"].push_back(layerToJson(layer));
    }

    //primitives contain geometry and styling
    j["primitives"] = json::array();
    for(const auto& kv : project.primitives){
        j["primitives"].push_back(primitiveToJson(*kv.second));
    }

    // make sure the folder exists
    std::filesystem::path p(filepath);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());


    std::ofstream out(filepath);
    if (!out) throw std::runtime_error("Failed to open file for writing: " + filepath);
    out << j.dump(2);
}

inline Project LoadProjectFromFile(const std::string& filepath){
    std::ifstream in(filepath);
    if(!in) throw std::runtime_error("Failed to open file for reading: " + filepath);

    json j;
    in >> j;

    if(j.value("format", "") != "spheredraw_project"){
        throw std::runtime_error("Not a SphereDraw project file (format mismatch).");
    }

    PrimIDManager::reset();

    Project project;
    project.formatVersion = j.value("format_version", 1);
    project.createdUtc = j.value("created_utc", "");
    project.modifiedUtc = j.value("modified_utc", "");

    //Layers
    project.layers.clear();
    if(j.contains("layers")){
        for(const auto& lj : j.at("layers")){
            project.layers.push_back(layerFromJson(lj));
        }
    }
    project.ensureDefaultLayer();

    //Primitives
    project.primitives.clear();
    if (j.contains("primitives")) {
        for (const auto& pj : j.at("primitives")) {
            // Reserve the ID first so collisions are caught immediately
            uint32_t id = pj.at("id").get<uint32_t>();
            if (!PrimIDManager::add_id(id)) {
                throw std::runtime_error("Duplicate primitive id in file: " + std::to_string(id));
            }

            auto prim = primitiveFromJson(pj);
            project.primitives[prim->getID()] = std::move(prim);
        }
    }

    //Validate: layer references exist
    for(auto& layer : project.layers){
        std::vector<uint32_t> filtered;
        filtered.reserve(layer.primitiveIDs.size());
        for(uint32_t pid : layer.primitiveIDs){
            if(project.primitives.find(pid) != project.primitives.end()){
                filtered.push_back(pid);
            }
        }
        layer.primitiveIDs = std::move(filtered);
    }

    return project;
}