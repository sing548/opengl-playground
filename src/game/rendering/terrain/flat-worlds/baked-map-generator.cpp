#include "baked-map-generator.h"

#include "stb/stb_image.h"
#include "picojson/picojson.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <charconv>
#include <iostream>

#include "../../../../engine/helpers/file-helper.h"

#include "terrain-config.h"

BakedMapGenerator::BakedMapGenerator(WorldInfo wi)
{
    auto path = std::filesystem::path(FileHelper::GetAssetsDir()) / "worlds" / wi.TilePath / wi.OriginTile;
    
    ReadMapConfig(path);

    unsigned short* data = stbi_load_16((path / "heightmap.png").string().c_str(), &width_, &height_, nullptr, 1);
    
    if (data == nullptr)
    {
        std::string s = stbi_failure_reason();
        throw std::runtime_error("heightmap.png file read failed" + s);
    }

    const float scale = (maxHeight_ - minHeight_) / 65535.0f;
    const size_t count = (size_t)width_ * height_;
    vData_.resize(count);
    std::transform(data, data + count, vData_.begin(), [&](uint16_t d) { return (float) d * scale + minHeight_; });

    stbi_image_free(data);
}

float BakedMapGenerator::HeightAt(float x, float z) const
{
    const float fu = (x / worldSize_ + 0.5f) * (width_ - 1);
    const float fv = (z / worldSize_ + 0.5f) * (height_ - 1);

    const float cu = std::clamp(fu, 0.0f, (float)(width_ - 1));
    const float cv = std::clamp(fv, 0.0f, (float)(height_ - 1));

    const int x0 = (int)std::floor(cu); 
    const int z0 = (int)std::floor(cv);
    const int x1 = std::min(x0 + 1, width_ - 1);
    const int z1 = std::min(z0 + 1, height_ - 1);
    const float fx = cu - x0;
    const float fz = cv - z0;

    auto texel = [&](int px, int pz) {
        return vData_[pz * width_ + px];
    };

    const float h = std::lerp(std::lerp(texel(x0, z0), texel(x1, z0), fx),
                              std::lerp(texel(x0, z1), texel(x1, z1), fx),
                              fz);

    return h;
}

void BakedMapGenerator::ReadMapConfig(std::filesystem::path path)
{
    std::string heightmapCode;
    std::ifstream heightmapFile;
    path = path / "heightmap.json";

    try 
    {
        heightmapFile.open(path.string());

        std::stringstream ss;

        ss << heightmapFile.rdbuf();
        heightmapFile.close();

        heightmapCode = ss.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cerr << "Loading heightmap: " << path << std::endl;
        std::cerr << "ERROR::TERRAIN::FILE_NOT_SUCCESULLY_READ: " << e.what() << std::endl;
        throw std::runtime_error("Heightmap file read failed");
    }

    picojson::value v;
    std::string err = picojson::parse(v, heightmapCode);

    if (!err.empty())
    {
        std::cerr << "Loading heightmap: " << path << std::endl;
        std::cerr << "ERROR::TERRAIN::FILE_NOT_SUCCESULLY_PARSED: " << err << std::endl;
        throw std::runtime_error("Heightmap file parse failed");
    }

    if (v.contains("worldSize") && v.get("worldSize").is<double>())
        worldSize_ = static_cast<float>(v.get("worldSize").get<double>());
    else
    {
        std::cerr << "Loading heightmap: " << path << std::endl;
        std::cerr << "ERROR::TERRAIN::KEY_NOT_CONTAINED: worldSize" << std::endl;
        throw std::runtime_error("Heightmap file parse failed");
    }

    if (v.contains("minHeight") && v.get("minHeight").is<double>())
        minHeight_ = static_cast<float>(v.get("minHeight").get<double>());
    else
    {
        std::cerr << "Loading heightmap: " << path << std::endl;
        std::cerr << "ERROR::TERRAIN::KEY_NOT_CONTAINED: minHeight" << std::endl;
        throw std::runtime_error("Heightmap file parse failed");
    }

    if (v.contains("maxHeight") && v.get("maxHeight").is<double>())
        maxHeight_ = static_cast<float>(v.get("maxHeight").get<double>());
    else
    {
        std::cerr << "Loading heightmap: " << path << std::endl;
        std::cerr << "ERROR::TERRAIN::KEY_NOT_CONTAINED: maxHeight" << std::endl;
        throw std::runtime_error("Heightmap file parse failed");
    }
}
