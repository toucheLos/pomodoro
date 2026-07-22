#pragma once

#include <QDialog>

class Config;
class AudioPlayer;
class QSpinBox;
class QComboBox;
class QSlider;
class QCheckBox;

// Small settings dialog: durations, per-event sound pickers (with Preview),
// master volume, and auto-advance toggle. Writes back to Config on accept.
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(Config* config, AudioPlayer* audio, QWidget* parent = nullptr);

private slots:
    void accept() override;

private:
    QComboBox* buildSoundCombo(const QString& current);
    void previewSound(QComboBox* combo);
    void refreshSoundLists();

    Config* m_config;
    AudioPlayer* m_audio;

    QSpinBox* m_work = nullptr;
    QSpinBox* m_short = nullptr;
    QSpinBox* m_long = nullptr;
    QSpinBox* m_perLong = nullptr;
    QComboBox* m_endSound = nullptr;
    QComboBox* m_resumeSound = nullptr;
    QSlider* m_volume = nullptr;
    QCheckBox* m_autoStart = nullptr;
};
