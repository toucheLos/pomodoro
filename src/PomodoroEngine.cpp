#include "PomodoroEngine.h"
#include "Config.h"

#include <algorithm>

PomodoroEngine::PomodoroEngine(Config* config, QObject* parent)
    : QObject(parent), m_config(config) {
    m_timer.setInterval(200);  // sub-second so the display never lags a tick
    connect(&m_timer, &QTimer::timeout, this, &PomodoroEngine::onTick);
    m_clock.start();
    beginPhase(Phase::Work, /*autoStart=*/false);
}

QString PomodoroEngine::phaseLabel(Phase p) {
    switch (p) {
        case Phase::Work: return QStringLiteral("WORK");
        case Phase::ShortBreak: return QStringLiteral("BREAK");
        case Phase::LongBreak: return QStringLiteral("LONG BREAK");
    }
    return QString();
}

int PomodoroEngine::durationSecsFor(Phase p) const {
    switch (p) {
        case Phase::Work: return std::max(1, m_config->workMinutes()) * 60;
        case Phase::ShortBreak: return std::max(1, m_config->shortBreakMinutes()) * 60;
        case Phase::LongBreak: return std::max(1, m_config->longBreakMinutes()) * 60;
    }
    return 60;
}

int PomodoroEngine::phaseDurationSecs() const { return durationSecsFor(m_phase); }

int PomodoroEngine::remainingSecs() const {
    qint64 ms = m_remainingMs;
    if (m_running) {
        ms = m_deadlineMs - m_clock.elapsed();
    }
    if (ms < 0) ms = 0;
    return (int)((ms + 999) / 1000);  // round up so it shows 0:00 only at the end
}

void PomodoroEngine::beginPhase(Phase p, bool autoStart) {
    m_phase = p;
    m_remainingMs = (qint64)durationSecsFor(p) * 1000;
    m_running = false;
    m_timer.stop();
    emit phaseChanged(p);
    emit tick(remainingSecs());
    emit runStateChanged(false);
    if (autoStart) start();
}

void PomodoroEngine::start() {
    if (m_running) return;
    if (m_remainingMs <= 0) m_remainingMs = (qint64)durationSecsFor(m_phase) * 1000;
    m_deadlineMs = m_clock.elapsed() + m_remainingMs;
    m_running = true;
    m_timer.start();
    emit runStateChanged(true);
    emit tick(remainingSecs());
}

void PomodoroEngine::pause() {
    if (!m_running) return;
    m_remainingMs = m_deadlineMs - m_clock.elapsed();
    if (m_remainingMs < 0) m_remainingMs = 0;
    m_running = false;
    m_timer.stop();
    emit runStateChanged(false);
    emit tick(remainingSecs());
}

void PomodoroEngine::toggle() { m_running ? pause() : start(); }

void PomodoroEngine::skip() {
    // Treat as if the phase completed, so sounds/counters behave consistently.
    advancePhase(/*userInitiated=*/true);
}

void PomodoroEngine::reset() {
    m_completedPomodoros = 0;
    beginPhase(Phase::Work, /*autoStart=*/false);
}

void PomodoroEngine::reloadDurations() {
    // If idle at the top of a phase, refresh its remaining time to the new value.
    if (!m_running) {
        m_remainingMs = (qint64)durationSecsFor(m_phase) * 1000;
        emit tick(remainingSecs());
    }
}

void PomodoroEngine::advancePhase(bool userInitiated) {
    const Phase ended = m_phase;
    emit phaseEnded(ended);

    Phase next;
    if (ended == Phase::Work) {
        ++m_completedPomodoros;
        const int per = std::max(1, m_config->pomodorosPerLongBreak());
        next = (m_completedPomodoros % per == 0) ? Phase::LongBreak : Phase::ShortBreak;
    } else {
        next = Phase::Work;  // any break returns to work
    }

    const bool autoStart = m_config->autoStartNextPhase();
    beginPhase(next, autoStart);
    if (autoStart) {
        // The new phase is now counting down: signal the resume alert.
        emit phaseStarted(next);
    }
    Q_UNUSED(userInitiated);
}

void PomodoroEngine::onTick() {
    if (!m_running) return;
    const qint64 left = m_deadlineMs - m_clock.elapsed();
    if (left <= 0) {
        advancePhase(/*userInitiated=*/false);
        return;
    }
    emit tick(remainingSecs());
}
