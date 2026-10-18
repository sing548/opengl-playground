#pragma once

enum class SystemOrder : int {
// ----- region FrameStart -----
    NetworkPollSystem = 100,

// ----- region Input -----
    InputSystem = 200,
    NetworkInputConsumeSystem = 210,
    
// ----- region PreSimulation -----
    PiHistorySystem = 300,

// ----- region Simulation -----
    PhysicsSystem = 400,
    PlayerSystem = 410,
    NpcSystem = 420,
    ShotSystem = 430,
    TerrainSystem = 440,
    //ToDo: Correct game<->engine break with Fluid System
    FluidSystem = 450,

// ----- region PostSimulation -----
    NetworkStateDistributionSystem = 500,
    NetworkInputDistributionSystem = 510,

// ----- region PostTick -----
    NetworkMergeSystem = 600,
    NetworkReconcileSystem = 610,

// ----- region PreRender -----
    BlendingSystem = 700,
    //ToDo: Correct game<->engine break with Free Camera System
    FreeCameraSystem = 705,
    CameraSystem = 710,

// ----- region PostRender -----
};
