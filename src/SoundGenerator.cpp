#include "SoundGenerator.h"
#include "Config.h"

#include <QFile>
#include <QDir>
#include <QtGlobal>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace {

constexpr int kSampleRate = 44100;
constexpr double kPi = 3.14159265358979323846;

// A mono float buffer we accumulate into, then write as 16-bit PCM.
struct Buffer {
    std::vector<float> s;
    void ensure(int n) { if ((int)s.size() < n) s.resize(n, 0.0f); }
    void add(int i, float v) { if (i >= 0 && i < (int)s.size()) s[i] += v; }
};

int secToSamples(double sec) { return (int)std::lround(sec * kSampleRate); }

// Add a decaying sine partial starting at sample offset `start`.
void addPartial(Buffer& b, double startSec, double durSec, double freq,
                double amp, double decay, double attackSec = 0.004) {
    const int start = secToSamples(startSec);
    const int n = secToSamples(durSec);
    b.ensure(start + n);
    const int attack = std::max(1, secToSamples(attackSec));
    for (int i = 0; i < n; ++i) {
        const double t = (double)i / kSampleRate;
        double env = std::exp(-decay * t);
        if (i < attack) env *= (double)i / attack;   // short attack ramp
        const double v = amp * env * std::sin(2.0 * kPi * freq * t);
        b.add(start + i, (float)v);
    }
}

// Add a linear frequency chirp (used for the bird-ish sound).
void addChirp(Buffer& b, double startSec, double durSec, double f0, double f1,
              double amp) {
    const int start = secToSamples(startSec);
    const int n = secToSamples(durSec);
    b.ensure(start + n);
    double phase = 0.0;
    for (int i = 0; i < n; ++i) {
        const double frac = (double)i / n;
        const double f = f0 + (f1 - f0) * frac;
        phase += 2.0 * kPi * f / kSampleRate;
        // Bell-shaped envelope so each chirp fades in and out.
        const double env = std::sin(kPi * frac);
        b.add(start + i, (float)(amp * env * std::sin(phase)));
    }
}

bool writeWav(const QString& path, const Buffer& buf) {
    // Peak-normalize to avoid clipping while keeping headroom.
    float peak = 0.0f;
    for (float v : buf.s) peak = std::max(peak, std::fabs(v));
    const float gain = peak > 0.0f ? (0.89f / peak) : 1.0f;

    const int n = (int)buf.s.size();
    std::vector<int16_t> pcm(n);
    for (int i = 0; i < n; ++i) {
        float v = buf.s[i] * gain;
        v = std::max(-1.0f, std::min(1.0f, v));
        pcm[i] = (int16_t)std::lround(v * 32767.0f);
    }

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;

    const uint32_t dataBytes = (uint32_t)(n * sizeof(int16_t));
    const uint32_t byteRate = kSampleRate * 1 * sizeof(int16_t);
    auto u32 = [&](uint32_t v) { f.write(reinterpret_cast<const char*>(&v), 4); };
    auto u16 = [&](uint16_t v) { f.write(reinterpret_cast<const char*>(&v), 2); };

    f.write("RIFF", 4);
    u32(36 + dataBytes);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    u32(16);                 // PCM fmt chunk size
    u16(1);                  // audio format = PCM
    u16(1);                  // channels = mono
    u32(kSampleRate);
    u32(byteRate);
    u16((uint16_t)(1 * sizeof(int16_t)));  // block align
    u16(16);                 // bits per sample
    f.write("data", 4);
    u32(dataBytes);
    f.write(reinterpret_cast<const char*>(pcm.data()), dataBytes);
    f.close();
    return true;
}

// ---- The five built-in voices -------------------------------------------

Buffer makeChime() {
    // Bell-like: a fundamental plus inharmonic partials, exponential decay.
    Buffer b;
    const double base = 880.0;
    const double ratios[] = {1.0, 2.01, 2.76, 3.98, 5.42};
    const double amps[]   = {1.0, 0.55, 0.38, 0.22, 0.13};
    const double decays[] = {2.6, 3.2, 4.0, 5.0, 6.5};
    for (int k = 0; k < 5; ++k)
        addPartial(b, 0.0, 2.4, base * ratios[k], amps[k], decays[k]);
    return b;
}

Buffer makeSoftGong() {
    // Low fundamental, slow attack, long lingering decay.
    Buffer b;
    const double base = 196.0;
    const double ratios[] = {1.0, 1.48, 2.03, 2.67};
    const double amps[]   = {1.0, 0.5, 0.32, 0.18};
    for (int k = 0; k < 4; ++k)
        addPartial(b, 0.0, 3.8, base * ratios[k], amps[k], 1.1 + 0.5 * k, 0.06);
    return b;
}

Buffer makeArpeggio() {
    // Gentle major triad rolled upward, then the octave.
    Buffer b;
    const double notes[] = {523.25, 659.25, 783.99, 1046.50}; // C5 E5 G5 C6
    for (int k = 0; k < 4; ++k) {
        const double start = k * 0.14;
        addPartial(b, start, 0.9, notes[k], 0.9, 4.5);
        addPartial(b, start, 0.9, notes[k] * 2.0, 0.25, 6.0); // soft overtone
    }
    return b;
}

Buffer makeBirds() {
    // A few short upward chirps at slightly varied pitches.
    Buffer b;
    const double starts[] = {0.00, 0.18, 0.33, 0.55, 0.70};
    const double f0[]     = {1800, 2100, 1600, 2400, 1900};
    const double f1[]     = {3200, 3400, 2900, 3800, 3300};
    for (int k = 0; k < 5; ++k)
        addChirp(b, starts[k], 0.12, f0[k], f1[k], 0.8);
    return b;
}

Buffer makeDing() {
    // Clean single sine ping with a touch of shimmer.
    Buffer b;
    addPartial(b, 0.0, 1.2, 1046.50, 1.0, 5.0);
    addPartial(b, 0.0, 1.2, 2093.00, 0.2, 7.0);
    return b;
}

struct Voice { const char* name; Buffer (*make)(); };
const Voice kVoices[] = {
    {"chime.wav",     makeChime},
    {"soft-gong.wav", makeSoftGong},
    {"arpeggio.wav",  makeArpeggio},
    {"birds.wav",     makeBirds},
    {"ding.wav",      makeDing},
};

} // namespace

QStringList SoundGenerator::builtinNames() {
    QStringList names;
    for (const auto& v : kVoices) names << QString::fromLatin1(v.name);
    return names;
}

void SoundGenerator::ensureBuiltins() {
    const QString dir = Config::soundsDir();
    for (const auto& v : kVoices) {
        const QString path = dir + "/" + QString::fromLatin1(v.name);
        if (QFile::exists(path)) continue;
        Buffer buf = v.make();
        writeWav(path, buf);
    }
}
