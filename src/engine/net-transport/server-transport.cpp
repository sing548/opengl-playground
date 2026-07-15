#include "server-transport.h"

#include <map>
#include <cassert>
#include <utility>
#include <iostream>

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

namespace {
    ServerTransport::Impl* pCallbackInstance_ = nullptr;
    void SteamNetConnectionStatusChangedCallbackServer(SteamNetConnectionStatusChangedCallback_t* pInfo);
}

struct ServerTransport::Impl{
    ISteamNetworkingSockets* pInterface_ = nullptr;
    std::map<HSteamNetConnection, ConnectionId> mapClients_;

    HSteamNetPollGroup hPollGroup_ = k_HSteamNetPollGroup_Invalid;
    HSteamListenSocket hListenSock_ = k_HSteamListenSocket_Invalid;
    uint32_t nextConnectionId_ = 1;
    std::vector<Event> pending_;

    explicit Impl(uint16_t port)
    {

        #ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
	    	SteamDatagramErrMsg errMsg;
	    	if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
            {
                std::cerr << "GameNetworkingSockets_Init failed.  " << errMsg << std::endl;
                throw;
            }
	    		//FatalError( "GameNetworkingSockets_Init failed.  %s", errMsg );
	    #else
	    	SteamDatagram_SetAppID( 4888 ); // Just set something, doesn't matter what
	    	SteamDatagram_SetUniverse( false, k_EUniverseDev );

	    	SteamDatagramErrMsg errMsg;
	    	if ( !SteamDatagramClient_Init( errMsg ) )
	    		FatalError( "SteamDatagramClient_Init failed.  %s", errMsg );

	    	// Disable authentication when running with Steam, for this
	    	// example, since we're not a real app.
	    	//
	    	// Authentication is disabled automatically in the open-source
	    	// version since we don't have a trusted third party to issue
	    	// certs.
	    	SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1 );
	    #endif



        std::printf("Opening listening on port %d\n", port);
        pInterface_ = SteamNetworkingSockets();

        SteamNetworkingIPAddr serverAddress;
        serverAddress.Clear();
        serverAddress.m_port = port;

        SteamNetworkingConfigValue_t opt;
        opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, reinterpret_cast<void*>(SteamNetConnectionStatusChangedCallbackServer) );
        
        hListenSock_ = pInterface_->CreateListenSocketIP( serverAddress, 1, &opt );

        if ( hListenSock_ == k_HSteamListenSocket_Invalid )
            std::cerr << "Failed to listen on port" << std::endl;

        hPollGroup_ = pInterface_->CreatePollGroup();

        if ( hPollGroup_ == k_HSteamNetPollGroup_Invalid )
            std::cerr << "Failed to listen on port" << std::endl;

        std::printf( "Server listening on port %d\n", port );
    }

    ~Impl()
    {
        for (auto& [conn, _] : mapClients_)
        {
        	pInterface_->CloseConnection(conn, 0, "Server shutting down", true);
        }
        mapClients_.clear();

        if (hListenSock_ != k_HSteamListenSocket_Invalid)
        {
        	pInterface_->CloseListenSocket(hListenSock_);
        	hListenSock_ = k_HSteamListenSocket_Invalid;
        }

        if (hPollGroup_ != k_HSteamNetPollGroup_Invalid)
        {
        	pInterface_->DestroyPollGroup(hPollGroup_);
        	hPollGroup_ = k_HSteamNetPollGroup_Invalid;
        }
    }

    void Send(ConnectionId to, std::span<const std::byte> bytes, bool reliable, bool noNagle)
    {
        for (auto& [conn, id] : mapClients_)
        {
            if (id == to)
            {
                SendPackageToClient(conn, bytes, reliable, noNagle);
                return;
            }
        }
        std::cerr << "Send: Unknown ConnectionId " << to << std::endl;
    }

    void Broadcast(std::span<const std::byte> bytes, bool reliable, bool noNagle)
    {
        auto flag = (reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable)
                    | (noNagle  ? k_nSteamNetworkingSend_NoNagle  : 0);
        for (auto& conn : mapClients_)
        {
            SendPackageToClient(conn.first, bytes, reliable, noNagle);
        }
    }

    std::vector<ServerTransport::Event> PollEvents()
    {
        pCallbackInstance_ = this;
        pInterface_->RunCallbacks();
        PollIncomingMessages();
        return std::exchange(pending_, {});
    }

    void PollIncomingMessages()
    {
        // ToDo: Maybe re-add multi-threading
        while ( true )
        {
        	ISteamNetworkingMessage *pIncomingMsg = nullptr;
        	int numMsgs = pInterface_->ReceiveMessagesOnPollGroup( hPollGroup_, &pIncomingMsg, 1 );
        	
            if ( numMsgs == 0 )
        		break;
        	
            if ( numMsgs < 0 )
            {
        		std::cerr << "Error checking for messages" << std::endl;
                break;
            }

        	assert( numMsgs == 1 && pIncomingMsg );
        	auto itClient = mapClients_.find( pIncomingMsg->m_conn );
        	assert( itClient != mapClients_.end() );
        
            const auto* data = reinterpret_cast<const std::byte*>(pIncomingMsg->m_pData);
            
            Event event;
            event.conn = itClient->second;
            event.kind = Event::Kind::Message;
            event.bytes.assign(data, data + pIncomingMsg->m_cbSize);
            
            pending_.push_back(std::move(event));
        	pIncomingMsg->Release();
        }
    }
    
    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
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
    			if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected )
    			{
    				auto itClient = mapClients_.find( pInfo->m_hConn );
    				assert( itClient != mapClients_.end() );

                    pending_.push_back(Event{ Event::Kind::Disconnected, itClient->second, {}});
    				mapClients_.erase( itClient );
                }
    			pInterface_->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
                break;
    		}

    		case k_ESteamNetworkingConnectionState_Connecting:
    		{
                assert( mapClients_.find( pInfo->m_hConn ) == mapClients_.end() );
    			std::printf( "Connection request from %s\n", pInfo->m_info.m_szConnectionDescription );

    			if ( pInterface_->AcceptConnection( pInfo->m_hConn ) != k_EResultOK )
    			{
    				pInterface_->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
    				std::printf( "Can't accept connection.  (It was already closed?)" );
    				break;
    			}

    			// Assign the poll group
    			if ( !pInterface_->SetConnectionPollGroup( pInfo->m_hConn, hPollGroup_ ) )
    			{
    				pInterface_->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
    				std::printf( "Failed to set poll group?" );
    				break;
    			}

                // ToDo: Get id in a sensible way, not like before
                ConnectionId id = nextConnectionId_++;
                mapClients_.emplace(pInfo->m_hConn, id);
                pending_.push_back(Event{ Event::Kind::Connected, id, {} });
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
    }
    
private:
    void SendPackageToClient(HSteamNetConnection conn, std::span<const std::byte> bytes, bool reliable, bool noNagle)
    {
        auto flag = (reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable)
                    | (noNagle  ? k_nSteamNetworkingSend_NoNagle  : 0);

        EResult res = pInterface_->SendMessageToConnection(conn, bytes.data(), bytes.size(), flag, nullptr);

        if (res != k_EResultOK)
            std::cout << "Send failed to conn: " << conn << ". " << res << "\n";
    }
};

namespace
{
    void SteamNetConnectionStatusChangedCallbackServer( SteamNetConnectionStatusChangedCallback_t *pInfo )
    {
        pCallbackInstance_->OnSteamNetConnectionStatusChanged( pInfo );
    };
}

ServerTransport::ServerTransport(uint16_t port) : impl_(std::make_unique<Impl>(port)) { }

ServerTransport::~ServerTransport() = default;

void ServerTransport::Send(ConnectionId to, std::span<const std::byte> bytes, bool reliable, bool noNagle)
{
    impl_->Send(to, bytes, reliable, noNagle);
}

void ServerTransport::Broadcast(std::span<const std::byte> bytes, bool reliable, bool noNagle)
{
    impl_->Broadcast(bytes, reliable, noNagle);
}

std::vector<ServerTransport::Event> ServerTransport::PollEvents()
{
    return impl_->PollEvents();
}

void ServerTransport::SetFakeNetwork(int lagMs, float pkgLossPct, float pkgJitterPct)
{
    auto* utils = SteamNetworkingUtils();

    utils->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Send, lagMs / 2);
    utils->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Recv, lagMs / 2);
    
    utils->SetGlobalConfigValueFloat(k_ESteamNetworkingConfig_FakePacketLoss_Send, pkgLossPct);
    utils->SetGlobalConfigValueFloat(k_ESteamNetworkingConfig_FakePacketLoss_Recv, pkgLossPct);

    utils->SetGlobalConfigValueFloat(k_ESteamNetworkingConfig_FakePacketJitter_Recv_Pct, pkgLossPct);
    utils->SetGlobalConfigValueFloat(k_ESteamNetworkingConfig_FakePacketJitter_Send_Pct, pkgLossPct);
}
