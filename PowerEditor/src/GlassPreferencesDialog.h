/**
 * GlassPreferencesDialog.h — Liquid Glass UI Preferences Dialog
 */

#ifndef GLASS_PREFERENCES_DIALOG_H
#define GLASS_PREFERENCES_DIALOG_H

#include "LiquidGlass.h"
#include "GlassSettings.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QMouseEvent>

class GlassPreferencesDialog : public QDialog {
public:
    explicit GlassPreferencesDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Preferences");
        setModal(true);
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setFixedSize(500, 360);
        setStyleSheet(LiquidGlassStyleSheet::kDialog);

        auto* mainLay = new QVBoxLayout(this);
        mainLay->setContentsMargins(20, 20, 20, 20);
        mainLay->setSpacing(15);

        // Title bar
        auto* titleBar = new QWidget(this);
        auto* titleLay = new QHBoxLayout(titleBar);
        titleLay->setContentsMargins(0,0,0,0);
        auto* titleLabel = new QLabel("⚙ Preferences", titleBar);
        titleLabel->setStyleSheet("color: rgba(240,245,255,240); font-size:16px; font-weight:bold;");
        auto* closeBtn = new QPushButton("✕", titleBar);
        closeBtn->setFixedSize(24, 24);
        closeBtn->setStyleSheet("QPushButton{background:rgba(255,60,60,160);color:white;border:none;border-radius:12px;} QPushButton:hover{background:rgba(255,80,80,220);}");
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
        titleLay->addWidget(titleLabel);
        titleLay->addStretch();
        titleLay->addWidget(closeBtn);
        mainLay->addWidget(titleBar);

        // Form layout for settings
        auto* formLay = new QFormLayout;
        formLay->setVerticalSpacing(12);
        formLay->setHorizontalSpacing(20);

        // 1. Theme / Glass Intensity
        m_glassCombo = new QComboBox(this);
        m_glassCombo->addItems({"Low (Mica)", "Medium (Mica Tabbed)", "High (Acrylic)"});
        m_glassCombo->setCurrentIndex(GlassSettings::instance().glassIntensity());
        formLay->addRow("Glass Intensity:", m_glassCombo);

        // 2. Bubble Animations
        m_bubbleCheck = new QCheckBox("Enable liquid bubble animations", this);
        m_bubbleCheck->setChecked(GlassSettings::instance().bubbleAnimations());
        formLay->addRow("Visuals:", m_bubbleCheck);

        // 3. Default Font
        m_fontCombo = new QFontComboBox(this);
        m_fontCombo->setFontFilters(QFontComboBox::MonospacedFonts);
        m_fontCombo->setCurrentFont(GlassSettings::instance().editorFont());
        
        m_fontSizeSpin = new QSpinBox(this);
        m_fontSizeSpin->setRange(6, 72);
        m_fontSizeSpin->setValue(GlassSettings::instance().editorFont().pointSize());
        
        auto* fontLay = new QHBoxLayout;
        fontLay->addWidget(m_fontCombo);
        fontLay->addWidget(new QLabel("Size:"));
        fontLay->addWidget(m_fontSizeSpin);
        formLay->addRow("Editor Font:", fontLay);

        // 4. Word Wrap
        m_wrapCheck = new QCheckBox("Enable Word Wrap by default", this);
        m_wrapCheck->setChecked(GlassSettings::instance().wordWrapDefault());
        formLay->addRow("Word Wrap:", m_wrapCheck);

        // 5. Default Encoding
        m_encCombo = new QComboBox(this);
        m_encCombo->addItems({"UTF-8", "UTF-8 BOM", "UTF-16 BE BOM", "UTF-16 LE BOM", "ANSI"});
        m_encCombo->setCurrentText(GlassSettings::instance().defaultEncoding());
        formLay->addRow("Default Encoding:", m_encCombo);

        mainLay->addLayout(formLay);
        mainLay->addStretch();

        // Bottom buttons
        auto* btnLay = new QHBoxLayout;
        btnLay->addStretch();
        auto* cancelBtn = new QPushButton("Cancel", this);
        auto* applyBtn = new QPushButton("Apply", this);
        auto* okBtn = new QPushButton("OK", this);
        okBtn->setDefault(true);
        
        btnLay->addWidget(cancelBtn);
        btnLay->addWidget(applyBtn);
        btnLay->addWidget(okBtn);
        mainLay->addLayout(btnLay);

        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        connect(applyBtn, &QPushButton::clicked, this, &GlassPreferencesDialog::applySettings);
        connect(okBtn, &QPushButton::clicked, this, [this](){
            applySettings();
            accept();
        });

        installEventFilter(this);
    }

    void setApplyCallback(std::function<void()> cb) { m_applyCb = cb; }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        LiquidGlassPainter::drawGlassBackground(p, QRectF(rect()), 14.0, true);
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            m_dragPos = e->globalPosition().toPoint() - frameGeometry().topLeft();
            e->accept();
        }
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) {
            move(e->globalPosition().toPoint() - m_dragPos);
            e->accept();
        }
    }

private:
    void applySettings() {
        auto& s = GlassSettings::instance();
        s.setGlassIntensity(m_glassCombo->currentIndex());
        s.setBubbleAnimations(m_bubbleCheck->isChecked());
        
        QFont f = m_fontCombo->currentFont();
        f.setPointSize(m_fontSizeSpin->value());
        s.setEditorFont(f);
        
        s.setWordWrapDefault(m_wrapCheck->isChecked());
        s.setDefaultEncoding(m_encCombo->currentText());
        
        if (m_applyCb) m_applyCb();
    }

    std::function<void()> m_applyCb;
    QComboBox* m_glassCombo;
    QCheckBox* m_bubbleCheck;
    QFontComboBox* m_fontCombo;
    QSpinBox* m_fontSizeSpin;
    QCheckBox* m_wrapCheck;
    QComboBox* m_encCombo;
    QPoint m_dragPos;
};

#endif // GLASS_PREFERENCES_DIALOG_H
