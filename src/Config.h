#pragma once

#include <QString>
#include <QByteArray>

// Cross-platform persistent configuration backed by QSettings.
// On Linux this lives under ~/.config/pomodoro/, on Windows the registry,
// on macOS a plist -- all handled by QSettings automatically.
class Config {
public:
    Config();

    // Durations (minutes).
    int workMinutes() const;
    int shortBreakMinutes() const;
    int longBreakMinutes() const;
    int pomodorosPerLongBreak() const;

    void setWorkMinutes(int m);
    void setShortBreakMinutes(int m);
    void setLongBreakMinutes(int m);
    void setPomodorosPerLongBreak(int n);

    // Sounds -- stored as bare file names resolved against the sounds folder.
    QString endSound() const;     // played when a phase reaches 0:00
    QString resumeSound() const;  // played when the next phase starts
    void setEndSound(const QString& name);
    void setResumeSound(const QString& name);

    // Master volume 0.0 - 1.0.
    float volume() const;
    void setVolume(float v);

    bool autoStartNextPhase() const;
    void setAutoStartNextPhase(bool on);

    // Widget placement.
    QByteArray widgetGeometry() const;
    void setWidgetGeometry(const QByteArray& geo);

    // Absolute path to the user sounds directory (created if missing).
    static QString soundsDir();
    // Resolve a stored sound name to an absolute path (may be empty if unset).
    static QString resolveSound(const QString& name);
};
