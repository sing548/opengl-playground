#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <string>

#include "engine/window/window.h"
#include "engine/shaders/shader.h"
#include "engine/models/model.h"
#include "engine/models/scene.h"
#include "engine/rendering/renderer.h"
#include "engine/engine.h"

#include "engine/rendering/sky/sky.h"
#include "game/rendering/grass/grass.h"

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
	std::unique_ptr<Grass> grass = std::make_unique<Grass>();

    // Currently needs to be ordered at creation - Skybox last
    engine->AddSceneRenderable(std::move(grass));
    engine->AddSceneRenderable(std::move(sky));

    engine->Run();

    return 0;
}
