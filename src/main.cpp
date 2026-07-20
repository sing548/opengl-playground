#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <string>

#include "engine/engine.h"
#include "engine/models/scene.h"
#include "engine/models/model.h"
#include "engine/window/window.h"
#include "engine/shaders/shader.h"
#include "engine/rendering/renderer.h"
#include "engine/helpers/file-helper.h"

#include "game/systems/networking/network-input-consume-system.h"
#include "game/systems/networking/network-input-distribution-system.h"
#include "game/systems/networking/network-merge-system.h"
#include "game/systems/networking/network-poll-system.h"
#include "game/systems/networking/network-state-distribution-system.h"
#include "game/systems/blending-system.h"
#include "game/systems/camera-system.h"
#include "game/systems/npc-system.h"
#include "game/systems/physics-system.h"
#include "game/systems/pi-history-system.h"
#include "game/systems/player-system.h"
#include "game/systems/shot-system.h"
#include "game/systems/homing-system.h"
#include "game/systems/terrain-system.h"

#include "engine/rendering/sky/sky.h"
#include "game/rendering/grass/grass.h"
#include "game/rendering/flight/con-trail.h"
#include "game/rendering/flight/drive-plume.h"
#include "game/rendering/vfx/death-explosion.h"
#include "game/rendering/materials/game-material.h"

#include "game/rendering/materials/model-material.h"
#include "game/rendering/materials/hitbox-material.h"
#include "game/rendering/materials/terrain-material.h"


int main(int argc, const char *argv[]) {

    bool bServer = false;
	bool bClient = false;
	int nPort = 5001;
    std::string serverUrl;

    FileHelper::SetBaseDir(argv[0]);

	for ( int i = 1 ; i < argc ; ++i )
	{
		if ( !bClient && !bServer )
		{
			if ( !strcmp( argv[i], "client" ) )
			{
				bClient = true;
                bServer = false;
				continue;
			}
			if ( !strcmp( argv[i], "server" ) )
			{
				bServer = true;
				continue;
			}
		}
		if ( !strcmp( argv[i], "--port" ) )
		{
			++i;
			if ( i >= argc )
            {
                std::cout << std::printf("i >= argc") <<std::endl;
                return 0;
            }
			nPort = atoi( argv[i] );
			if ( nPort <= 0 || nPort > 65535 )
            {
                std::cout << std::printf("Invalid port %d", nPort) <<std::endl;
                return 0;
            }
			continue;
		}

        if ( !strcmp( argv[i], "--url" ))
        {
            ++i;
            if ( i >= argc )
            {
                std::cout << std::printf("i => argc") << std::endl;
                return 0;
            }
            serverUrl =  std::string(( argv[i] ));
            continue;
        }
	}

    std::unique_ptr<Engine> engine = nullptr;

    try {
        if (bServer)
            engine = std::make_unique<Engine>(EngineMode::Server, "", nPort);
        else if (bClient)
            engine = std::make_unique<Engine>(EngineMode::Client, serverUrl);
        else 
        {
            engine = std::make_unique<Engine>(EngineMode::Standalone);
        }
    }
    catch (const std::runtime_error e)
    {
        std::cerr << "Failed to initialize GLFW" << e.what() << std::endl;
        return -1;
    }

    std::string base = (std::filesystem::path(FileHelper::GetAssetsDir()) / "skybox" / "NASA2").string();
	std::vector<std::string> faces
	{
	    (std::filesystem::path(base) / "posx.png").string(),
	    (std::filesystem::path(base) / "negx.png").string(),
	    (std::filesystem::path(base) / "posy.png").string(),
	    (std::filesystem::path(base) / "negy.png").string(),
	    (std::filesystem::path(base) / "posz.png").string(),
	    (std::filesystem::path(base) / "negz.png").string()
	};

    std::unique_ptr<Sky> sky = std::make_unique<Sky>(faces);
    engine->AddSceneRenderable(std::move(sky));
	std::unique_ptr<Grass> grass = std::make_unique<Grass>();
    engine->AddSceneRenderable(std::move(grass));
    std::unique_ptr<DrivePlume> dp = std::make_unique<DrivePlume>(engine->GetGameWorld());
    //engine->AddSceneRenderable(std::move(dp));
    std::unique_ptr<DeathExplosion> deathExplosion = std::make_unique<DeathExplosion>(engine->GetGameWorld());
    engine->AddSceneRenderable(std::move(deathExplosion));
    std::unique_ptr<ConTrail> ct = std::make_unique<ConTrail>(engine->GetGameWorld());
    engine->AddSceneRenderable(std::move(ct));


    auto phys = std::make_unique<PhysicsSystem>();
    engine->AddGameplaySystem(std::move(phys));

    auto npc = std::make_unique<NpcSystem>();
    engine->AddGameplaySystem(std::move(npc));

    auto player = std::make_unique<PlayerSystem>();
    engine->AddGameplaySystem(std::move(player));

    auto shot = std::make_unique<ShotSystem>();
    engine->AddGameplaySystem(std::move(shot));

    auto homing = std::make_unique<HomingSystem>();
    engine->AddGameplaySystem(std::move(homing));

    auto terr = std::make_unique<TerrainSystem>();
    engine->AddGameplaySystem(std::move(terr));

    auto cam = std::make_unique<CameraSystem>();
    engine->AddGameplaySystem(std::move(cam));

    auto hist = std::make_unique<PiHistorySystem>();
    engine->AddGameplaySystem(std::move(hist));

    auto blend = std::make_unique<BlendingSystem>();
    engine->AddGameplaySystem(std::move(blend));

    auto nDistr = std::make_unique<NetworkStateDistributionSystem>();
    engine->AddGameplaySystem(std::move(nDistr));

    auto nInpDist = std::make_unique<NetworkInputDistributionSystem>();
    engine->AddGameplaySystem(std::move(nInpDist));

    auto nPoll = std::make_unique<NetworkPollSystem>();
    engine->AddGameplaySystem(std::move(nPoll));

    auto nCons = std::make_unique<NetworkInputConsumeSystem>();
    engine->AddGameplaySystem(std::move(nCons));

    auto nMerge = std::make_unique<NetworkMergeSystem>();
    engine->AddGameplaySystem(std::move(nMerge));

    std::string modelVert  = (std::filesystem::path(FileHelper::GetShaderDir()) / "model.vert").string();
    std::string modelFrag  = (std::filesystem::path(FileHelper::GetShaderDir()) / "model.frag").string();
	std::string hitboxVert = (std::filesystem::path(FileHelper::GetShaderDir()) / "hitbox.vert").string();
	std::string hitboxFrag = (std::filesystem::path(FileHelper::GetShaderDir()) / "hitbox.frag").string();
    std::string terrainVert = (std::filesystem::path(FileHelper::GetShaderDir()) / "terrain.vert").string();
	std::string terrainFrag = (std::filesystem::path(FileHelper::GetShaderDir()) / "terrain.frag").string();
    
    auto modelShader  = std::make_unique<Shader>(modelVert.c_str(), modelFrag.c_str());
	auto hitboxShader = std::make_unique<Shader>(hitboxVert.c_str(), hitboxFrag.c_str());
    auto terrainShader = std::make_unique<Shader>(terrainVert.c_str(), terrainFrag.c_str());

    auto modelMat = std::make_unique<ModelMaterial>(std::move(modelShader));
    engine->AddMaterial(static_cast<uint16_t>(GameMaterial::Model), std::move(modelMat));

    auto hitboxMat = std::make_unique<HitboxMaterial>(std::move(hitboxShader));
    engine->AddMaterial(static_cast<uint16_t>(GameMaterial::Hitbox), std::move(hitboxMat));

    auto terrainMat = std::make_unique<TerrainMaterial>(std::move(terrainShader), engine->GetAssMan());
    auto tH = std::make_unique<TerrainHandler>(std::move(terrainMat));
    engine->AddTerrainHandler(std::move(tH));

    engine->Run();

    return 0;
}
