#include "Config.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

namespace {
QSettings settings() {
    // Organization/application are set in main() so paths are stable.
    return QSettings();
}
}

Config::Config() = default;

int Config::workMinutes() const { return settings().value("durations/work", 25).toInt(); }
int Config::shortBreakMinutes() const { return settings().value("durations/shortBreak", 5).toInt(); }
int Config::longBreakMinutes() const { return settings().value("durations/longBreak", 15).toInt(); }
int Config::pomodorosPerLongBreak() const { return settings().value("durations/perLongBreak", 4).toInt(); }

void Config::setWorkMinutes(int m) { auto s = settings(); s.setValue("durations/work", m); }
void Config::setShortBreakMinutes(int m) { auto s = settings(); s.setValue("durations/shortBreak", m); }
void Config::setLongBreakMinutes(int m) { auto s = settings(); s.setValue("durations/longBreak", m); }
void Config::setPomodorosPerLongBreak(int n) { auto s = settings(); s.setValue("durations/perLongBreak", n); }

QString Config::endSound() const { return settings().value("sounds/end", "chime.wav").toString(); }
QString Config::resumeSound() const { return settings().value("sounds/resume", "arpeggio.wav").toString(); }
void Config::setEndSound(const QString& name) { auto s = settings(); s.setValue("sounds/end", name); }
void Config::setResumeSound(const QString& name) { auto s = settings(); s.setValue("sounds/resume", name); }

float Config::volume() const { return settings().value("audio/volume", 0.8).toFloat(); }
void Config::setVolume(float v) { auto s = settings(); s.setValue("audio/volume", v); }

bool Config::autoStartNextPhase() const { return settings().value("flow/autoStart", true).toBool(); }
void Config::setAutoStartNextPhase(bool on) { auto s = settings(); s.setValue("flow/autoStart", on); }

QByteArray Config::widgetGeometry() const { return settings().value("widget/geometry").toByteArray(); }
void Config::setWidgetGeometry(const QByteArray& geo) { auto s = settings(); s.setValue("widget/geometry", geo); }

QString Config::soundsDir() {
    // Keep a single, clean "pomodoro" folder (not the org/app nesting QSettings
    // uses) so the path shown to the user matches ~/.config/pomodoro/sounds.
    QString base = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (base.isEmpty()) {
        base = QDir::homePath() + "/.config";
    }
    QString dir = base + "/pomodoro/sounds";
    QDir().mkpath(dir);
    return dir;
}

QString Config::resolveSound(const QString& name) {
    if (name.isEmpty()) return QString();
    // Absolute paths are honored directly (lets a user point anywhere).
    if (QDir::isAbsolutePath(name)) return name;
    return soundsDir() + "/" + name;
}
