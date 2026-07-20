#pragma once

struct Settings {
    bool adjustCamera = true;
    bool mouseLooking = false;
    bool thirdPerson = false;
    bool skyBox = true;
    bool debugView = false;
    bool hitboxes = false;
    bool bloom = true;
    bool simpleFlight = true;
    bool predictiveClient = true;
    bool terrain = false;
    bool grass = true;
    bool imGui = false;
    bool npcs = true;
    int fakeLag = 0;
    float pkgLossPct = 0.0f;
    float pkgJitter = 0.0f;
};

