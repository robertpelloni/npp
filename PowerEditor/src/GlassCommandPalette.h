/**
 * GlassCommandPalette.h — VS Code style Command Palette for Notepad++ BobUI
 */

#ifndef GLASS_COMMAND_PALETTE_H
#define GLASS_COMMAND_PALETTE_H

#include "LiquidGlass.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QAction>
#include <QKeyEvent>
#include <QApplication>
#include <QMenuBar>
#include <QScreen>

class GlassCommandPalette : public QDialog {
public:
    explicit GlassCommandPalette(QWidget* parent = nullptr) : QDialog(parent) {
        setModal(true);
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(600, 400);
        setStyleSheet(LiquidGlassStyleSheet::kDialog);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);
        layout->setSpacing(5);

        m_searchEdit = new QLineEdit(this);
        m_searchEdit->setPlaceholderText("Type a command to run...");
        m_searchEdit->setStyleSheet(
            "QLineEdit { background: rgba(255, 255, 255, 20); border: 1px solid rgba(255, 255, 255, 30); "
            "border-radius: 8px; padding: 12px; color: white; font-size: 16px; font-family: 'Segoe UI'; }"
        );
        layout->addWidget(m_searchEdit);

        m_listWidget = new QListWidget(this);
        m_listWidget->setStyleSheet(
            "QListWidget { background: transparent; border: none; outline: none; }"
            "QListWidget::item { color: rgba(235, 240, 255, 200); padding: 10px; border-bottom: 1px solid rgba(255, 255, 255, 10); }"
            "QListWidget::item:selected { background: rgba(90, 130, 255, 100); color: white; border-radius: 6px; }"
        );
        layout->addWidget(m_listWidget);

        connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString& text){
            filterCommands(text);
        });
        connect(m_listWidget, &QListWidget::itemActivated, this, [this](){
            executeSelected();
        });
        
        m_searchEdit->installEventFilter(this);
    }

    void populate(const QList<QAction*>& actions) {
        m_allActions = actions;
        filterCommands("");
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        LiquidGlassPainter::drawGlassBackground(p, QRectF(rect()), 14.0, true);
    }

    bool eventFilter(QObject* obj, QEvent* ev) override {
        if (obj == m_searchEdit && ev->type() == QEvent::KeyPress) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(ev);
            if (ke->key() == Qt::Key_Down) {
                m_listWidget->setCurrentRow((m_listWidget->currentRow() + 1) % m_listWidget->count());
                return true;
            } else if (ke->key() == Qt::Key_Up) {
                int next = m_listWidget->currentRow() - 1;
                if (next < 0) next = m_listWidget->count() - 1;
                m_listWidget->setCurrentRow(next);
                return true;
            } else if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                executeSelected();
                return true;
            } else if (ke->key() == Qt::Key_Escape) {
                reject();
                return true;
            }
        }
        return QDialog::eventFilter(obj, ev);
    }

private slots:
    void filterCommands(const QString& text) {
        m_listWidget->clear();
        for (auto* action : m_allActions) {
            if (action->text().isEmpty() || action->text().contains("&")) {
                QString cleanText = action->text().replace("&", "");
                if (cleanText.contains(text, Qt::CaseInsensitive)) {
                    auto* item = new QListWidgetItem(cleanText, m_listWidget);
                    item->setData(Qt::UserRole, QVariant::fromValue(action));
                }
            } else if (action->text().contains(text, Qt::CaseInsensitive)) {
                auto* item = new QListWidgetItem(action->text(), m_listWidget);
                item->setData(Qt::UserRole, QVariant::fromValue(action));
            }
        }
        if (m_listWidget->count() > 0) {
            m_listWidget->setCurrentRow(0);
        }
    }

    void executeSelected() {
        auto* item = m_listWidget->currentItem();
        if (item) {
            auto* action = item->data(Qt::UserRole).value<QAction*>();
            if (action) {
                accept();
                action->trigger();
            }
        }
    }

private:
    QLineEdit* m_searchEdit;
    QListWidget* m_listWidget;
    QList<QAction*> m_allActions;
};

#endif // GLASS_COMMAND_PALETTE_H
