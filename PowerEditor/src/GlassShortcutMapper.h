/**
 * GlassShortcutMapper.h — Liquid Glass Shortcut Configuration
 */

#ifndef GLASS_SHORTCUT_MAPPER_H
#define GLASS_SHORTCUT_MAPPER_H

#include "LiquidGlass.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>

class GlassShortcutMapper : public QDialog {
public:
    explicit GlassShortcutMapper(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Shortcut Mapper");
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(600, 500);
        setStyleSheet(LiquidGlassStyleSheet::kDialog);

        auto* mainLay = new QVBoxLayout(this);
        mainLay->setContentsMargins(20, 20, 20, 20);

        auto* title = new QLabel("⌨ Shortcut Mapper", this);
        title->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
        mainLay->addWidget(title);

        m_table = new QTableWidget(10, 2, this);
        m_table->setHorizontalHeaderLabels({"Action", "Shortcut"});
        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_table->setStyleSheet(
            "QTableWidget { background: rgba(0,0,0,50); color: white; border: 1px solid rgba(255,255,255,10); }"
            "QHeaderView::section { background: rgba(255,255,255,15); color: white; padding: 5px; }"
        );
        mainLay->addWidget(m_table);

        auto* closeBtn = new QPushButton("Close", this);
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
        mainLay->addWidget(closeBtn);

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
    QTableWidget* m_table;
    QPoint m_dragPos;
};

#endif // GLASS_SHORTCUT_MAPPER_H
