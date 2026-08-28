class TerrainConfig
{
public:
    static constexpr int RenderArea = 20;
    static constexpr int LowLoDArea = 8;
    static constexpr float RegionSize = 30.0f;
    static constexpr int RegionResolution = 64;
    static constexpr int LowLodRegionResolution = 16;
    static constexpr int Hysteresis = 7;
    
    static constexpr float SnowStartFrac = 0.5f;
    static constexpr float SnowEndFrac = 0.7f;
    static constexpr float RockStart = 0.2f;
    static constexpr float RockEnd = 0.4f;
    static constexpr float FogStart = 300.0f;
    static constexpr float FogEnd = 600.0f;
    static constexpr float FogMax = 0.8f;
    static constexpr float FogColor = 0.01f;
    static constexpr float TexScale = 0.2f;
};
