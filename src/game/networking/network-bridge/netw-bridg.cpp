#include "netw-bridg.h"

#include <thread>

#include "../../../engine/models/asset-manager.h"
#include "../../../engine/net-transport/cli-transp.h"
#include "../../../engine/net-transport/serv-transp.h"

#include "../../../engine/networking/shared-strucs.h"

#include "../../spawner/spawner.h"
#include "../../game-world/game-world.h"


NetwBridg::NetwBridg(Role role, const std::string& serverAddr) : role_ (role)
{
    if (role == NetwBridg::Role::Client)
    {
        client_ = std::make_unique<CliTransp>(serverAddr);
    }
    else
    {
        // ToDo: Read ID from serverAddr
        int port = 5001;
        server_ = std::make_unique<ServTransp>(port);
    }
}

NetwBridg::~NetwBridg() = default;

void NetwBridg::ManageGameStateDistribution(Scene& scene, float dT)
{

    uint32_t newClientId = 0;
	const auto now = std::chrono::steady_clock::now();

    for (uint32_t id : scene.GetRemoveMarkedModels())
    {
        if (std::find(pendingRemoved_.begin(), pendingRemoved_.end(), id) == pendingRemoved_.end())
            pendingRemoved_.push_back(id);
    }

	tickTimer_ += dT;


	if (tickTimer_ >= tickRate_)
	{
		currentTick_++;
		tickTimer_ -= tickRate_;

        auto gameState = BuildGameState(scene);

        msgpack::sbuffer buffer;
	    msgpack::packer<msgpack::sbuffer> pk(buffer);
	    pk.pack_array(2);
	    pk.pack_uint8(0);
	    pk.pack(gameState);
        
        std::span<const std::byte> bytes {
            reinterpret_cast<const std::byte*>(buffer.data()),
            buffer.size()
        };

        server_->Broadcast(bytes, true);
        scene.ClearAddedModels();
        pendingRemoved_.clear();
	}
}

void NetwBridg::RespawnPlayers(GameWorld& world, AssetManager& assMan)
{
    for (auto& [connId, _] : connectionsToPlayers_)
    {
        // Spawn new player-model and send id to client -> Client can take "ownership"
        PhysicalInfo pi = PhysicalInfo();
        // ToDo: add some randomness, so 2 players connecting at the same time dont spawn on top of each other.
        pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
        pi.rotation_ = glm::quat(1, 0, 0, 0);
        pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);

        uint32_t playerId = spawner::SpawnPlayer(world, assMan, pi);

        msgpack::sbuffer buffer;
		msgpack::packer<msgpack::sbuffer> pk(buffer);
		pk.pack_array(2);
		pk.pack_uint8(1);
		pk.pack(playerId);

        playersToConnections_.emplace(playerId, connId);
        connectionsToPlayers_[connId] = playerId;

        std::span<const std::byte> bytes {
            reinterpret_cast<const std::byte*>(buffer.data()),
            buffer.size()
        };
		server_->Send(connId, bytes, true);

        /*auto gsBuffer = BuildAndPackGameState(world.GetScene(), true);

        std::span<const std::byte> gsBytes {
            reinterpret_cast<const std::byte*>(gsBuffer.data()),
            gsBuffer.size()
        };
        server_->Send(connId, gsBytes, true);*/
    }
}

void NetwBridg::PollEvents(GameWorld& world, AssetManager& assMan)
{
    if (role_ == Role::Server)
        PollInternalServer(world, assMan);
    else
        PollInternalClient();
}

msgpack::sbuffer NetwBridg::BuildAndPackGameState(const Scene& scene, bool fullState)
{
    auto gameState = BuildGameState(scene, fullState);

    msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> pk(buffer);
	pk.pack_array(2);
	pk.pack_uint8(0);
	pk.pack(gameState);

    return buffer;
}

GameState NetwBridg::BuildGameState(const Scene& scene, bool fullState)
{
    GameState gs;
    gs.tick = currentTick_;

    // Destroyed entities
    for (unsigned int id : pendingRemoved_)
    {
        gs.destroyedEntities.push_back(id);
    }

    auto& addedModels = scene.GetAddedModels();

    for (const auto& [id, model] : scene.GetModels())
    {
        const bool isNew = std::find(addedModels.begin(), addedModels.end(), id) != addedModels.end();

        if (isNew || fullState)
        {
            // --- Full creation data ---
            EntityCreationState e;
            e.id                 = id;
            e.type               = static_cast<uint32_t>(model.type_);
            e.radius             = model.GetRadius();
            e.position           = model.GetPosition();
            e.scale              = model.GetScale();
            e.rotation           = model.GetRotation();
            e.velocity           = model.GetVelocity();
            e.angularVelocity    = model.GetRotationSpeed();

            gs.createdEntities.push_back(e);
        }
        else
        {
			if (model.type_ == ModelType::PLAYER || model.type_ == ModelType::NPC)
			{
				// ToDo: "Lightweight" state update ---
				EntityState e;
				e.id                = id;
				e.position          = model.GetPosition();
				e.scale             = model.GetScale();
				e.rotation          = model.GetRotation();
				e.velocity          = model.GetVelocity();
				e.angularVelocity   = model.GetRotationSpeed();
	
				gs.entities.push_back(e);
			}
        }
    }

    return gs;
}

void NetwBridg::PollInternalServer(GameWorld& world, AssetManager& assMan)
{
    auto events = server_->PollEvents();

    for (auto& ev : events)
    {
        switch (ev.kind)
        {
            case ServTransp::Event::Kind::Connected:
            {
                std::cout << "Connected event: " << ev.conn << std::endl;

                // Spawn new player-model and send id to client -> Client can take "ownership"
                PhysicalInfo pi = PhysicalInfo();
                // ToDo: add some randomness, so 2 players connecting at the same time dont spawn on top of each other.
                pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
                pi.rotation_ = glm::quat(1, 0, 0, 0);
                pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);

                uint32_t playerId = spawner::SpawnPlayer(world, assMan, pi);

                msgpack::sbuffer buffer;
			    msgpack::packer<msgpack::sbuffer> pk(buffer);
			    pk.pack_array(2);
			    pk.pack_uint8(1);
			    // ToDo: Re-evaluate if sending playerID like this is correct (or just good enough for now)
			    pk.pack(playerId);

                playersToConnections_.emplace(playerId, ev.conn);
                connectionsToPlayers_.emplace(ev.conn, playerId);

                std::span<const std::byte> bytes {
                    reinterpret_cast<const std::byte*>(buffer.data()),
                    buffer.size()
                };
			    server_->Send(ev.conn, bytes, true);

                // Send complete state of Scene to this client only - one-time setup
                auto gsBuffer = BuildAndPackGameState(world.GetScene(), true);

                std::span<const std::byte> gsBytes {
                    reinterpret_cast<const std::byte*>(gsBuffer.data()),
                    gsBuffer.size()
                };
                server_->Send(ev.conn, gsBytes, true);

                break;
            }
            case ServTransp::Event::Kind::Disconnected:
            {
                // Just accept and remove that player for now. Maybe think about reconnect logic, or just destroying playermodel
                
                auto playerId = connectionsToPlayers_[ev.conn];
                playersToConnections_.erase(playerId);
                connectionsToPlayers_.erase(ev.conn);

                std::cout << "Disconnected event: " << ev.conn << std::endl;
                break;
            }
            case ServTransp::Event::Kind::Message:
            {
                msgpack::object_handle oh = msgpack::unpack(
		        	reinterpret_cast<const char*>(ev.bytes.data()), ev.bytes.size());
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
		        		is.tick = currentTick_;

                        auto id = connectionsToPlayers_.at(ev.conn);
                        inputStates_[id] = is;

		        		break;
		        	}
		        	case 1:
		        	{
		        		// This is a placeholder for other messages. Just leave be for now
                        std::cout << "Some other message from id: " << ev.conn << std::endl;
		        		break;
		        	}
		        	default:
		        	{
		        		std::cerr << "Invalid message type" << std::endl;
		        		return;
		        	}
		        }
                break;
            }
            default:
                break;
        }
    }
}

void NetwBridg::SendInputState(InputState& state)
{
    msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> pk(buffer);
	pk.pack_array(2);
	pk.pack_uint8(0);
	pk.pack(state);

    std::span<const std::byte> bytes {
        reinterpret_cast<const std::byte*>(buffer.data()),
        buffer.size()
    };

    client_->Send(bytes , true);
}

void NetwBridg::PollInternalClient()
{
    auto events = client_->PollEvents();

    for (auto& ev : events)
    {
        switch (ev.kind)
        {
            case CliTransp::Event::Kind::Connected:
            {
                std::cout << "Connected to server OK\n";
                break;
            }
            case CliTransp::Event::Kind::Disconnected:
            {
                break;
            }
            case CliTransp::Event::Kind::Message:
            {
                // Unpack message
                msgpack::object_handle oh = msgpack::unpack(
                    reinterpret_cast<const char*>(ev.bytes.data()), ev.bytes.size());
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
		        		if (previousTick_ != 0 && newTick - previousTick_ != 1)
                			std::cout <<   "Skipped tick(1) between: " << previousTick_ << " and " << newTick << std::endl;
                    
		        		previousTick_ = newTick;
                        pendingStates_.push_back(std::move(gs));
		        		break;
		        	}
		        	case 1:
		        	{
		        		int i;
		        		payload.convert(i);
		        		std::cout << "PlayerId: " << i << " received!" << std::endl;
		        		playerId_ = i;
                        break;
		        	}
                }
            }
        }
    }
}

std::tuple<unsigned int, std::vector<uint32_t>> NetwBridg::MergeClientWithNetwork(GameWorld& gameWorld, AssetManager& assMan)
{
    GameState gs;
	std::vector<uint32_t> killedPlayers;
	
    if (pendingStates_.size() == 0) return { 0, killedPlayers };

    gs = std::move(pendingStates_.front());
    pendingStates_.pop_front();

    if (gs.tick == 0) 
		return { 0, killedPlayers };

	gameWorld.GetScene().currentTick = gs.tick;

	for (uint32_t id : gs.destroyedEntities)
    {
		auto& players = gameWorld.GetPlayerData();
		if (players.contains(id)) 
			killedPlayers.push_back(id);
			
        gameWorld.MarkEntityForDelete(id);
    }

	for (auto &entity : gs.createdEntities)
	{
		PhysicalInfo pi;
		pi.position_			= entity.position;
		pi.rotation_			= entity.rotation;
		pi.angularVelocity_		= entity.angularVelocity;
		pi.scale_				= entity.scale;
		pi.velocity_			= entity.velocity;
	
		switch (entity.type) 
		{
			case 0: 
				spawner::SpawnPlayer(gameWorld, assMan, pi, entity.id);
				break;
			case 1: 
				spawner::SpawnShotFromNetwork(gameWorld, assMan, pi, entity.id);
				break;
			case 2: 
				spawner::SpawnNpc(gameWorld, assMan, pi, entity.id);
				break;
			default: break;
		}
	}

	for (const auto& entity : gs.entities)
    {
        if (!gameWorld.GetScene().ModelExists(entity.id))
        {
            std::cout << "Entity not loaded: " << entity.id << std::endl;
            continue;
        }

        auto& m = gameWorld.GetScene().GetModelByReference(entity.id);

        m.SetPosition(entity.position);
        m.SetRotation(entity.rotation);
        m.SetRotationSpeed(entity.angularVelocity);
        m.SetScale(entity.scale);
        m.SetVelocity(entity.velocity);
    }
 
	return { playerId_, killedPlayers };
}
