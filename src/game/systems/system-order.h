#pragma once

enum class SystemOrder : int {
// ----- region FrameStart -----
    NetworkPollSystem = 10,

// ----- region Input -----
    NetworkInputConsumeSystem = 20,
    
// ----- region PreSimulation -----
    PiHistorySystem = 30,

// ----- region Simulation -----
    PhysicsSystem = 40,
    PlayerSystem = 50,
    NpcSystem = 60,
    ShotSystem = 70,
    TerrainSystem = 80,

// ----- region PostSimulation -----
    NetworkStateDistributionSystem = 90,
    NetworkInputDistributionSystem = 100,

// ----- region PostTick -----
    NetworkMergeSystem = 110,

// ----- region PreRender -----
    BlendingSystem = 120,
    CameraSystem = 130,

// ----- region PostRender -----
};
