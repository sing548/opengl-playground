#include "client-transport.h"

#include <utility>
#include <cassert>
#include <iostream>

#include <steam/steam_api_common.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace
{
    ClientTransport::Impl* pCallbackInstance_ = nullptr;
    void SteamNetConnectionStatusChangedCallbackClient(SteamNetConnectionStatusChangedCallback_t* pInfo);
}

struct ClientTransport::Impl
{
    ISteamNetworkingSockets *pInterface_ = nullptr;
    HSteamNetConnection hConnection_ = k_HSteamNetConnection_Invalid;
    std::vector<Event> pending_;

    explicit Impl(const std::string& serverAddr)
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
        
        // Select instance to use.  For now we'll always use the default.
	    pInterface_ = SteamNetworkingSockets();

        SteamNetworkingIPAddr addr;
        addr.Clear();

        if (!addr.ParseString(serverAddr.c_str()))
        {
            std::cerr << "Client connection: bad address: " << serverAddr << std::endl;
            pending_.push_back(Event{ Event::Kind::Disconnected, {} });
            return;
        }

	    SteamNetworkingConfigValue_t opt;
	    opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, reinterpret_cast<void*>(SteamNetConnectionStatusChangedCallbackClient) );
        
        char szAddr[ SteamNetworkingIPAddr::k_cchMaxString ];
	    addr.ToString( szAddr, sizeof(szAddr), true );
	    std::printf( "Connecting as client to server at %s\n", szAddr );


	    hConnection_ = pInterface_->ConnectByIPAddress( addr, 1, &opt );
	    if ( hConnection_ == k_HSteamNetConnection_Invalid )
        {
            std::cerr << "Failed to create connection" << std::endl;
            pending_.push_back(Event{ Event::Kind::Disconnected, {} });
        }
        
    }

    ~Impl()
    {
        if (hConnection_ != k_HSteamNetConnection_Invalid)
        {
            pInterface_->CloseConnection(hConnection_, 0, "Bye", true);
            hConnection_ = k_HSteamNetConnection_Invalid;
        }
    }

    void Send(std::span<const std::byte> bytes, bool reliable)
    {
        if (hConnection_ == k_HSteamNetConnection_Invalid)
        {
            std::cerr << "Client sending failed: not connected!" << std::endl;
            return;
        }

        auto flag = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
        EResult res = pInterface_->SendMessageToConnection(hConnection_, bytes.data(), bytes.size(), flag, nullptr);

        if (res != k_EResultOK)
            std::cerr << "Client sending failed: " << res << std::endl;
    }

    std::vector<Event> PollEvents()
    {
        pCallbackInstance_ = this;
        pInterface_->RunCallbacks();
        ReadIncoming();
        return std::exchange(pending_, {});
    }

    void ReadIncoming()
    {
        ISteamNetworkingMessage* pIncomingMsg = nullptr;

        while (true)
        {
            int numMsgs = pInterface_->ReceiveMessagesOnConnection(hConnection_, &pIncomingMsg, 1);
            if (numMsgs == 0)
                break; // no more messages
            if (numMsgs < 0)
            {
                std::cerr << "Error receiving messages" << std::endl;
                break;
            }

            const auto* data = reinterpret_cast<const std::byte*>(pIncomingMsg->m_pData);

            Event e;
            e.kind = Event::Kind::Message;
            e.bytes.assign(data, data + pIncomingMsg->m_cbSize);
            pending_.push_back(std::move(e));

            // Release this message immediately
            pIncomingMsg->Release();
        }
    }

    void OnStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
    {
        assert( pInfo->m_hConn == hConnection_ || hConnection_ == k_HSteamNetConnection_Invalid );

	    // What's the state of the connection?
	    switch ( pInfo->m_info.m_eState )
	    {
	    	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	    	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
                std::printf("Disconnected\n");
                pending_.push_back(Event{ Event::Kind::Disconnected, {} });
	    		pInterface_->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
	    		hConnection_ = k_HSteamNetConnection_Invalid;
	    		break;

	    	case k_ESteamNetworkingConnectionState_Connecting:
	    		// We will get this callback when we start connecting.
	    		// We can ignore this.
	    		break;

	    	case k_ESteamNetworkingConnectionState_Connected:
	    		std::printf( "Connected to server OK\n" );
                pending_.push_back(Event{ Event::Kind::Connected, {} });
	    		break;

	    	default:
	    		// Silences -Wswitch
	    		break;
	    }
    }
};

namespace
{
    void SteamNetConnectionStatusChangedCallbackClient(SteamNetConnectionStatusChangedCallback_t* pInfo)
    {
        pCallbackInstance_->OnStatusChanged(pInfo);
    }
}

ClientTransport::ClientTransport(const std::string& serverAddr) : impl_(std::make_unique<Impl>(serverAddr)) { }

ClientTransport::~ClientTransport() = default;

void ClientTransport::Send(std::span<const std::byte> bytes, bool reliable)
{
    impl_->Send(bytes, reliable);
}

std::vector<ClientTransport::Event> ClientTransport::PollEvents()
{
    return impl_->PollEvents();
}
