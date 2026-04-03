/**
 * GlassMarkdownPreview.h — Live Markdown Rendering for BobUI
 */

#ifndef GLASS_MARKDOWN_PREVIEW_H
#define GLASS_MARKDOWN_PREVIEW_H

#include "LiquidGlass.h"
#include <QTextBrowser>
#include <QVBoxLayout>

class GlassMarkdownPreview : public QWidget {
public:
    explicit GlassMarkdownPreview(QWidget* parent = nullptr) : QWidget(parent) {
        auto* lay = new QVBoxLayout(this);
        lay->setContentsMargins(0, 0, 0, 0);

        m_browser = new QTextBrowser(this);
        m_browser->setStyleSheet(
            "QTextBrowser { background: rgba(15, 18, 25, 180); color: #e0e0e0; border: none; padding: 20px; font-family: 'Segoe UI'; }"
        );
        lay->addWidget(m_browser);
    }

    void updateMarkdown(const QString& md) {
        m_browser->setMarkdown(md);
    }

private:
    QTextBrowser* m_browser;
};

#endif // GLASS_MARKDOWN_PREVIEW_H
