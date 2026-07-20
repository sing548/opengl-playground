#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <memory>
#include <string>

struct ma_engine;

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void PlayOneShot(const std::string& relativeAssetPath);

private:
    std::unique_ptr<ma_engine> engine_;
    bool initialized_ = false;
};

#endif
