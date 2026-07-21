#pragma once

enum class SystemOrder : int {
// ----- region FrameStart -----
    NetworkPollSystem = 10,

// ----- region Input -----
    InputSystem = 20,
    NetworkInputConsumeSystem = 30,
    
// ----- region PreSimulation -----
    PiHistorySystem = 40,

// ----- region Simulation -----
    PhysicsSystem = 50,
    PlayerSystem = 60,
    NpcSystem = 70,
    ShotSystem = 80,
    TerrainSystem = 90,

// ----- region PostSimulation -----
    NetworkStateDistributionSystem = 100,
    NetworkInputDistributionSystem = 110,

// ----- region PostTick -----
    NetworkMergeSystem = 120,
    NetworkReconcileSystem = 130,

// ----- region PreRender -----
    BlendingSystem = 140,
    CameraSystem = 150,

// ----- region PostRender -----
};
