#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include "picojson/picojson.h"
#include <glm/gtc/quaternion.hpp>

#include "../../../engine/rendering/materials/material.h"
#include "../../../engine/rendering/terrain/chunk-structs.h"
#include "../../../engine/rendering/terrain/chunk-generator.h"

enum class WorldType
{
    BAKED,
    GENERATED
};

struct WorldInfo
{
    glm::vec3   Origin      { 0.0f };
    glm::quat   Orientation { 1.0f, 0.0f, 0.0f, 0.0f };
    float       Radius      { 0.0f };
    WorldType   Type;
    std::string TilePath;
    std::string OriginTile;

    static struct WorldInfo FromJson(const picojson::value& v)
    {
        WorldInfo wi;

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

        auto it_type = obj.find("type");

        if (it_type != obj.end() && it_type->second.is<std::string>())
        {
            std::string str = it_type->second.get<std::string>();

            if (str == "BAKED")          wi.Type = WorldType::BAKED;
            else if (str == "GENERATED") wi.Type = WorldType::GENERATED;
            else throw std::runtime_error("Unknown WorldType: " + str);
        }

        auto it_tilePath = obj.find("tilePath");

        if (it_tilePath != obj.end() && it_tilePath->second.is<std::string>())
            wi.TilePath = it_tilePath->second.get<std::string>();

        auto it_originTile = obj.find("originTile");
 
        if (it_originTile != obj.end() && it_originTile->second.is<std::string>())
            wi.OriginTile = it_originTile->second.get<std::string>();

        return wi;
    }
};

struct World
{
    WorldInfo                                           info;
    std::unique_ptr<Material>                           material;
    std::unique_ptr<IChunkGenerator>                    generator;
    std::unordered_map<glm::ivec2, Chunk, IVec2Hash>    chunks;

    glm::ivec2 lastArea { 0,0 };
    bool       lastCheckInRange = false;
};

inline std::vector<WorldInfo> LoadWorldInfos(const std::filesystem::path& path)
{
    std::string json;
    std::ifstream worldsJson;

    try 
    {
        worldsJson.open(path.string());

        std::stringstream ss;

        ss << worldsJson.rdbuf();
        worldsJson.close();
        json = ss.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cerr << "Loading worlds info: " << path << std::endl;
        std::cerr << "ERROR::TERRAIN::FILE_NOT_SUCCESULLY_READ: " << e.what() << std::endl;
        throw std::runtime_error("worlds info file read failed");
    }

    picojson::value v;
    std::string err = picojson::parse(v, json);

    if (!err.empty())
    {
        std::cerr << "Loading worlds info: " << path << std::endl;
        std::cerr << "ERROR::TERRAIN::FILE_NOT_SUCCESULLY_PARSED: " << err << std::endl;
        throw std::runtime_error("worlds info file read parsed");
    }

    std::vector<WorldInfo> worlds;

    if (v.is<picojson::array>())
    {   
        const auto& arr = v.get<picojson::array>();

        for (const auto& vw : arr)
        {
            worlds.push_back(WorldInfo::FromJson(vw));
        }
    }
    else
        throw std::runtime_error("worlds json not expected format");
    return worlds;
}
