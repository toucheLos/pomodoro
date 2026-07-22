#pragma once

#include <QString>
#include <memory>

// Thin wrapper around a single miniaudio engine. Playback is fire-and-forget
// and non-blocking, so it never stalls the Qt event loop. Decodes wav/mp3/flac.
class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    // Play a sound file by absolute path. Missing/invalid files are a no-op.
    void play(const QString& absolutePath);

    // Master volume 0.0 - 1.0.
    void setVolume(float v);

    bool ok() const;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};
