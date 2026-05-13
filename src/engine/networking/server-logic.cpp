#include "server-logic.h"

HSteamNetPollGroup ServerLogic::m_hPollGroup = k_HSteamNetPollGroup_Invalid;  // Initialize poll group to invalid value
HSteamListenSocket ServerLogic::m_hListenSock = k_HSteamNetPollGroup_Invalid;  // Initialize listen socket to invalid valeu
ISteamNetworkingSockets* ServerLogic::m_pInterface = nullptr;  // Initialize pointer to null
std::map<HSteamNetConnection, ServerLogic::Client_t> ServerLogic::m_mapClients;  // Initialize map of clients
SteamNetworkingMicroseconds g_logTimeZero;
ServerLogic *ServerLogic::s_pCallbackInstance = nullptr;
std::chrono::steady_clock::time_point gameStateUpdateTime = std::chrono::steady_clock::now();

ServerLogic::~ServerLogic()
{
	for (auto& [conn, _] : m_mapClients)
	{
		m_pInterface->CloseConnection(conn, 0, "Server shutting down", true);
	}
	m_mapClients.clear();

	if (m_hListenSock != k_HSteamListenSocket_Invalid)
	{
		m_pInterface->CloseListenSocket(m_hListenSock);
		m_hListenSock = k_HSteamListenSocket_Invalid;
	}

	if (m_hPollGroup != k_HSteamNetPollGroup_Invalid)
	{
		m_pInterface->DestroyPollGroup(m_hPollGroup);
		m_hPollGroup = k_HSteamNetPollGroup_Invalid;
	}
}

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
	
    while (running) 
	{
        PollIncomingMessagesServer(running);
        PollConnectionStateChangesServer();
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
};

void ServerLogic::Shutdown()
{
	std::lock_guard<std::mutex> lock(mtx_);
	cv_.notify_all();
}

void ServerLogic::DistributeGameState(std::atomic<bool>& running)
{
    std::unique_lock<std::mutex> lock(mtx_);

    while (running)
    {
		cv_.wait(lock, [&]{
			return !running || (gameState_ != nullptr);
		});
	
		if (!running || gameState_ == nullptr) break;
	
		auto stateToSend = *gameState_;
		lastSyncedTick_ = gameState_->tick;
		lock.unlock();
		SendGameStateToAllClients(stateToSend);
		lock.lock();
		
		gameState_.reset();
    }
}

uint32_t ServerLogic::UpdateGameState(std::unique_ptr<GameState> gs)
{
	std::lock_guard<std::mutex> lock(mtx_);
	gameState_ = std::move(gs);
	gameStateUpdateTime = std::chrono::steady_clock::now();
	cv_.notify_one();

	uint32_t returnValue = 0;
	if (newClientConnected_)
		returnValue = lastConnectedClient_;
	newClientConnected_ = false;
	return returnValue;
}

void ServerLogic::PollIncomingMessagesServer(std::atomic<bool>& running)
{
	while ( running )
	{
		ISteamNetworkingMessage *pIncomingMsg = nullptr;
		int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup( m_hPollGroup, &pIncomingMsg, 1 );
		if ( numMsgs == 0 )
			break;
		if ( numMsgs < 0 )
			std::cerr << "Error checking for messages" << std::endl;

		assert( numMsgs == 1 && pIncomingMsg );

		{
			std::lock_guard<std::mutex> lock(clientsMtx_);
			auto itClient = m_mapClients.find( pIncomingMsg->m_conn );
			assert( itClient != m_mapClients.end() );
		}		

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
				InputState is;
				payload.convert(is);
				is.tick = lastSyncedTick_.load();

				std::lock_guard<std::mutex> lock(inputMtx_);
				auto& slot = inputStates_[is.id];

				slot.id		  = is.id;
				slot.tick	  = is.tick;
				slot.left	  = is.left;
				slot.right	  = is.right;
				slot.forward  = is.forward;
				slot.backward = is.backward;
				slot.shoot	  = is.shoot;
				
				slot.shootShot = slot.shootShot || is.shootShot;
				
				break;
			}
			case 1:
			{
				std::string sCmd;
				sCmd.assign( (const char *)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize );
				const char *cmd = sCmd.c_str();

				// We don't need this anymore.
				pIncomingMsg->Release();
				break;
			}
			default:
			{
				std::cerr << "Invalid message type" << std::endl;
				return;
			}
		}
	}
};

std::unordered_map<int, InputState> ServerLogic::GetLatestInputStates()
{
	std::lock_guard lock(inputMtx_);
	auto out = std::move(inputStates_);
	inputStates_.clear();
	return out;
};

void ServerLogic::PollConnectionStateChangesServer()
{
    s_pCallbackInstance = this;
	m_pInterface->RunCallbacks();
};

void ServerLogic::OnSteamNetConnectionStatusChangedServer( SteamNetConnectionStatusChangedCallback_t *pInfo )
	{
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
				uint32_t id = 0;
				{
					std::lock_guard<std::mutex> lock(clientsMtx_);
					// Locate the client.  Note that it should have been found, because this
					// is the only codepath where we remove clients (except on shutdown),
					// and connection change callbacks are dispatched in queue order.
					auto itClient = m_mapClients.find( pInfo->m_hConn );
					assert( itClient != m_mapClients.end() );
					id   = itClient->second.id;
					m_mapClients.erase( itClient );
				}

				const char *pszDebugLogAction;
				if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
				{
					pszDebugLogAction = "problem detected locally";
					std::printf("Departed id: %u", id);
				}
				else
				{
					// Note that here we could check the reason code to see if
					// it was a "usual" connection or an "unusual" one.
					pszDebugLogAction = "closed by peer";
					std::printf("Departed id: %u", id);
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
			{
				std::lock_guard<std::mutex> lock(clientsMtx_);
				// This must be a new connection
				assert( m_mapClients.find( pInfo->m_hConn ) == m_mapClients.end() );
			}
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

			unsigned int id = lastConnectedClient_;

			{
				std::lock_guard<std::mutex> lock(clientsMtx_);
				m_mapClients[ pInfo->m_hConn ];
			}

			msgpack::sbuffer buffer;
			msgpack::packer<msgpack::sbuffer> pk(buffer);
			pk.pack_array(2);
			pk.pack_uint8(1);
			// ToDo: This is hacky, make this better in future
			pk.pack(id - 1);

			SendPackageToClient(pInfo->m_hConn, buffer);
			//SendPackageToClient(id, buffer);

			shotTick[id - 1] = 0;
			newClientConnected_ = id;
			lastConnectedClient_--;
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
	
void ServerLogic::SendGameStateToAllClients(const GameState& stateToSend)
{
	const auto now = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lock(mtx_);


		deltaSamples_++;
        // Log once per second
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastLogTime_).count() >= 1)
        {
            /*std::cout
                << "Sent updates: " << deltaSamples_ << " times last second, "
                << std::endl;*/

            // Reset accumulators
            accumulatedDelta_ = std::chrono::nanoseconds{0};
            accumulatedTickInterval_ = std::chrono::nanoseconds{0};
            deltaSamples_ = 0;
            tickSamples_ = 0;
            lastLogTime_ = now;
        }
    }

	
	std::vector<HSteamNetConnection> conns;
	{
		std::lock_guard<std::mutex> lock(clientsMtx_);
		conns.reserve(m_mapClients.size());
		for (auto& c : m_mapClients) conns.push_back(c.first);
	}
	
	std::lock_guard<std::mutex> lock(mtx_);
	
	msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> pk(buffer);
	pk.pack_array(2);
	pk.pack_uint8(0);
	pk.pack(stateToSend);

	for (auto& conn : conns)
	{
		auto now = std::chrono::steady_clock::now();
		SendPackageToClient(conn, buffer);
	}
}

void ServerLogic::SendPackageToClient(HSteamNetConnection conn, const msgpack::sbuffer &buffer)
{
	EResult res = m_pInterface->SendMessageToConnection( conn, buffer.data(), buffer.size(), k_nSteamNetworkingSend_Reliable, nullptr );

	if (res != k_EResultOK)
		std::cout << "Send failed: " << res << " tick: " << gameState_->tick << "\n";
};

void ServerLogic::SendStringToClient( HSteamNetConnection conn, const char *str )
{
	m_pInterface->SendMessageToConnection( conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr );
};

void ServerLogic::SendStringToAllClients(const char* str, HSteamNetConnection except) { 
	std::vector<HSteamNetConnection> conns;
	{
		std::lock_guard<std::mutex> lock(clientsMtx_);
		for (auto& c : m_mapClients) {
        	if (c.first != except) conns.push_back(c.first);
		}
	}
	for (auto conn : conns)	SendStringToClient(conn, str);
    
};
