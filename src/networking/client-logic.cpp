#include "client-logic.h"

ClientLogic *ClientLogic::s_pCallbackInstance = nullptr;
ISteamNetworkingSockets* ClientLogic::m_pInterface = nullptr;
HSteamNetConnection ClientLogic::m_hConnection = k_HSteamNetConnection_Invalid;

ClientLogic::ClientLogic()
{
}

ClientLogic::~ClientLogic()
{
};

void ClientLogic::ClientLoop(const SteamNetworkingIPAddr &serverAddr, std::atomic<bool>& running)
{
    // Select instance to use.  For now we'll always use the default.
	m_pInterface = SteamNetworkingSockets();

	// Start connecting
	char szAddr[ SteamNetworkingIPAddr::k_cchMaxString ];
	serverAddr.ToString( szAddr, sizeof(szAddr), true );
	std::printf( "Connecting to chat server at %s", szAddr );
	SteamNetworkingConfigValue_t opt;
	opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallbackClient );
	m_hConnection = m_pInterface->ConnectByIPAddress( serverAddr, 1, &opt );
	if ( m_hConnection == k_HSteamNetConnection_Invalid )
        std::cerr << "Failed to create connection" << std::endl;

	const int tickRate = 30;
    const std::chrono::milliseconds tickDuration(1000 / tickRate);
    auto previousTickTime = std::chrono::steady_clock::now();

	while ( running )
	{
		auto now = std::chrono::steady_clock::now();
        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTickTime);

		// Ensure the server loop runs at the desired tick rate
        if (deltaTime >= tickDuration) {
            previousTickTime = now;  // Update the previous tick time
		
			PollIncomingMessagesClient(running);
			PollConnectionStateChangesClient();
        	//std::string msg = "I am the client";
        	//m_pInterface->SendMessageToConnection(m_hConnection, msg.c_str(), (uint32)msg.length(), k_nSteamNetworkingSend_Reliable, nullptr);
			//PollLocalUserInput();
        }
        else {
            // Sleep for the remaining time to maintain the fixed tick rate
            std::this_thread::sleep_for(tickDuration - deltaTime);
        }
	}

    m_pInterface->CloseConnection( m_hConnection, 0, "Goodbye", true );
};

void ClientLogic::PollIncomingMessagesClient(std::atomic<bool>& running)
{
	while ( running )
	{
		ISteamNetworkingMessage *pIncomingMsg = nullptr;
		int numMsgs = m_pInterface->ReceiveMessagesOnConnection( m_hConnection, &pIncomingMsg, 1 );
		if ( numMsgs == 0 )
			break;
		if ( numMsgs < 0 )
            std::cerr << "Error checking for messages" << std::endl;

		msgpack::object_handle oh = msgpack::unpack(
										static_cast<const char*>(pIncomingMsg->m_pData), pIncomingMsg->m_cbSize);
		
		msgpack::object obj = oh.get();
		obj.convert(gameState_);

		// Just echo anything we get from the server
		//fwrite( pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout );
		//fputc( '\n', stdout );

		// We don't need this anymore.
		pIncomingMsg->Release();
	}
}

const GameState& ClientLogic::GetLatestGameState() const
{
	return gameState_;
}

void ClientLogic::PollConnectionStateChangesClient()
{
	s_pCallbackInstance = this;
	m_pInterface->RunCallbacks();
}

void ClientLogic::OnSteamNetConnectionStatusChangedClient( SteamNetConnectionStatusChangedCallback_t *pInfo, std::atomic<bool>& running )
{
	assert( pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid );

	// What's the state of the connection?
	switch ( pInfo->m_info.m_eState )
	{
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			running = false;

			// Print an appropriate message
			if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting )
			{
				// Note: we could distinguish between a timeout, a rejected connection,
				// or some other transport problem.
                std::printf( "We sought the remote host, yet our efforts were met with defeat.  (%s)", pInfo->m_info.m_szEndDebug );
			}
			else if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
			{

				std::printf( "Alas, troubles beset us; we have lost contact with the host.  (%s)", pInfo->m_info.m_szEndDebug );
			}
			else
			{
				// NOTE: We could check the reason code for a normal disconnection
				std::printf( "The host hath bidden us farewell.  (%s)", pInfo->m_info.m_szEndDebug );
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
			m_hConnection = k_HSteamNetConnection_Invalid;
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
			// We will get this callback when we start connecting.
			// We can ignore this.
			break;

		case k_ESteamNetworkingConnectionState_Connected:
			std::printf( "Connected to server OK" );
			break;

		default:
			// Silences -Wswitch
			break;
	}
}
