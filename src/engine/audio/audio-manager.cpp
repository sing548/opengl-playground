#include "audio-manager.h"

#include <filesystem>
#include <iostream>

#include "miniaudio.h"

#include "../helpers/file-helper.h"

AudioManager::AudioManager()
    : engine_(std::make_unique<ma_engine>()),
      rng_(std::random_device{}())
{
    if (ma_engine_init(nullptr, engine_.get()) != MA_SUCCESS)
    {
        std::cerr << "AudioManager: failed to initialize miniaudio engine" << std::endl;
        engine_.reset();
        return;
    }
    initialized_ = true;
}

AudioManager::~AudioManager()
{
    for (auto& s : active_) ma_sound_uninit(s.get());
    active_.clear();
    if (initialized_ && engine_) ma_engine_uninit(engine_.get());
}

std::string AudioManager::ResolvePath(const std::string& rel) const
{
    return (std::filesystem::path(FileHelper::GetAssetsDir()) / rel).string();
}

void AudioManager::ReapFinished()
{
    std::erase_if(active_, [](const std::unique_ptr<ma_sound>& s) {
        if (ma_sound_at_end(s.get()))
        {
            ma_sound_uninit(s.get());
            return true;
        }
        return false;
    });
}

void AudioManager::PlayOneShot(const std::string& relativeAssetPath)
{
    if (!initialized_) return;
    ma_engine_play_sound(engine_.get(), ResolvePath(relativeAssetPath).c_str(), nullptr);
}

void AudioManager::PlayOneShotPitched(const std::string& relativeAssetPath, float minPitch, float maxPitch)
{
    if (!initialized_) return;
    ReapFinished();

    auto sound = std::make_unique<ma_sound>();
    if (ma_sound_init_from_file(engine_.get(), ResolvePath(relativeAssetPath).c_str(),
                                MA_SOUND_FLAG_DECODE, nullptr, nullptr, sound.get()) != MA_SUCCESS)
    {
        std::cerr << "AudioManager: failed to load " << relativeAssetPath << std::endl;
        return;
    }

    std::uniform_real_distribution<float> dist(minPitch, maxPitch);
    ma_sound_set_pitch(sound.get(), dist(rng_));
    ma_sound_start(sound.get());
    active_.push_back(std::move(sound));
}
