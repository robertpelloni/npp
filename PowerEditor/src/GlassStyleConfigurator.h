/**
 * GlassStyleConfigurator.h — Liquid Glass Style/Theme Editor
 */

#ifndef GLASS_STYLE_CONFIGURATOR_H
#define GLASS_STYLE_CONFIGURATOR_H

#include "LiquidGlass.h"
#include "GlassSettings.h"
#include "GlassThemeManager.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include <QMouseEvent>

class GlassStyleConfigurator : public QDialog {
public:
    explicit GlassStyleConfigurator(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Style Configurator");
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(700, 500);
        setStyleSheet(LiquidGlassStyleSheet::kDialog);

        auto* mainLay = new QVBoxLayout(this);
        mainLay->setContentsMargins(20, 20, 20, 20);

        // Title
        auto* titleLabel = new QLabel("🎨 Style Configurator", this);
        titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
        mainLay->addWidget(titleLabel);

        auto* bodyLay = new QHBoxLayout;
        
        // Lexer List
        m_lexerList = new QListWidget(this);
        m_lexerList->addItem("Global Styles");
        m_lexerList->addItem("C++");
        m_lexerList->addItem("Python");
        m_lexerList->addItem("HTML");
        m_lexerList->setFixedWidth(150);
        bodyLay->addWidget(m_lexerList);

        // Style Detail
        auto* detailLay = new QVBoxLayout;
        m_styleList = new QListWidget(this);
        detailLay->addWidget(new QLabel("Styles:"));
        detailLay->addWidget(m_styleList);
        
        auto* colorLay = new QHBoxLayout;
        m_fgBtn = new QPushButton("Foreground Color", this);
        m_bgBtn = new QPushButton("Background Color", this);
        colorLay->addWidget(m_fgBtn);
        colorLay->addWidget(m_bgBtn);
        detailLay->addLayout(colorLay);
        
        bodyLay->addLayout(detailLay);
        mainLay->addLayout(bodyLay);

        auto* bottomLay = new QHBoxLayout;
        bottomLay->addStretch();
        auto* closeBtn = new QPushButton("Save & Close", this);
        bottomLay->addWidget(closeBtn);
        mainLay->addLayout(bottomLay);

        connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
        
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
    QListWidget* m_lexerList;
    QListWidget* m_styleList;
    QPushButton* m_fgBtn;
    QPushButton* m_bgBtn;
    QPoint m_dragPos;
};

#endif // GLASS_STYLE_CONFIGURATOR_H
