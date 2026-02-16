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
	std::printf( "Connecting as client to server at %s\n", szAddr );
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

        previousTickTime = now;  // Update the previous tick time
		
		PollIncomingMessagesClient(running);
		PollConnectionStateChangesClient();

		if (messageReceived_)
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

    m_pInterface->CloseConnection( m_hConnection, 0, "Goodbye", true );
};

void ClientLogic::PollIncomingMessagesClient(std::atomic<bool>& running)
{
    ISteamNetworkingMessage* pIncomingMsg = nullptr;

    while (running)
    {
        int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
        if (numMsgs == 0)
            break; // no more messages
        if (numMsgs < 0)
        {
            std::cerr << "Error receiving messages" << std::endl;
            break;
        }

        // Unpack message
        msgpack::object_handle oh = msgpack::unpack(
            static_cast<const char*>(pIncomingMsg->m_pData), pIncomingMsg->m_cbSize);
        msgpack::object obj = oh.get();

		if (obj.type != msgpack::type::ARRAY || obj.via.array.size != 2)
		{
		    std::cerr << "Invalid message format" << std::endl;
		    return;
		}


		uint8_t type = obj.via.array.ptr[0].as<uint8_t>();
		msgpack::object payload = obj.via.array.ptr[1];

		switch (type)
		{
			case 0:
			{
				GameState gs;
				payload.convert(gs);
				int newTick = gs.tick;
				if (newTick - previoustick != 1)
        			std::cout <<   "Skipped tick(1) between: " << previoustick << " and " << newTick << std::endl;

				previoustick = newTick;
        		{
        		    std::lock_guard lock(gsMutex);
        		    pendingStates.push_back(std::move(gs));
        		}
				break;
			}
			case 1:
			{
				int i;
				payload.convert(i);
				std::cout << "PlayerId: " << i << " received!" << std::endl;
				playerId_ = i;
			}
		}

        // Release this message immediately
        pIncomingMsg->Release();
        pIncomingMsg = nullptr;
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
			std::printf( "Connected to server OK\n" );
			break;

		default:
			// Silences -Wswitch
			break;
	}
}

void ClientLogic::SendStateToServer(const InputState& state)
{
	msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> pk(buffer);
	pk.pack_array(2);
	pk.pack_uint8(0);
	pk.pack(state);

	SendBufferToServer(buffer);
};

void ClientLogic::SendBufferToServer(const msgpack::sbuffer& buffer)
{
	m_pInterface->SendMessageToConnection( m_hConnection, buffer.data(), buffer.size(), k_nSteamNetworkingSend_Reliable, nullptr );
};
