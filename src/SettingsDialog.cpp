#include "SettingsDialog.h"
#include "Config.h"
#include "AudioPlayer.h"
#include "SoundGenerator.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QSet>

SettingsDialog::SettingsDialog(Config* config, AudioPlayer* audio, QWidget* parent)
    : QDialog(parent), m_config(config), m_audio(audio) {
    setWindowTitle(tr("Pomodoro Settings"));
    setModal(true);

    auto form = new QFormLayout;

    auto mkSpin = [this](int val, int lo, int hi) {
        auto s = new QSpinBox(this);
        s->setRange(lo, hi);
        s->setValue(val);
        s->setSuffix(tr(" min"));
        return s;
    };
    m_work  = mkSpin(m_config->workMinutes(), 1, 180);
    m_short = mkSpin(m_config->shortBreakMinutes(), 1, 120);
    m_long  = mkSpin(m_config->longBreakMinutes(), 1, 180);

    m_perLong = new QSpinBox(this);
    m_perLong->setRange(1, 12);
    m_perLong->setValue(m_config->pomodorosPerLongBreak());

    form->addRow(tr("Work"), m_work);
    form->addRow(tr("Short break"), m_short);
    form->addRow(tr("Long break"), m_long);
    form->addRow(tr("Pomodoros / long break"), m_perLong);

    // Sound rows: combo + Preview button.
    auto soundRow = [this](QComboBox* combo) {
        auto row = new QHBoxLayout;
        row->addWidget(combo, 1);
        auto play = new QPushButton(tr("Preview"), this);
        connect(play, &QPushButton::clicked, this, [this, combo]{ previewSound(combo); });
        row->addWidget(play);
        return row;
    };

    m_endSound = buildSoundCombo(m_config->endSound());
    m_resumeSound = buildSoundCombo(m_config->resumeSound());
    form->addRow(tr("End sound"), soundRow(m_endSound));
    form->addRow(tr("Resume sound"), soundRow(m_resumeSound));

    m_volume = new QSlider(Qt::Horizontal, this);
    m_volume->setRange(0, 100);
    m_volume->setValue((int)(m_config->volume() * 100));
    form->addRow(tr("Volume"), m_volume);

    m_autoStart = new QCheckBox(tr("Auto-start next phase"), this);
    m_autoStart->setChecked(m_config->autoStartNextPhase());

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

    auto hint = new QLabel(
        tr("Drop your own .wav/.mp3/.flac files into:\n%1").arg(Config::soundsDir()), this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color: gray; font-size: 10px;");

    auto root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(m_autoStart);
    root->addWidget(hint);
    root->addWidget(buttons);
}

QComboBox* SettingsDialog::buildSoundCombo(const QString& current) {
    auto combo = new QComboBox(this);

    // Built-ins first, then any extra user files, de-duplicated.
    QStringList items = SoundGenerator::builtinNames();
    QSet<QString> seen(items.begin(), items.end());

    QDir dir(Config::soundsDir());
    const QStringList filters{"*.wav", "*.mp3", "*.flac", "*.ogg"};
    for (const QFileInfo& fi : dir.entryInfoList(filters, QDir::Files, QDir::Name)) {
        if (!seen.contains(fi.fileName())) {
            items << fi.fileName();
            seen.insert(fi.fileName());
        }
    }

    combo->addItems(items);
    int idx = combo->findText(current);
    if (idx < 0 && !current.isEmpty()) { combo->addItem(current); idx = combo->count() - 1; }
    if (idx >= 0) combo->setCurrentIndex(idx);
    return combo;
}

void SettingsDialog::previewSound(QComboBox* combo) {
    if (!m_audio || !combo) return;
    // Apply the current slider volume so the preview matches what you'll hear.
    m_audio->setVolume(m_volume->value() / 100.0f);
    m_audio->play(Config::resolveSound(combo->currentText()));
}

void SettingsDialog::refreshSoundLists() { /* reserved for future live refresh */ }

void SettingsDialog::accept() {
    m_config->setWorkMinutes(m_work->value());
    m_config->setShortBreakMinutes(m_short->value());
    m_config->setLongBreakMinutes(m_long->value());
    m_config->setPomodorosPerLongBreak(m_perLong->value());
    m_config->setEndSound(m_endSound->currentText());
    m_config->setResumeSound(m_resumeSound->currentText());
    m_config->setVolume(m_volume->value() / 100.0f);
    m_config->setAutoStartNextPhase(m_autoStart->isChecked());
    QDialog::accept();
}
