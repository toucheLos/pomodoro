#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

class Config;

// Pomodoro state machine with a drift-free countdown. It computes remaining
// time from a monotonic deadline rather than accumulating tick errors.
class PomodoroEngine : public QObject {
    Q_OBJECT
public:
    enum class Phase { Work, ShortBreak, LongBreak };
    Q_ENUM(Phase)

    explicit PomodoroEngine(Config* config, QObject* parent = nullptr);

    Phase phase() const { return m_phase; }
    bool running() const { return m_running; }
    int remainingSecs() const;      // whole seconds remaining, rounded up
    int phaseDurationSecs() const;  // total length of the current phase
    int completedPomodoros() const { return m_completedPomodoros; }

    static QString phaseLabel(Phase p);

public slots:
    void start();        // begin/resume the countdown
    void pause();        // hold; remaining time is preserved
    void toggle();       // start if paused, pause if running
    void skip();         // end the current phase immediately (as if it hit 0)
    void reset();        // stop and return to a fresh Work phase
    void reloadDurations(); // re-read durations from Config (after settings edit)

signals:
    void tick(int remainingSecs);
    void phaseChanged(PomodoroEngine::Phase newPhase);
    void runStateChanged(bool running);
    void phaseEnded(PomodoroEngine::Phase endedPhase);      // play "end" sound
    void phaseStarted(PomodoroEngine::Phase startedPhase);  // play "resume" sound

private:
    void beginPhase(Phase p, bool autoStart);
    void advancePhase(bool userInitiated);
    void onTick();
    int durationSecsFor(Phase p) const;

    Config* m_config;
    QTimer m_timer;
    QElapsedTimer m_clock;

    Phase m_phase = Phase::Work;
    bool m_running = false;
    qint64 m_remainingMs = 0;   // authoritative when paused
    qint64 m_deadlineMs = 0;    // m_clock.elapsed() value at which phase ends
    int m_completedPomodoros = 0;
};
