#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <memory>
#include <random>
#include <string>
#include <vector>

struct ma_engine;
struct ma_sound;

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void PlayOneShot(const std::string& relativeAssetPath);
    void PlayOneShotPitched(const std::string& relativeAssetPath, float minPitch, float maxPitch);

private:
    void ReapFinished();
    std::string ResolvePath(const std::string& rel) const;

    std::unique_ptr<ma_engine> engine_;
    std::vector<std::unique_ptr<ma_sound>> active_;
    std::mt19937 rng_;
    bool initialized_ = false;
};

#endif
