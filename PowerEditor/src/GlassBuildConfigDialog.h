/**
 * GlassBuildConfigDialog.h — Geany-style Build Configuration
 */

#ifndef GLASS_BUILD_CONFIG_DIALOG_H
#define GLASS_BUILD_CONFIG_DIALOG_H

#include "LiquidGlass.h"
#include "GlassSettings.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class GlassBuildConfigDialog : public QDialog {
public:
    explicit GlassBuildConfigDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Build Commands");
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(500, 300);
        setStyleSheet(LiquidGlassStyleSheet::kDialog);

        auto* mainLay = new QVBoxLayout(this);
        mainLay->setContentsMargins(20, 20, 20, 20);

        auto* title = new QLabel("🛠 Build Commands", this);
        title->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
        mainLay->addWidget(title);

        auto* form = new QFormLayout;
        m_buildEdit = new QLineEdit(this);
        m_buildEdit->setText(GlassSettings::instance().buildCommand());
        m_execEdit = new QLineEdit(this);
        m_execEdit->setText(GlassSettings::instance().executeCommand());
        
        form->addRow("Build Command:", m_buildEdit);
        form->addRow("Execute Command:", m_execEdit);
        mainLay->addLayout(form);
        
        auto* hint = new QLabel("%f = full path, %e = filename without extension", this);
        hint->setStyleSheet("color: rgba(255,255,255,100); font-size: 11px;");
        mainLay->addWidget(hint);

        mainLay->addStretch();

        auto* btnLay = new QHBoxLayout;
        btnLay->addStretch();
        auto* okBtn = new QPushButton("Save", this);
        btnLay->addWidget(okBtn);
        mainLay->addLayout(btnLay);

        connect(okBtn, &QPushButton::clicked, this, [this](){
            GlassSettings::instance().setBuildCommand(m_buildEdit->text());
            GlassSettings::instance().setExecuteCommand(m_execEdit->text());
            accept();
        });

        installEventFilter(this);
    }

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
    QLineEdit* m_buildEdit;
    QLineEdit* m_execEdit;
    QPoint m_dragPos;
};

#endif // GLASS_BUILD_CONFIG_DIALOG_H
