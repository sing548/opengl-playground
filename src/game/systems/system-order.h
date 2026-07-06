#pragma once

enum class SystemOrder : int {
// ----- region Input -----
    
// ----- region PreSimulation -----

    PiHistorySystem = 10,

// ----- region Simulation -----
    PhysicsSystem = 20,
    PlayerSystem = 30,
    NpcSystem = 40,
    ShotSystem = 50,
    TerrainSystem = 60,

// ----- region PostSimulation -----

// ----- region PreRender -----
    BlendingSystem = 70,
    CameraSystem = 80,

// ----- region PostRender -----
};
