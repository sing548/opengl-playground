#include "server-logic.h"

HSteamNetPollGroup ServerLogic::m_hPollGroup = k_HSteamNetPollGroup_Invalid;  // Initialize poll group to invalid value
HSteamListenSocket ServerLogic::m_hListenSock = k_HSteamNetPollGroup_Invalid;  // Initialize listen socket to invalid valeu
ISteamNetworkingSockets* ServerLogic::m_pInterface = nullptr;  // Initialize pointer to null
std::map<HSteamNetConnection, ServerLogic::Client_t> ServerLogic::m_mapClients;  // Initialize map of clients
SteamNetworkingMicroseconds g_logTimeZero;
ServerLogic *ServerLogic::s_pCallbackInstance = nullptr;

void ServerLogic::ServerLoop(int port, std::atomic<bool>& running)
{
    std::printf("Opening listening on port %d", port);
    m_pInterface = SteamNetworkingSockets();
    SteamNetworkingIPAddr serverAddress;
    serverAddress.Clear();
    serverAddress.m_port = port;
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallbackServer );
    m_hListenSock = m_pInterface->CreateListenSocketIP( serverAddress, 1, &opt );

    if ( m_hListenSock == k_HSteamListenSocket_Invalid )
        std::cerr << "Failed to listen on port" << std::endl;

    m_hPollGroup = m_pInterface->CreatePollGroup();

    if ( m_hPollGroup == k_HSteamNetPollGroup_Invalid )
        std::cerr << "Failed to listen on port" << std::endl;

    std::printf( "Server listening on port %d\n", port );

    const int tickRate = 30;
    const std::chrono::milliseconds tickDuration(1000 / tickRate);
    auto previousTickTime = std::chrono::steady_clock::now();

    // Game server loop
    while (running) {
        auto now = std::chrono::steady_clock::now();
        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTickTime);

        // Ensure the server loop runs at the desired tick rate
        if (deltaTime >= tickDuration) {
            previousTickTime = now;  // Update the previous tick time

            PollIncomingMessagesServer(running);
            PollConnectionStateChangesServer();
            SendStringToAllClients("This will be the game state");
        }
        else {
            // Sleep for the remaining time to maintain the fixed tick rate
            std::this_thread::sleep_for(tickDuration - deltaTime);
        }
    }
};

void ServerLogic::PollIncomingMessagesServer(std::atomic<bool>& running)
{
    char temp[ 1024 ];

	while ( running )
	{
		ISteamNetworkingMessage *pIncomingMsg = nullptr;
		int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup( m_hPollGroup, &pIncomingMsg, 1 );
		if ( numMsgs == 0 )
			break;
		if ( numMsgs < 0 )
			std::cerr << "Error checking for messages" << std::endl;

		assert( numMsgs == 1 && pIncomingMsg );
		auto itClient = m_mapClients.find( pIncomingMsg->m_conn );
		assert( itClient != m_mapClients.end() );

		// '\0'-terminate it to make it easier to parse
		std::string sCmd;
		sCmd.assign( (const char *)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize );
		const char *cmd = sCmd.c_str();

		// We don't need this anymore.
		pIncomingMsg->Release();

		// Check for known commands.  None of this example code is secure or robust.
		// Don't write a real server like this, please.

		// Assume it's just a ordinary chat message, dispatch to everybody else
		sprintf( temp, "%s: %s", itClient->second.m_sNick.c_str(), cmd );
        std::cout << temp << std::endl;
		//SendStringToAllClients( temp, itClient->first );
	}
};

void ServerLogic::PollConnectionStateChangesServer()
{
    s_pCallbackInstance = this;
	m_pInterface->RunCallbacks();
};

void ServerLogic::OnSteamNetConnectionStatusChangedServer( SteamNetConnectionStatusChangedCallback_t *pInfo )
	{
		char temp[1024];

		// What's the state of the connection?
		switch ( pInfo->m_info.m_eState )
		{
			case k_ESteamNetworkingConnectionState_None:
				// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				// Ignore if they were not previously connected.  (If they disconnected
				// before we accepted the connection.)
				if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected )
				{
					// Locate the client.  Note that it should have been found, because this
					// is the only codepath where we remove clients (except on shutdown),
					// and connection change callbacks are dispatched in queue order.
					auto itClient = m_mapClients.find( pInfo->m_hConn );
					assert( itClient != m_mapClients.end() );

					// Select appropriate log messages
					const char *pszDebugLogAction;
					if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
					{
						pszDebugLogAction = "problem detected locally";
						sprintf( temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug );
					}
					else
					{
						// Note that here we could check the reason code to see if
						// it was a "usual" connection or an "unusual" one.
						pszDebugLogAction = "closed by peer";
						sprintf( temp, "%s hath departed", itClient->second.m_sNick.c_str() );
					}

					// Spew something to our own log.  Note that because we put their nick
					// as the connection description, it will show up, along with their
					// transport-specific data (e.g. their IP address)
					std::printf( "Connection %s %s, reason %d: %s\n",
						pInfo->m_info.m_szConnectionDescription,
						pszDebugLogAction,
						pInfo->m_info.m_eEndReason,
						pInfo->m_info.m_szEndDebug
					);

					m_mapClients.erase( itClient );

					// Send a message so everybody else knows what happened
					//SendStringToAllClients( temp );
				}
				else
				{
					assert( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting );
				}

				// Clean up the connection.  This is important!
				// The connection is "closed" in the network sense, but
				// it has not been destroyed.  We must close it on our end, too
				// to finish up.  The reason information do not matter in this case,
				// and we cannot linger because it's already closed on the other end,
				// so we just pass 0's.
				m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
				break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
			{
				// This must be a new connection
				assert( m_mapClients.find( pInfo->m_hConn ) == m_mapClients.end() );

				std::printf( "Connection request from %s", pInfo->m_info.m_szConnectionDescription );

				// A client is attempting to connect
				// Try to accept the connection.
				if ( m_pInterface->AcceptConnection( pInfo->m_hConn ) != k_EResultOK )
				{
					// This could fail.  If the remote host tried to connect, but then
					// disconnected, the connection may already be half closed.  Just
					// destroy whatever we have on our side.
					m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
					std::printf( "Can't accept connection.  (It was already closed?)" );
					break;
				}

				// Assign the poll group
				if ( !m_pInterface->SetConnectionPollGroup( pInfo->m_hConn, m_hPollGroup ) )
				{
					m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
					std::printf( "Failed to set poll group?" );
					break;
				}

				// Generate a random nick.  A random temporary nick
				// is really dumb and not how you would write a real chat server.
				// You would want them to have some sort of signon message,
				// and you would keep their client in a state of limbo (connected,
				// but not logged on) until them.  I'm trying to keep this example
				// code really simple.
				char nick[ 64 ];
				sprintf( nick, "BraveWarrior%d", 10000 + ( rand() % 100000 ) );

				// Send them a welcome message
				sprintf( temp, "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine command '/nick' we shall know thee otherwise.", nick ); 
				//SendStringToClient( pInfo->m_hConn, temp ); 

				// Also send them a list of everybody who is already connected
				if ( m_mapClients.empty() )
				{
					//SendStringToClient( pInfo->m_hConn, "Thou art utterly alone." ); 
				}
				else
				{
					sprintf( temp, "%d companions greet you:", (int)m_mapClients.size() ); 
					for ( auto &c: m_mapClients )
                    {
						//SendStringToClient( pInfo->m_hConn, c.second.m_sNick.c_str() ); 
                    }
				}

				// Let everybody else know who they are for now
				sprintf( temp, "Hark!  A stranger hath joined this merry host.  For now we shall call them '%s'", nick ); 
				//SendStringToAllClients( temp, pInfo->m_hConn ); 

				// Add them to the client list, using std::map wacky syntax
				m_mapClients[ pInfo->m_hConn ];
				//SetClientNick( pInfo->m_hConn, nick );
				break;
			}

			case k_ESteamNetworkingConnectionState_Connected:
				// We will get a callback immediately after accepting the connection.
				// Since we are the server, we can ignore this, it's not news to us.
				break;

			default:
				// Silences -Wswitch
				break;
		}
	};

void ServerLogic::SendStringToClient( HSteamNetConnection conn, const char *str )
{
	m_pInterface->SendMessageToConnection( conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr );
};

void ServerLogic::SendStringToAllClients(const char* str, HSteamNetConnection except) { 
    for (auto& c : m_mapClients) {
        if (c.first != except)
            SendStringToClient(c.first, str);
    }
};
