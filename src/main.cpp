#include <QApplication>
#include <QLockFile>
#include <QDir>

#include "Config.h"
#include "AudioPlayer.h"
#include "SoundGenerator.h"
#include "PomodoroEngine.h"
#include "PomodoroWidget.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("pomodoro");
    QApplication::setApplicationName("pomodoro");
    QApplication::setQuitOnLastWindowClosed(true);

    // Single-instance guard: if one is already running, exit quietly so
    // re-running `pomodoro` doesn't stack widgets. QLockFile handles stale
    // locks from a crashed process automatically.
    QLockFile lock(QDir::tempPath() + "/pomodoro-single-instance.lock");
    lock.setStaleLockTime(0);
    if (!lock.tryLock(100)) {
        return 0;  // an instance is already up
    }

    // First-run: synthesize the built-in soothing sounds.
    SoundGenerator::ensureBuiltins();

    Config config;
    AudioPlayer audio;
    audio.setVolume(config.volume());

    PomodoroEngine engine(&config);
    PomodoroWidget widget(&config, &engine, &audio);
    widget.show();

    return app.exec();
}
