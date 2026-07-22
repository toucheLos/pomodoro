#include "AudioPlayer.h"

// miniaudio's implementation is compiled in exactly this one translation unit.
#define MINIAUDIO_IMPLEMENTATION
// We only need playback/decoding; trim capture and niche backends for build speed.
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#include "miniaudio.h"

#include <QFileInfo>
#include <QFile>
#include <QDebug>

struct AudioPlayer::Impl {
    ma_engine engine{};
    bool initialized = false;
};

AudioPlayer::AudioPlayer() : d(std::make_unique<Impl>()) {
    ma_result r = ma_engine_init(nullptr, &d->engine);
    if (r != MA_SUCCESS) {
        qWarning() << "AudioPlayer: failed to init audio engine, code" << r
                   << "-- sounds disabled";
        d->initialized = false;
    } else {
        d->initialized = true;
    }
}

AudioPlayer::~AudioPlayer() {
    if (d->initialized) {
        ma_engine_uninit(&d->engine);
    }
}

void AudioPlayer::play(const QString& absolutePath) {
    if (!d->initialized) return;
    if (absolutePath.isEmpty()) return;
    if (!QFileInfo::exists(absolutePath)) {
        qWarning() << "AudioPlayer: sound not found:" << absolutePath;
        return;
    }
    const QByteArray path = QFile::encodeName(absolutePath);
    ma_result r = ma_engine_play_sound(&d->engine, path.constData(), nullptr);
    if (r != MA_SUCCESS) {
        qWarning() << "AudioPlayer: could not play" << absolutePath << "code" << r;
    }
}

void AudioPlayer::setVolume(float v) {
    if (!d->initialized) return;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    ma_engine_set_volume(&d->engine, v);
}

bool AudioPlayer::ok() const { return d->initialized; }
