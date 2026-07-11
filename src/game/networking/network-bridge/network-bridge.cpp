#include "network-bridge.h"

#include <chrono>

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
    else if (role == NetworkBridge::Role::Server)
    {
        server_ = std::make_unique<ServerTransport>(port);
    }
}

NetworkBridge::~NetworkBridge() = default;

void NetworkBridge::ManageGameStateDistribution(GameWorld& gameWorld, float dT)
{
    auto& scene = gameWorld.GetScene();
    uint32_t newClientId = 0;
	const auto now = std::chrono::steady_clock::now();

    for (uint32_t id : scene.GetRemoveMarkedModels())
    {
        if (std::find(pendingRemoved_.begin(), pendingRemoved_.end(), id) == pendingRemoved_.end())
            pendingRemoved_.push_back(id);
    }

    for (uint32_t id : scene.GetAddedModels())
    {
        if (std::find(pendingAdded_.begin(), pendingAdded_.end(), id) == pendingAdded_.end())
            pendingAdded_.push_back(id);
    }

	tickTimer_ += dT;


	if (tickTimer_ >= tickRate_)
	{
		currentTick_++;
		tickTimer_ -= tickRate_;

        auto [defBuffer, tranBuffer] = BuildAndPackGameState(gameWorld);

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

        pendingAdded_.clear();
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

std::tuple<msgpack::sbuffer, msgpack::sbuffer> NetworkBridge::BuildAndPackGameState(const GameWorld& gameWorld, bool fullState)
{
    auto [definitiveState, transientState] = BuildGameState(gameWorld, fullState);

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

std::tuple<GameState, GameState> NetworkBridge::BuildGameState(const GameWorld& gameWorld, bool fullState)
{
    GameState transientState;
    GameState definitiveState;
    const auto& scene = gameWorld.GetScene();

    transientState.tick = currentTick_;
    definitiveState.tick = currentTick_;

    definitiveState.playerToLastProcessedInput = latestInputTickPerPlayer_;
    transientState.playerToLastProcessedInput  = latestInputTickPerPlayer_;

    // Destroyed entities
    for (unsigned int id : pendingRemoved_)
    {
        definitiveState.destroyedEntities.push_back(id);
    }

    for (const auto& [id, model] : scene.GetModels())
    {
        const bool isNew = std::find(pendingAdded_.begin(), pendingAdded_.end(), id) != pendingAdded_.end();

        if (isNew || fullState)
        {
            // --- Full creation data ---
            EntityCreationState e;
            e.id                 = id;
            e.type               = static_cast<uint32_t>(model.type_);
            e.ownerId            = model.type_ == ModelType::SHOT ? gameWorld.GetShotData(id).ownerId : 0;
            e.radius             = model.GetRadius();
            e.position           = model.GetPosition();
            e.scale              = model.GetScale();
            e.rotation           = model.GetRotation();
            e.velocity           = model.GetVelocity();
            e.angularVelocity    = model.GetRotationSpeed();
            e.sourceTick         = model.type_ == ModelType::SHOT ? gameWorld.GetShotData(id).creationTick : 0;

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

    for (auto& [id, pd] : gameWorld.GetPlayerData())
    {
        PlayerDataState pds { id, pd.lastHit, pd.lifes };
        transientState.playerData.push_back(pds);
    }

    for (auto& [id, npcd] : gameWorld.GetNpcData())
    {
        NpcDataState nds { id, npcd.lastHit, npcd.lifes };
        transientState.npcData.push_back(nds);
    }

    // ToDo: revisit on server side replay implementation
    //if (!pastStates_.contains(currentTick_))
    //    pastStates_.emplace(currentTick_, transientState);

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
                auto [gsBuffer, _] = BuildAndPackGameState(world, true);

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

                        auto id = connectionsToPlayers_.at(ev.conn);
                        inputStates_[id] = is;
                        latestInputTickPerPlayer_[id] = is.tick;

                        //ReplayInputState(id);

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

void NetworkBridge::ReplayInputState(uint32_t playerId)
{
    auto& inputState = inputStates_[playerId];

    
    if (!pastStates_.contains(inputState.tick))
    return;
    
    auto& oldGameState = pastStates_[inputState.tick];
    
    // ToDo: Re-Calculate actions for player
}

void NetworkBridge::SendInputState(InputState& state)
{
    state.tick = currentTick_++;

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

    sentInputStates_.emplace(state.tick, state);
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
                    
                        if (newTick != previousTick_)
                        {
                            previousTick_ = newTick;
                            timeAtLastTick_ = std::chrono::steady_clock::now();
                        }
                        
                        if (type == 2 || gs.entities.size() != 0)
                        {
                            inter_.FeedSnapshot(previousTick_, gs.entities);

                            for (auto& ent : gs.entities)
                            {
                                if (ent.id != playerId_) continue;

                                auto ack = gs.playerToLastProcessedInput.find(playerId_);

                                if (ack != gs.playerToLastProcessedInput.end() && gs.tick >= latestStateTick_)
                                {
                                    latestAckTick_      = ack->second;
                                    latestStateTick_    = gs.tick;
                                    latestPlayerState_  = ent;
                                    latestStateValid_   = true;
                                }

                                break;
                            }

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

void NetworkBridge::MergeClientWithNetwork(GameWorld& gameWorld, AssetManager& assMan, bool predictiveClient)
{
    float renderTime = CalculateRenderTime();

    for (auto& gs : pendingStates_)
    {
        if (gs.eventsApplied) continue;
        gs.eventsApplied = true;
        if (gs.tick == 0) continue;

        for (auto& data : gs.playerData)
        {
            if (!gameWorld.IsPlayer(data.id)) continue;
            auto& pd = gameWorld.GetPlayerData(data.id);
            pd.lastHit = data.lastHit;
            pd.lifes = data.lifes;
        }

        for (auto& data : gs.npcData)
        {
            if (!gameWorld.IsNpc(data.id)) continue;
            auto& npcData = gameWorld.GetNpcData(data.id);
            npcData.lastHit = data.lastHit;
            npcData.lifes = data.lifes;
        }

        if (predictiveClient)
        {
            for (uint32_t id : gs.destroyedEntities)
                if (gameWorld.IsShot(id))
                    gameWorld.MarkEntityForDelete(id);

            for (auto& entity : gs.createdEntities)
            {
                if (entity.type != 1) continue;

                PhysicalInfo pi;
	    	    pi.position_			= entity.position;
	    	    pi.rotation_			= entity.rotation;
	    	    pi.angularVelocity_		= entity.angularVelocity;
	    	    pi.scale_				= entity.scale;
	    	    pi.velocity_			= entity.velocity;

                if (entity.ownerId == playerId_)
                    {
                        auto match = pendingShotCreations.find(entity.sourceTick);

                        if (match != pendingShotCreations.end())
                        {
                            gameWorld.GetScene().ReassignId(match->second, entity.id);
                            gameWorld.ReassignShotId(match->second, entity.id);
                            pendingShotCreations.erase(match);
                        }
                        else
                        {
                            // ToDo: Maybe revisit, a bit "hacky" right now
                            if (pendingShotCreations.size() > 0 && pendingShotCreations.begin()->first > entity.sourceTick + 50)
                                pendingShotCreations.erase(pendingShotCreations.begin());
                        }
                    }
                    else
                    {
                        float shotAge = std::max(0.0f, (serverClock_ + renderDelay_) - gs.tick * tickRate_);
                        spawner::SpawnShotFromNetwork(gameWorld, assMan, pi, entity.ownerId, entity.id, shotAge);
                    }
            }
        }
    }

    while (!pendingStates_.empty())
    {
        const auto& first = pendingStates_.front();

        if (first.tick * tickRate_ > renderTime) break;
        if (first.tick == 0) 
        {
            pendingStates_.pop_front();
            continue;
        }

        auto gs = std::move(pendingStates_.front());
        pendingStates_.pop_front();

        gameWorld.GetScene().currentTick = gs.tick;

	    for (uint32_t id : gs.destroyedEntities)
            gameWorld.MarkEntityForDelete(id);

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
	    			break;
	    		case 2: 
	    			spawner::SpawnNpc(gameWorld, assMan, pi, entity.id);
	    			break;
	    		default: break;
	    	}
	    }
    }

    inter_.InterpolateGameState(gameWorld.GetScene(), renderTime, playerId_, predictiveClient);

	return;
}

std::map<uint32_t, InputState>& NetworkBridge::ResetPlayerToLastInputState(GameWorld& world)
{
    if (!latestStateValid_ || playerId_ == 0 || !world.GetScene().ModelExists(playerId_))
        return emptyStates_;

    auto& playerModel = world.GetScene().GetModelByReference(playerId_);

        playerModel.SetPosition(latestPlayerState_.position);
    playerModel.SetScale(latestPlayerState_.scale);
    playerModel.SetRotation(latestPlayerState_.rotation);
    playerModel.SetVelocity(latestPlayerState_.velocity);
    playerModel.SetRotationSpeed(latestPlayerState_.angularVelocity);

    sentInputStates_.erase(sentInputStates_.begin(), sentInputStates_.upper_bound(latestAckTick_));

    return sentInputStates_;
}

float NetworkBridge::CalculateRenderTime()
{
    float k = 0.75;
    float maxAdj = 0.05;

    auto now = std::chrono::steady_clock::now();
    float timeSinceLastTick = std::chrono::duration<float>(
        now - timeAtLastTick_
    ).count();
    float target = previousTick_ * tickRate_ + timeSinceLastTick;

    if (!serverClockInit_)
    {
        serverClock_ = target;
        serverClockInit_ = true;
    }
    else
    {
        float dT = std::chrono::duration<float>(now - lastUpdateTime_).count();

        serverClock_ += dT;

        float drift = target - serverClock_;

        ++renderTimeDriftNum_;
        renderTimeDriftSum_ += drift;
        if (drift > renderTimeDriftMax_)
            renderTimeDriftMax_ = drift;
        
        if (std::abs(drift) > 0.2f) serverClock_ = target;
        else                        serverClock_ += dT * (std::clamp(drift * k, -maxAdj, +maxAdj));
        //std::cout << "Updated servertime. Drift: " << drift << std::endl;
    }

    lastUpdateTime_ = now;
    return serverClock_ - renderDelay_;
}
