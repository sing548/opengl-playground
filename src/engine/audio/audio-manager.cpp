#include "audio-manager.h"

#include <filesystem>
#include <iostream>

#include "miniaudio.h"

#include "../helpers/file-helper.h"

AudioManager::AudioManager()
    : engine_(std::make_unique<ma_engine>())
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
    if (initialized_ && engine_) ma_engine_uninit(engine_.get());
}

void AudioManager::PlayOneShot(const std::string& relativeAssetPath)
{
    if (!initialized_) return;
    std::string full = (std::filesystem::path(FileHelper::GetAssetsDir()) / relativeAssetPath).string();
    ma_engine_play_sound(engine_.get(), full.c_str(), nullptr);
}
