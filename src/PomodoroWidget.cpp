#include "PomodoroWidget.h"
#include "Config.h"
#include "AudioPlayer.h"
#include "SettingsDialog.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QCloseEvent>

namespace {
constexpr int kWidth = 210;
constexpr int kHeight = 84;

QColor accentFor(PomodoroEngine::Phase p) {
    switch (p) {
        case PomodoroEngine::Phase::Work:       return QColor(0xE8, 0x6A, 0x4B); // warm
        case PomodoroEngine::Phase::ShortBreak: return QColor(0x4B, 0xA3, 0xC7); // cool
        case PomodoroEngine::Phase::LongBreak:  return QColor(0x5B, 0xC0, 0x8A); // green
    }
    return QColor(0x88, 0x88, 0x88);
}
}

PomodoroWidget::PomodoroWidget(Config* config, PomodoroEngine* engine,
                               AudioPlayer* audio, QWidget* parent)
    : QWidget(parent), m_config(config), m_engine(engine), m_audio(audio) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(kWidth, kHeight);
    setWindowTitle(QStringLiteral("Pomodoro"));

    m_accent = accentFor(m_engine->phase());

    m_phaseLabel = new QLabel(this);
    m_phaseLabel->setStyleSheet("color: rgba(255,255,255,190); font-weight:600;"
                                " font-size:11px; letter-spacing:1px;");
    m_timeLabel = new QLabel(this);
    m_timeLabel->setStyleSheet("color: white; font-size:30px; font-weight:700;");

    auto topRow = new QHBoxLayout;
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->addWidget(m_phaseLabel);
    topRow->addStretch();
    topRow->addWidget(m_timeLabel);

    auto makeBtn = [this](const QString& glyph, const QString& tip) {
        auto b = new QPushButton(glyph, this);
        b->setToolTip(tip);
        b->setCursor(Qt::PointingHandCursor);
        b->setFixedHeight(22);
        b->setStyleSheet(
            "QPushButton { color: white; background: rgba(255,255,255,28);"
            " border: none; border-radius: 6px; font-size: 13px; padding: 0 6px; }"
            "QPushButton:hover { background: rgba(255,255,255,60); }"
            "QPushButton:pressed { background: rgba(0,0,0,50); }");
        return b;
    };

    m_startBtn    = makeBtn(QStringLiteral("▶"), tr("Start / Pause"));   // ▶
    m_skipBtn     = makeBtn(QStringLiteral("⏭"), tr("Skip to next"));    // ⏭
    m_resetBtn    = makeBtn(QStringLiteral("↺"), tr("Reset"));           // ↺
    m_settingsBtn = makeBtn(QStringLiteral("⚙"), tr("Settings"));        // ⚙

    auto btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->setSpacing(6);
    btnRow->addWidget(m_startBtn, 2);
    btnRow->addWidget(m_skipBtn, 1);
    btnRow->addWidget(m_resetBtn, 1);
    btnRow->addWidget(m_settingsBtn, 1);

    auto root = new QVBoxLayout(this);
    root->setContentsMargins(14, 10, 14, 12);
    root->setSpacing(6);
    root->addLayout(topRow);
    root->addLayout(btnRow);

    connect(m_startBtn, &QPushButton::clicked, m_engine, &PomodoroEngine::toggle);
    connect(m_skipBtn, &QPushButton::clicked, m_engine, &PomodoroEngine::skip);
    connect(m_resetBtn, &QPushButton::clicked, m_engine, &PomodoroEngine::reset);
    connect(m_settingsBtn, &QPushButton::clicked, this, &PomodoroWidget::openSettings);

    connect(m_engine, &PomodoroEngine::tick, this, &PomodoroWidget::onTick);
    connect(m_engine, &PomodoroEngine::phaseChanged, this, &PomodoroWidget::onPhaseChanged);
    connect(m_engine, &PomodoroEngine::runStateChanged, this, &PomodoroWidget::onRunStateChanged);
    connect(m_engine, &PomodoroEngine::phaseEnded, this, &PomodoroWidget::onPhaseEnded);
    connect(m_engine, &PomodoroEngine::phaseStarted, this, &PomodoroWidget::onPhaseStarted);

    applyPhaseStyle(m_engine->phase());
    updateTimeLabel(m_engine->remainingSecs());

    const QByteArray geo = m_config->widgetGeometry();
    if (!geo.isEmpty()) restoreGeometry(geo);
}

void PomodoroWidget::updateTimeLabel(int remainingSecs) {
    const int m = remainingSecs / 60;
    const int s = remainingSecs % 60;
    m_timeLabel->setText(QString::asprintf("%d:%02d", m, s));

    const int total = m_engine->phaseDurationSecs();
    m_progress = total > 0 ? 1.0 - (double)remainingSecs / total : 0.0;
    if (m_progress < 0.0) m_progress = 0.0;
    if (m_progress > 1.0) m_progress = 1.0;
    update();
}

void PomodoroWidget::applyPhaseStyle(PomodoroEngine::Phase p) {
    m_accent = accentFor(p);
    m_phaseLabel->setText(PomodoroEngine::phaseLabel(p));
    update();
}

void PomodoroWidget::onTick(int remainingSecs) { updateTimeLabel(remainingSecs); }

void PomodoroWidget::onPhaseChanged(PomodoroEngine::Phase p) {
    applyPhaseStyle(p);
    updateTimeLabel(m_engine->remainingSecs());
}

void PomodoroWidget::onRunStateChanged(bool running) {
    m_startBtn->setText(running ? QStringLiteral("⏸")   // ⏸
                                : QStringLiteral("▶")); // ▶
}

void PomodoroWidget::onPhaseEnded(PomodoroEngine::Phase) {
    playSound(m_config->endSound());
}

void PomodoroWidget::onPhaseStarted(PomodoroEngine::Phase) {
    playSound(m_config->resumeSound());
}

void PomodoroWidget::playSound(const QString& soundName) {
    if (!m_audio) return;
    m_audio->play(Config::resolveSound(soundName));
}

void PomodoroWidget::openSettings() {
    SettingsDialog dlg(m_config, m_audio, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_audio->setVolume(m_config->volume());
        m_engine->reloadDurations();
        updateTimeLabel(m_engine->remainingSecs());
    }
}

void PomodoroWidget::paintEvent(QPaintEvent*) {
    QPainter pr(this);
    pr.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r = rect().adjusted(1, 1, -1, -1);
    const double radius = 16.0;

    QPainterPath card;
    card.addRoundedRect(r, radius, radius);

    // Dark translucent card, subtly tinted by the phase accent.
    QColor base(28, 30, 36, 235);
    pr.fillPath(card, base);

    // Accent strip along the left edge.
    QPainterPath strip;
    strip.addRoundedRect(QRectF(r.left(), r.top(), 6, r.height()), 3, 3);
    pr.fillPath(strip, m_accent);

    // Progress bar along the bottom.
    const double pad = 14.0;
    const double barY = r.bottom() - 4.0;
    const double barW = r.width() - 2 * pad;
    QPen bg(QColor(255, 255, 255, 40)); bg.setWidthF(2.0); bg.setCapStyle(Qt::RoundCap);
    pr.setPen(bg);
    pr.drawLine(QPointF(r.left() + pad, barY), QPointF(r.left() + pad + barW, barY));
    QPen fg(m_accent); fg.setWidthF(2.0); fg.setCapStyle(Qt::RoundCap);
    pr.setPen(fg);
    pr.drawLine(QPointF(r.left() + pad, barY),
                QPointF(r.left() + pad + barW * m_progress, barY));
}

void PomodoroWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragOffset = e->globalPosition().toPoint() - frameGeometry().topLeft();
        e->accept();
    }
}

void PomodoroWidget::mouseMoveEvent(QMouseEvent* e) {
    if (m_dragging && (e->buttons() & Qt::LeftButton)) {
        move(e->globalPosition().toPoint() - m_dragOffset);
        e->accept();
    }
}

void PomodoroWidget::mouseReleaseEvent(QMouseEvent* e) {
    if (m_dragging) {
        m_dragging = false;
        m_config->setWidgetGeometry(saveGeometry());
        e->accept();
    }
}

void PomodoroWidget::closeEvent(QCloseEvent* e) {
    m_config->setWidgetGeometry(saveGeometry());
    e->accept();
}
