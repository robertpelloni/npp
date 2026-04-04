/**
 * GlassPreferencesDialog.h — Comprehensive Liquid Glass Preferences
 */

#ifndef GLASS_PREFERENCES_DIALOG_H
#define GLASS_PREFERENCES_DIALOG_H

#include "LiquidGlass.h"
#include "GlassSettings.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
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
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(700, 500);
        setStyleSheet(LiquidGlassStyleSheet::kDialog);

        auto* mainLay = new QVBoxLayout(this);
        mainLay->setContentsMargins(20, 20, 20, 20);

        // Header
        auto* header = new QHBoxLayout;
        auto* title = new QLabel("⚙ Preferences", this);
        title->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
        auto* closeBtn = new QPushButton("✕", this);
        closeBtn->setFixedSize(24, 24);
        closeBtn->setStyleSheet("QPushButton{background:rgba(255,60,60,160);color:white;border:none;border-radius:12px;}");
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
        header->addWidget(title);
        header->addStretch();
        header->addWidget(closeBtn);
        mainLay->addLayout(header);

        auto* body = new QHBoxLayout;
        
        // Sidebar
        m_sidebar = new QListWidget(this);
        m_sidebar->setFixedWidth(150);
        m_sidebar->addItem("General");
        m_sidebar->addItem("Editing");
        m_sidebar->addItem("Backup");
        m_sidebar->addItem("Theme");
        m_sidebar->setStyleSheet(
            "QListWidget { background: rgba(255,255,255,10); border: none; border-radius: 8px; }"
            "QListWidget::item { padding: 10px; color: rgba(235, 240, 255, 180); }"
            "QListWidget::item:selected { background: rgba(90, 130, 255, 100); color: white; border-radius: 6px; }"
        );
        body->addWidget(m_sidebar);

        // Content
        m_stack = new QStackedWidget(this);
        m_stack->addWidget(createGeneralPage());
        m_stack->addWidget(createEditingPage());
        m_stack->addWidget(createBackupPage());
        m_stack->addWidget(createThemePage());
        body->addWidget(m_stack);

        mainLay->addLayout(body);

        auto* footer = new QHBoxLayout;
        auto* resetBtn = new QPushButton("Reset Defaults", this);
        connect(resetBtn, &QPushButton::clicked, this, [this](){
            resetToDefaults();
        });
        footer->addWidget(resetBtn);
        footer->addStretch();
        auto* saveBtn = new QPushButton("Apply & Close", this);
        saveBtn->setFixedWidth(120);
        connect(saveBtn, &QPushButton::clicked, this, [this](){
            applySettings();
            accept();
        });
        footer->addWidget(saveBtn);
        mainLay->addLayout(footer);

        connect(m_sidebar, &QListWidget::currentRowChanged, m_stack, &QStackedWidget::setCurrentIndex);
        m_sidebar->setCurrentRow(0);

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
        if (e->button() == Qt::LeftButton) m_dragPos = e->globalPosition().toPoint() - frameGeometry().topLeft();
        e->accept();
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) move(e->globalPosition().toPoint() - m_dragPos);
        e->accept();
    }

private:
    QWidget* createGeneralPage() {
        auto* w = new QWidget;
        auto* lay = new QFormLayout(w);
        m_showMinimap = new QCheckBox("Show Document Minimap", this);
        m_showMinimap->setChecked(GlassSettings::instance().showMinimap());
        m_bubbleCheck = new QCheckBox("Enable Liquid Bubbles", this);
        m_bubbleCheck->setChecked(GlassSettings::instance().bubbleAnimations());
        lay->addRow("Visuals:", m_showMinimap);
        lay->addRow("", m_bubbleCheck);
        return w;
    }

    QWidget* createEditingPage() {
        auto* w = new QWidget;
        auto* lay = new QFormLayout(w);
        m_fontCombo = new QFontComboBox(this);
        m_fontCombo->setCurrentFont(GlassSettings::instance().editorFont());
        m_fontSize = new QSpinBox(this);
        m_fontSize->setRange(6, 72);
        m_fontSize->setValue(GlassSettings::instance().editorFont().pointSize());
        m_wrapCheck = new QCheckBox("Word Wrap by Default", this);
        m_wrapCheck->setChecked(GlassSettings::instance().wordWrapDefault());
        lay->addRow("Font:", m_fontCombo);
        lay->addRow("Size:", m_fontSize);
        lay->addRow("Editor:", m_wrapCheck);
        return w;
    }

    QWidget* createBackupPage() {
        auto* w = new QWidget;
        auto* lay = new QFormLayout(w);
        m_backupCheck = new QCheckBox("Enable Auto-backup", this);
        m_backupCheck->setChecked(GlassSettings::instance().backupEnabled());
        m_backupInterval = new QSpinBox(this);
        m_backupInterval->setRange(5, 3600);
        m_backupInterval->setSuffix(" seconds");
        m_backupInterval->setValue(GlassSettings::instance().backupInterval() / 1000);
        lay->addRow("Backup:", m_backupCheck);
        lay->addRow("Interval:", m_backupInterval);
        return w;
    }

    QWidget* createThemePage() {
        auto* w = new QWidget;
        auto* lay = new QFormLayout(w);
        m_glassIntensity = new QComboBox(this);
        m_glassIntensity->addItems({"Low (Mica)", "Medium (Mica Tabbed)", "High (Acrylic)"});
        m_glassIntensity->setCurrentIndex(GlassSettings::instance().glassIntensity());
        lay->addRow("Glass Mode:", m_glassIntensity);
        return w;
    }

    void applySettings() {
        auto& s = GlassSettings::instance();
        s.setShowMinimap(m_showMinimap->isChecked());
        s.setBubbleAnimations(m_bubbleCheck->isChecked());
        
        QFont f = m_fontCombo->currentFont();
        f.setPointSize(m_fontSize->value());
        s.setEditorFont(f);
        
        s.setWordWrapDefault(m_wrapCheck->isChecked());
        s.setBackupEnabled(m_backupCheck->isChecked());
        s.setBackupInterval(m_backupInterval->value() * 1000);
        s.setGlassIntensity(m_glassIntensity->currentIndex());
        
        if (m_applyCb) m_applyCb();
    }

    void resetToDefaults() {
        m_showMinimap->setChecked(true);
        m_bubbleCheck->setChecked(true);
        QFont defaultFont("Cascadia Code", 11);
        defaultFont.setStyleHint(QFont::Monospace);
        m_fontCombo->setCurrentFont(defaultFont);
        m_fontSize->setValue(11);
        m_wrapCheck->setChecked(false);
        m_backupCheck->setChecked(true);
        m_backupInterval->setValue(30);
        m_glassIntensity->setCurrentIndex(2);
    }

    QListWidget* m_sidebar;
    QStackedWidget* m_stack;
    std::function<void()> m_applyCb;

    QCheckBox* m_showMinimap;
    QCheckBox* m_bubbleCheck;
    QFontComboBox* m_fontCombo;
    QSpinBox* m_fontSize;
    QCheckBox* m_wrapCheck;
    QCheckBox* m_backupCheck;
    QSpinBox* m_backupInterval;
    QComboBox* m_glassIntensity;
    QPoint m_dragPos;
};

#endif // GLASS_PREFERENCES_DIALOG_H
