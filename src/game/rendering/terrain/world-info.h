#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include <glm/glm.hpp>
#include "picojson/picojson.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../../engine/rendering/materials/material.h"
#include "../../../engine/rendering/terrain/chunk-structs.h"
#include "../../../engine/rendering/terrain/chunk-generator.h"

enum class WorldType
{
    BAKED,
    GENERATED
};

struct FaceInfo
{
    WorldType   Type       { WorldType::GENERATED };
    std::string TilePath;
    std::string OriginTile;
};

struct WorldInfo
{
    glm::vec3   Origin      { 0.0f };
    glm::quat   Orientation { 1.0f, 0.0f, 0.0f, 0.0f };
    float       Radius      { 0.0f };
    float       Edge        { 0.0f };
    float       Thickness   { 0.0f };

    WorldType   Type       { WorldType::GENERATED };
    std::string TilePath;
    std::string OriginTile;

    glm::vec3 ToLocalCoords(const glm::vec3& worldCoords) const
    {
        return glm::conjugate(Orientation) * (worldCoords - Origin);
    }

    glm::mat4 Transform() const
    {
        return glm::translate(glm::mat4(1.0f), Origin) * glm::mat4_cast(Orientation);
    }

    bool ChunkOnDisk(const glm::ivec2& coord, float regionSize) const
    {
        const float minX = coord.x * regionSize;
        const float maxX = minX + regionSize;
        const float minZ = coord.y * regionSize;
        const float maxZ = minZ + regionSize;

        const float x = std::clamp(0.0f, minX, maxX);
        const float z = std::clamp(0.0f, minZ, maxZ);

        return x * x + z * z <= Radius * Radius;
    }

};

struct DiskWorldInfo
{
    glm::vec3   Origin      { 0.0f };
    glm::quat   Orientation { 1.0f, 0.0f, 0.0f, 0.0f };
    float       Radius      { 0.0f };
    float       Edge        { 0.0f };
    float       Thickness   { 0.0f };
    FaceInfo    FaceA;
    std::optional<FaceInfo> FaceB;

    std::vector<WorldInfo> ToWorlds() const
    {
        auto gen = [&](const FaceInfo& f, bool flipped)
        {
            WorldInfo wi;
            wi.Origin      = Origin;
            wi.Orientation = flipped ?
                Orientation * glm::angleAxis(glm::pi<float>(), glm::vec3(1, 0, 0)) :
                Orientation;
            wi.Radius      = Radius;
            wi.Edge        = Edge;
            wi.Thickness   = Thickness;
            wi.Type        = f.Type;
            wi.TilePath    = f.TilePath;
            wi.OriginTile  = f.OriginTile;

            return wi;
        };

        std::vector<WorldInfo> res;
        res.push_back(gen(FaceA, false));

        if (FaceB) res.push_back(gen(*FaceB, true));
        
        return res;
    }

    static struct DiskWorldInfo FromJson(const picojson::value& v)
    {
        DiskWorldInfo wi;

        if (!v.is<picojson::object>())
            throw std::runtime_error("World no valid json");

        const auto& obj = v.get<picojson::object>();

        auto it_origin = obj.find("origin");

        if (it_origin != obj.end() && it_origin->second.is<picojson::array>())
        {
            const auto& arr = it_origin->second.get<picojson::array>();
            
            if (arr.size() != 3)
                throw std::runtime_error("vec3 must have exactly 3 elements");
            
            if (!arr[0].is<double>() || !arr[1].is<double>() || !arr[2].is<double>())
                throw std::runtime_error("vec3 elements must be floats");

            wi.Origin = glm::vec3(
                static_cast<float>(arr[0].get<double>()),
                static_cast<float>(arr[1].get<double>()),
                static_cast<float>(arr[2].get<double>())
            );
        }

        auto it_orientation = obj.find("orientation");

        if (it_orientation != obj.end() && it_orientation->second.is<picojson::array>())
        {
            const auto& arr = it_orientation->second.get<picojson::array>();
            
            if (arr.size() != 4)
                throw std::runtime_error("quat must have exactly 4 elements");
            
            if (!arr[0].is<double>() || !arr[1].is<double>() || !arr[2].is<double>() || !arr[3].is<double>())
                throw std::runtime_error("quat elements must be floats");

            wi.Orientation = glm::quat(
                static_cast<float>(arr[0].get<double>()),
                static_cast<float>(arr[1].get<double>()),
                static_cast<float>(arr[2].get<double>()),
                static_cast<float>(arr[3].get<double>())
            );
        }

        auto it_radius = obj.find("radius");

        if (it_radius != obj.end() && it_radius->second.is<double>())
            wi.Radius = static_cast<float>(it_radius->second.get<double>());

        auto it_edge = obj.find("edge");

        if (it_edge != obj.end() && it_edge->second.is<double>())
            wi.Edge = static_cast<float>(it_edge->second.get<double>());

        auto it_thickness = obj.find("thickness");

        if (it_thickness != obj.end() && it_thickness->second.is<double>())
            wi.Thickness = static_cast<float>(it_thickness->second.get<double>());


        auto it_faceA = obj.find("faceA");

        if (it_faceA != obj.end() && it_faceA->second.is<picojson::object>())
        {
            auto pfaceA = it_faceA->second.get<picojson::object>();

            FaceInfo face;

            auto it_type = pfaceA.find("type");

            if (it_type != pfaceA.end() && it_type->second.is<std::string>())
            {
                std::string str = it_type->second.get<std::string>();

                if (str == "BAKED")          face.Type = WorldType::BAKED;
                else if (str == "GENERATED") face.Type = WorldType::GENERATED;
                else throw std::runtime_error("Unknown WorldType: " + str);
            }

            auto it_tilePath = pfaceA.find("tilePath");

            if (it_tilePath != pfaceA.end() && it_tilePath->second.is<std::string>())
                face.TilePath = it_tilePath->second.get<std::string>();

            auto it_originTile = pfaceA.find("originTile");
            
            if (it_originTile != pfaceA.end() && it_originTile->second.is<std::string>())
                face.OriginTile = it_originTile->second.get<std::string>();

            wi.FaceA = face;
        }

        auto it_faceB = obj.find("faceB");

        if (it_faceB != obj.end() && it_faceB->second.is<picojson::object>())
        {
            auto pfaceB = it_faceB->second.get<picojson::object>();

            FaceInfo face;

            auto it_type = pfaceB.find("type");

            if (it_type != pfaceB.end() && it_type->second.is<std::string>())
            {
                std::string str = it_type->second.get<std::string>();

                if (str == "BAKED")          face.Type = WorldType::BAKED;
                else if (str == "GENERATED") face.Type = WorldType::GENERATED;
                else throw std::runtime_error("Unknown WorldType: " + str);
            }

            auto it_tilePath = pfaceB.find("tilePath");

            if (it_tilePath != pfaceB.end() && it_tilePath->second.is<std::string>())
                face.TilePath = it_tilePath->second.get<std::string>();

            auto it_originTile = pfaceB.find("originTile");
            
            if (it_originTile != pfaceB.end() && it_originTile->second.is<std::string>())
                face.OriginTile = it_originTile->second.get<std::string>();

            wi.FaceB = face;
        }

        return wi;
    }
};

struct World
{
    WorldInfo                                           info;
    std::unique_ptr<Material>                           material;
    std::unique_ptr<IChunkGenerator>                    generator;
    std::unordered_map<glm::ivec2, Chunk, IVec2Hash>    chunks;
    std::shared_ptr<Mesh>                               proxy;

    glm::ivec2 lastArea { 0,0 };
    bool       lastCheckInRange = false;
};

inline std::vector<DiskWorldInfo> LoadDiskWorldInfos(const std::filesystem::path& path)
{
    std::string json;
    std::ifstream worldsJson(path);

    if (!worldsJson)
    {
        std::cerr << "Loading worlds info: " << path << std::endl;
        throw std::runtime_error("worlds info file read failed");
    }

    std::stringstream ss;

    ss << worldsJson.rdbuf();
    worldsJson.close();
    json = ss.str();

    picojson::value v;
    std::string err = picojson::parse(v, json);

    if (!err.empty())
    {
        std::cerr << "Loading worlds info: " << path << std::endl;
        std::cerr << "ERROR::TERRAIN::FILE_NOT_SUCCESULLY_PARSED: " << err << std::endl;
        throw std::runtime_error("worlds info file read parsed");
    }

    std::vector<DiskWorldInfo> worlds;

    if (v.is<picojson::array>())
    {   
        const auto& arr = v.get<picojson::array>();

        for (const auto& vw : arr)
        {
            worlds.push_back(DiskWorldInfo::FromJson(vw));
        }
    }
    else
        throw std::runtime_error("worlds json not expected format");

    return worlds;
}
