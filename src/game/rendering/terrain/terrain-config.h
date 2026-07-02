class TerrainConfig
{
public:
    static constexpr float HeightScale = 18.0f;
    static constexpr float HeightOffset = -19.0f;

    static constexpr float MinHeight = HeightOffset;
    static constexpr float MaxHeight = HeightOffset + HeightScale;

    static constexpr int RenderArea = 10;
    static constexpr int LowLoDArea = 7;
    static constexpr float RegionSize = 30.0f;
    static constexpr int RegionResolution = 64;
    static constexpr int LowLodRegionResolution = 16;
    static constexpr int Hysteresis = 5;
    static constexpr int Octaves = 5;
    static constexpr float Lacunarity = 2.0f;
    static constexpr float Gain = 0.5f;
    static constexpr float BaseFreq = 0.01f;
    
    static constexpr float SnowStart = MinHeight + 0.6 * HeightScale;
    static constexpr float SnowEnd = MinHeight + 0.8 * HeightScale;
    static constexpr float RockStart = 0.2f;
    static constexpr float RockEnd = 0.4f;
    static constexpr float FogStart = 100.0f;
    static constexpr float FogEnd = 400.0f;
    static constexpr float FogColor = 0.01f;
    static constexpr float TexScale = 0.2f;
};
