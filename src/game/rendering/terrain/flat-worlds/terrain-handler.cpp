#include "terrain-handler.h"

#include <algorithm>

TerrainHandler::TerrainHandler(std::vector<World> worlds) : worlds_(std::move(worlds)) 
{
    for (size_t i = 0; i < worlds_.size(); ++i)
    {
        auto& w = worlds_[i];
        w.proxy = chunkHandler_.UploadChunk(w.generator->GenerateProxy(64));

        if (!w.proxy)
            throw std::runtime_error("World " + w.info.TilePath + " produces empty mesh with radius: " + std::to_string(w.info.Radius) + ". Maybe increase resolution.");
    }
}

TerrainHandler::~TerrainHandler() = default;

void TerrainHandler::UpdateStreaming(const glm::vec3& observerPos)
{
    for (auto& world : worlds_)
    {
        const glm::vec3 local = world.info.ToLocalCoords(observerPos);

        const glm::ivec2 area {
            (int)std::floor(local.x / TerrainConfig::RegionSize),
            (int)std::floor(local.z / TerrainConfig::RegionSize)
        };

        constexpr float renderRange = TerrainConfig::RenderArea * TerrainConfig::RegionSize;
        constexpr float exitMargin  = TerrainConfig::Hysteresis * TerrainConfig::RegionSize;

        const float margin = world.lastCheckInRange ? renderRange + exitMargin : renderRange;

        const float d2 = local.x * local.x + local.z * local.z;
        const float radial = world.info.Radius + margin;

        const float top     = world.generator->MaxHeight();
        const float bottom  = world.generator->MinHeight() - world.info.Thickness;
        
        const float normalDist = local.y > top    ? local.y - top
                               : local.y < bottom ? bottom - local.y
                               : 0.0f;

        const bool inRange = d2 <= radial * radial
                          && normalDist <= margin;
                    
        // 0.85f as a margin to allow generation on other side.
        // ToDo: Re-think whether to make this configurable.
        const bool correctSide = local.y >= 0.0f || d2 > (world.info.Radius * 0.85f * world.info.Radius);

        const bool changed = (area != world.lastArea) || (inRange != world.lastCheckInRange);
        world.lastArea = area;
        world.lastCheckInRange = inRange;

        if (!changed) continue;
        
        const World* beginPtr = &worlds_[0];
        const World* currentPtr = &world; 

        size_t worldIdx = currentPtr - beginPtr;

        if (worldIdx >= worlds_.size())
            throw std::logic_error("Invalid world index in TerrainHandler::EnqueueChunks");

        std::erase_if(pendingNew_, [worldIdx](const PendingChunk& p) { return p.worldIndex == worldIdx; });
        std::erase_if(pendingUpgrade_, [worldIdx](const PendingChunk& p) { return p.worldIndex == worldIdx; });
        
        if (inRange && correctSide) EnqueueChunks(world, area, worldIdx);

        CullChunks(world, area);
    }

    std::sort(pendingNew_.begin(), pendingNew_.end(), [](const PendingChunk& a, const PendingChunk& b) { return a.dist > b.dist; });
    std::sort(pendingUpgrade_.begin(), pendingUpgrade_.end(), [](const PendingChunk& a, const PendingChunk& b) { return a.dist > b.dist; });
    
    DrainQueueAsync(std::chrono::milliseconds(2));
}

void TerrainHandler::EnqueueChunks(World& world, const glm::ivec2 area, int worldIdx)
{
    for (int i = area.x - TerrainConfig::RenderArea; i <= area.x + TerrainConfig::RenderArea; i++)
    {
        for (int j = area.y - TerrainConfig::RenderArea; j <= area.y + TerrainConfig::RenderArea; j++)
        {
            if (!world.info.ChunkOnDisk({ i, j }, TerrainConfig::RegionSize)) continue;

            const int dist    = std::max(std::abs(i - area.x), std::abs(j - area.y));
            const bool lowLod = dist > TerrainConfig::LowLoDArea;
            const int lod     = lowLod ? TerrainConfig::LowLodRegionResolution : TerrainConfig::RegionResolution;

            auto it = world.chunks.find({ i, j});

            PendingChunk chunk;
            chunk.worldIndex = worldIdx;
            chunk.coord = { i, j };
            chunk.lod = lod;
            chunk.dist = dist;

            if (it != world.chunks.end())
            {
                // Only re-generate higher quality. Don't regenerate lower quality - causes stuttering in frametimws 
                // rather than just a little higher draw count
                if (lowLod || it->second.lod == TerrainConfig::RegionResolution) continue;

                pendingUpgrade_.push_back(chunk);

                continue;
            }

            pendingNew_.push_back(chunk);
        }
    }
}

void TerrainHandler::DrainQueueAsync(std::chrono::microseconds budget)
{
    const auto deadline = std::chrono::steady_clock::now() + budget;

    constexpr unsigned int maxUploads = 4;
    unsigned int uploads = 0;

    for (auto it = asyncChunks_.begin(); it != asyncChunks_.end(); )
    {
        if (uploads >= maxUploads || std::chrono::steady_clock::now() >= deadline)
            break;

        if (!it->valid() || it->wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        {
            ++it;
            continue;
        }
        
        try 
        {
            auto res = it->get();
            World& world = worlds_.at(res.worldIndex);

            if (ChunkWanted(world, res.coord, world.lastArea))
            {
                res.chunk.mesh = chunkHandler_.UploadChunk(std::move(res.data));
                worlds_.at(res.worldIndex).chunks.insert_or_assign(res.coord, res.chunk);
                ++uploads;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Terrain chunk generation failed: " << e.what() << std::endl;
        }

        it = asyncChunks_.erase(it);
    }

    unsigned int drainSize = 12;
    const unsigned int maxSize = 20;

    while (!pendingNew_.empty() && drainSize > 0 && asyncChunks_.size() < maxSize)
    {
        --drainSize;
        const PendingChunk p = pendingNew_.back();
        pendingNew_.pop_back();


        asyncChunks_.push_back(
            std::async(std::launch::async,[this, p]() {
                ChunkRegion region {
                        p.coord,
                        TerrainConfig::RegionSize,
                        p.lod
                    };
                
                Chunk chunk;
                //chunk.mesh = chunkHandler_.UploadChunk(worlds_.at(p.worldIndex).generator->Generate(region));
                chunk.lod = p.lod;

                PendingAsyncChunk pac;
                pac.chunk = chunk;
                pac.data = worlds_.at(p.worldIndex).generator->Generate(region);

                pac.coord = p.coord;
                pac.worldIndex = p.worldIndex;

                return pac;
            })
        );
    } 

    while (!pendingUpgrade_.empty()  && drainSize > 0 && asyncChunks_.size() < maxSize)
    {
        --drainSize;

        const PendingChunk p = pendingUpgrade_.back();
        pendingUpgrade_.pop_back();

        asyncChunks_.push_back(
            std::async(std::launch::async,[this, p]() {
                ChunkRegion region {
                    p.coord,
                    TerrainConfig::RegionSize,
                    p.lod
                };

                Chunk chunk;
                //chunk.mesh = chunkHandler_.UploadChunk(worlds_.at(p.worldIndex).generator->Generate(region));
                chunk.lod = p.lod;

                PendingAsyncChunk pac;
                pac.chunk = chunk;
                pac.data = worlds_.at(p.worldIndex).generator->Generate(region);

                pac.coord = p.coord;
                pac.worldIndex = p.worldIndex;

                return pac;
            })
        );
    }
}

bool TerrainHandler::ChunkWanted(const World& world, const glm::ivec2& chunk, const glm::ivec2& area) const
{
    constexpr int limit = TerrainConfig::RenderArea + TerrainConfig::Hysteresis;

    return world.info.ChunkOnDisk(chunk, TerrainConfig::RegionSize) &&
           std::abs(chunk.x - area.x) <= limit                      &&
           std::abs(chunk.y - area.y) <= limit;
}

void TerrainHandler::CullChunks(World& world, const glm::ivec2& area)
{
    constexpr int limit = TerrainConfig::RenderArea + TerrainConfig::Hysteresis;

    for (auto it = world.chunks.begin(); it != world.chunks.end();)
    {
        const glm::ivec2 chunk = it->first;

        const bool chunkOutsideDisk = !world.info.ChunkOnDisk(chunk, TerrainConfig::RegionSize);
        const bool outOfRange       = std::abs(chunk.x - area.x) > limit || std::abs(chunk.y - area.y) > limit;

        if (chunkOutsideDisk || outOfRange)
            it = world.chunks.erase(it);
        else
            ++it;
    }
}

TerrainHandler::TerrainCollision TerrainHandler::CheckCollision(glm::vec3 pos, float radius)
{
    TerrainCollision col
    {
        false,
        0.0f,
        { 0.0f, 0.0f, 0.0f}
    };

    for (auto& world : worlds_)
    {
        const glm::vec3 local = world.info.ToLocalCoords(pos);

        if (local.y < 0.0f)
            continue;

        if (local.x * local.x + local.z * local.z >  world.info.Radius * world.info.Radius)
            continue;

        const float bottom = local.y - radius;
        if (bottom > world.generator->MaxHeight())
            continue;

        const float height = world.generator->HeightAt(local.x, local.z);
        if (bottom >= height)
            continue;

        col.collided = true;
        col.penetration = height - bottom;
        col.normal = world.info.Orientation * world.generator->NormalAt(local);
        break;
    }

    return col;
}

std::vector<DrawCommand> TerrainHandler::BuildDrawCommands(RenderPass rp)
{
    std::vector<DrawCommand> dcs;

    size_t reserveSpace = 0;
    for (auto& world : worlds_) reserveSpace += world.chunks.size();

    dcs.reserve(reserveSpace);

    for (auto& world : worlds_)
    {

        const glm::mat4 transform = world.info.Transform();
    
        for (auto& [iv, mp] : world.chunks)
        {
            if (!mp.mesh) continue;

            DrawCommand dc;
            dc.mesh = mp.mesh.get();
            dc.material = world.material.get();
            dc.transform = transform;
            dc.renderPass = rp;
            dcs.push_back(dc);
        }

        {
            // So they don't bite: just move the proxy-"very low lod" world down a bit
            constexpr float sink = 20.0f;
            DrawCommand dc;
            dc.mesh = world.proxy.get();
            dc.material = world.material.get();
            dc.transform = world.info.Transform() * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -sink, 0.0f));
            dc.renderPass = rp;
            dcs.push_back(dc);
        }
    }

    return dcs;
}
