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

    engine->Run();

    return 0;
}
