#include "network-bridge.h"

#include <chrono>
#include <thread>

#include "../../../engine/models/asset-manager.h"
#include "../../../engine/net-transport/client-transport.h"
#include "../../../engine/net-transport/server-transport.h"

#include "../../spawner/spawner.h"
#include "../../game-world/game-world.h"

#include <glm/glm.hpp>


NetworkBridge::NetworkBridge(Role role, const std::string& serverAddr, int port) : role_ (role)
{
    if (role == NetworkBridge::Role::Client)
    {
        client_ = std::make_unique<ClientTransport>(serverAddr);
    }
    else
    {
        server_ = std::make_unique<ServerTransport>(port);
    }
}

NetworkBridge::~NetworkBridge() = default;

void NetworkBridge::ManageGameStateDistribution(Scene& scene, float dT)
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

        auto [defBuffer, tranBuffer] = BuildAndPackGameState(scene);

        if (defBuffer.size() > 0)
        {
            std::span<const std::byte> defBytes {
                reinterpret_cast<const std::byte*>(defBuffer.data()),
                defBuffer.size()
            };
            server_->Broadcast(defBytes, true);
        }

        std::span<const std::byte> tranBytes {
            reinterpret_cast<const std::byte*>(tranBuffer.data()),
            tranBuffer.size()
        };
        server_->Broadcast(tranBytes, false);

        scene.ClearAddedModels();
        pendingRemoved_.clear();
	}
}

void NetworkBridge::RespawnPlayers(GameWorld& world, AssetManager& assMan)
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
		pk.pack_uint8(0);
		pk.pack(playerId);

        playersToConnections_.emplace(playerId, connId);
        connectionsToPlayers_[connId] = playerId;

        std::span<const std::byte> bytes {
            reinterpret_cast<const std::byte*>(buffer.data()),
            buffer.size()
        };
		server_->Send(connId, bytes, true);
    }
}

void NetworkBridge::PollEvents(GameWorld& world, AssetManager& assMan)
{
    if (role_ == Role::Server)
        PollInternalServer(world, assMan);
    else
        PollInternalClient();
}

std::tuple<msgpack::sbuffer, msgpack::sbuffer> NetworkBridge::BuildAndPackGameState(const Scene& scene, bool fullState)
{
    auto [definitiveState, transientState] = BuildGameState(scene, fullState);

    msgpack::sbuffer definitiveBuffer;

    if (fullState || !definitiveState.createdEntities.empty() || !definitiveState.destroyedEntities.empty())
    {
        msgpack::packer<msgpack::sbuffer> dPK(definitiveBuffer);
        dPK.pack_array(2);
        dPK.pack_uint8(1);
        dPK.pack(definitiveState);
    }

    msgpack::sbuffer transientBuffer;
	msgpack::packer<msgpack::sbuffer> tPK(transientBuffer);
	tPK.pack_array(2);
	tPK.pack_uint8(2);
	tPK.pack(transientState);

    return std::tuple(std::move(definitiveBuffer), std::move(transientBuffer));
}

std::tuple<GameState, GameState> NetworkBridge::BuildGameState(const Scene& scene, bool fullState)
{
    GameState transientState;
    GameState definitiveState;

    transientState.tick = currentTick_;
    definitiveState.tick = currentTick_;

    // Destroyed entities
    for (unsigned int id : pendingRemoved_)
    {
        definitiveState.destroyedEntities.push_back(id);
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

            definitiveState.createdEntities.push_back(e);
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
	
				transientState.entities.push_back(e);
			}
        }
    }

    return std::tuple(definitiveState, transientState);
}

void NetworkBridge::PollInternalServer(GameWorld& world, AssetManager& assMan)
{
    auto events = server_->PollEvents();

    for (auto& ev : events)
    {
        switch (ev.kind)
        {
            case ServerTransport::Event::Kind::Connected:
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
			    pk.pack_uint8(0);
			    // ToDo: Re-evaluate if sending playerID like this is correct (or just good enough for now)
			    pk.pack(playerId);

                playersToConnections_.emplace(playerId, ev.conn);
                connectionsToPlayers_.emplace(ev.conn, playerId);

                std::span<const std::byte> bytes {
                    reinterpret_cast<const std::byte*>(buffer.data()),
                    buffer.size()
                };
			    server_->Send(ev.conn, bytes, true);

                // Send complete state of Scene to this client only - one-time setup.
                // Possible ToDo: If this ever grows, maybe add second method so I don't throw a completely packed object away
                auto [gsBuffer, _] = BuildAndPackGameState(world.GetScene(), true);

                std::span<const std::byte> gsBytes {
                    reinterpret_cast<const std::byte*>(gsBuffer.data()),
                    gsBuffer.size()
                };
                server_->Send(ev.conn, gsBytes, true);

                break;
            }
            case ServerTransport::Event::Kind::Disconnected:
            {
                // Just accept and remove that player for now. Maybe think about reconnect logic, or just destroying playermodel
                
                auto playerId = connectionsToPlayers_[ev.conn];
                playersToConnections_.erase(playerId);
                connectionsToPlayers_.erase(ev.conn);
                world.MarkEntityForDelete(playerId);

                std::cout << "Disconnected event: " << ev.conn << std::endl;
                break;
            }
            case ServerTransport::Event::Kind::Message:
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

void NetworkBridge::SendInputState(InputState& state)
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

void NetworkBridge::PollInternalClient()
{
    auto events = client_->PollEvents();

    for (auto& ev : events)
    {
        switch (ev.kind)
        {
            case ClientTransport::Event::Kind::Connected:
            {
                std::cout << "Connected to server OK\n";
                break;
            }
            case ClientTransport::Event::Kind::Disconnected:
            {
                break;
            }
            case ClientTransport::Event::Kind::Message:
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
		        		int i;
		        		payload.convert(i);
		        		std::cout << "PlayerId: " << i << " received!" << std::endl;
		        		playerId_ = i;
                        break;
		        	}
		        	case 1:
                    case 2:
		        	{
		        		GameState gs;
		        		payload.convert(gs);
		        		int newTick = gs.tick;
		        		if (previousTick_ != 0 && newTick - previousTick_ != 1 && newTick != previousTick_)
                			std::cout <<   "Skipped tick(1) between: " << previousTick_ << " and " << newTick << std::endl;
                    
		        		previousTick_ = newTick;
                        timeAtLastTick_ = std::chrono::steady_clock::now();
                        
                        if (type == 2 || gs.entities.size() != 0)
                        {
                            inter_.FeedSnapshot(previousTick_, gs.entities);
                            gs.entities.clear();
                        }

                        pendingStates_.push_back(std::move(gs));

		        		break;
		        	}
                }
            }
        }
    }
}

void NetworkBridge::MergeClientWithNetwork(GameWorld& gameWorld, AssetManager& assMan)
{
    GameState gs;

    float timeSinceLastTick = std::chrono::duration<float>(
        std::chrono::steady_clock::now() - timeAtLastTick_
    ).count();
    float serverTimeNow = (previousTick_ * tickRate_) + timeSinceLastTick;
    float renderTime = serverTimeNow - inter_.lerp_;
	
    if (pendingStates_.empty()) return;


    if (pendingStates_[0].tick >= (renderTime / tickRate_)) return;

    gs = std::move(pendingStates_.front());
    pendingStates_.pop_front();

    if (gs.tick == 0) 
		return;

	gameWorld.GetScene().currentTick = gs.tick;

	for (uint32_t id : gs.destroyedEntities)
    {		
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
 
    inter_.InterpolateGameState(gameWorld.GetScene(), renderTime);
    
	return;
}

