#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

#include "window/window.h"
#include "shaders/shader.h"
#include "models/model.h"
#include "models/scene.h"
#include "rendering/renderer.h"
#include "engine/engine.h"

int main(int argc, const char *argv[]) {

    bool bServer = false;
	bool bClient = false;
	int nPort = 5001;
	SteamNetworkingIPAddr addrServer; addrServer.Clear();

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

		if ( bClient && addrServer.IsIPv6AllZeros() )
		{
			if ( !addrServer.ParseString( argv[i] ) )
            {
                std::printf("Invalid server address '%s'", argv[i]);
                return 0;
            }
			if ( addrServer.m_port == 0 )
				addrServer.m_port = 5001;
			continue;
		}
	}

    Engine* engine = nullptr;

    try {
        if (bServer)
            engine = new Engine(1);
        else if (bClient)
            engine = new Engine(2);
        else 
        {
            engine = new Engine(0);
            auto models = engine->BasicLevel();
            engine->SetupScene(models); 
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
