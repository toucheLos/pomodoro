#pragma once

#include <QWidget>
#include <QPoint>
#include "PomodoroEngine.h"

class Config;
class AudioPlayer;
class QLabel;
class QPushButton;

// The floating, frameless, always-on-top mini widget.
class PomodoroWidget : public QWidget {
    Q_OBJECT
public:
    PomodoroWidget(Config* config, PomodoroEngine* engine, AudioPlayer* audio,
                   QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void closeEvent(QCloseEvent*) override;

private slots:
    void onTick(int remainingSecs);
    void onPhaseChanged(PomodoroEngine::Phase p);
    void onRunStateChanged(bool running);
    void onPhaseEnded(PomodoroEngine::Phase p);
    void onPhaseStarted(PomodoroEngine::Phase p);
    void openSettings();

private:
    void updateTimeLabel(int remainingSecs);
    void applyPhaseStyle(PomodoroEngine::Phase p);
    void playSound(const QString& soundName);

    Config* m_config;
    PomodoroEngine* m_engine;
    AudioPlayer* m_audio;

    QLabel* m_phaseLabel = nullptr;
    QLabel* m_timeLabel = nullptr;
    QPushButton* m_startBtn = nullptr;
    QPushButton* m_skipBtn = nullptr;
    QPushButton* m_resetBtn = nullptr;
    QPushButton* m_settingsBtn = nullptr;

    double m_progress = 0.0;      // 0..1 elapsed fraction of current phase
    QColor m_accent;
    QPoint m_dragOffset;
    bool m_dragging = false;
};
