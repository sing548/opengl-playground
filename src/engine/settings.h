#pragma once

struct Settings {
    bool adjustCamera = true;
    bool thirdPerson = false;
    bool skyBox = true;
    bool debugView = false;
    bool hitboxes = false;
    bool bloom = true;
    bool flightAssist = true;
    bool flight3d = true;
    bool predictiveClient = true;
    bool terrain = false;
    bool grass = true;
    bool imGui = false;
    int fakeLag = 0;
    float pkgLossPct = 0.0f;
    float pkgJitter = 0.0f;
};

