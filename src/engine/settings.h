#pragma once

struct Sandbox {
    bool runSimulation = false;
    bool gpuSimulation = false;
    bool singleStep = false;
    bool flatSimulation = true;
    int stepsPerTick = 3;
    int particleCount = 3000;
    float boxWidth = 20.0f;
    float boxHeight = 15.0f;
    float boxDepth = 10.0f;
    float particleSizeee = 0.4f;
    float particleSpacing = 1.0f;
    float gravity = 0.0f;
    float restitution = 0.2f;
    float smoothingRadius = 1.5f;
    float targetDensity = 6.5f;
    float viscosityStrength = 0.1f;
    float pressureMultiplier = 6000.0f;
    float nearPressureMultiplier = 3.0f;
};

struct Settings {
    bool adjustCamera = true;
    bool thirdPerson = true;
    bool skyBox = true;
    bool debugView = false;
    bool hitboxes = false;
    bool bloom = true;
    bool flightAssist = true;
    bool flight3d = true;
    bool predictiveClient = true;
    bool terrain = true;
    bool grass = false;
    bool imGui = false;
    int fakeLag = 0;
    float pkgLossPct = 0.0f;
    float pkgJitter = 0.0f;

    //---------- Sandbox ----------
    Sandbox sandbox;
};
