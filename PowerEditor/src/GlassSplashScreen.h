/**
 * GlassSplashScreen.h — Liquid Glass Startup Screen
 */

#ifndef GLASS_SPLASH_SCREEN_H
#define GLASS_SPLASH_SCREEN_H

#include "LiquidGlass.h"
#include <QSplashScreen>
#include <QPainter>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLabel>

class GlassSplashScreen : public QSplashScreen {
public:
    explicit GlassSplashScreen(const QPixmap& pix = QPixmap()) : QSplashScreen(pix) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(600, 340);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(40, 40, 40, 40);
        layout->setSpacing(20);

        auto* title = new QLabel("NOTEPAD++", this);
        title->setStyleSheet("color: white; font-size: 42px; font-weight: 900; letter-spacing: 5px; background: transparent;");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        auto* subtitle = new QLabel("BOBUI LIQUID GLASS EDITION", this);
        subtitle->setStyleSheet("color: rgba(90, 130, 255, 200); font-size: 14px; font-weight: bold; letter-spacing: 2px; background: transparent;");
        subtitle->setAlignment(Qt::AlignCenter);
        layout->addWidget(subtitle);

        layout->addStretch();

        m_statusLabel = new QLabel("Initializing engines...", this);
        m_statusLabel->setStyleSheet("color: rgba(235, 240, 255, 180); font-size: 12px; background: transparent;");
        m_statusLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_statusLabel);

        m_progress = new QProgressBar(this);
        m_progress->setRange(0, 100);
        m_progress->setValue(0);
        m_progress->setTextVisible(false);
        m_progress->setFixedHeight(4);
        m_progress->setStyleSheet(
            "QProgressBar { background: rgba(255, 255, 255, 15); border: none; border-radius: 2px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4a9eff, stop:1 #88AAFF); border-radius: 2px; }"
        );
        layout->addWidget(m_progress);
    }

    void setProgress(int val) { m_progress->setValue(val); }
    void setMessage(const QString& msg) { m_statusLabel->setText(msg); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        // Draw the massive liquid glass backdrop
        LiquidGlassPainter::drawGlassBackground(p, QRectF(rect()), 24.0, true);
        
        // Add a soft global glow for the splash
        QRadialGradient glow(rect().center(), 400);
        glow.setColorAt(0, QColor(80, 140, 255, 30));
        glow.setColorAt(1, Qt::transparent);
        p.fillRect(rect(), glow);
    }

private:
    QProgressBar* m_progress;
    QLabel*       m_statusLabel;
};

#endif // GLASS_SPLASH_SCREEN_H
