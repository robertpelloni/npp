#ifndef BOB_SCINTILLA_H
#define BOB_SCINTILLA_H

#include <QWidget>
#include <QResizeEvent>
#include <QWindow>
#include <QString>
#include <QByteArray>
#include <QDebug>
#include <windows.h>
#include "Scintilla.h"
#include "Sci_Position.h"

// Note: To compile this, the target must link against Scintilla and Lexilla.

class BobScintilla : public QWidget {
public:
    explicit BobScintilla(QWidget* parent = nullptr) : QWidget(parent) {
        // We need a native window handle for the parent
        setAttribute(Qt::WA_NativeWindow);
        
        HINSTANCE hInstance = GetModuleHandle(NULL);
        
        m_sciHwnd = CreateWindowExW(
            0,
            L"Scintilla",
            L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN,
            0, 0, 100, 100,
            reinterpret_cast<HWND>(winId()),
            NULL,
            hInstance,
            NULL
        );

        if (!m_sciHwnd) {
            qWarning() << "Failed to create Scintilla HWND!";
            return;
        }

        // Set modern DirectWrite technology for better rendering
        send(SCI_SETTECHNOLOGY, SC_TECHNOLOGY_DIRECTWRITE);
        
        // Setup initial styling (Dark mode defaults to blend with glass)
        send(SCI_STYLESETBACK, STYLE_DEFAULT, 0x1A140E); // BGR format (dark navy)
        send(SCI_STYLESETFORE, STYLE_DEFAULT, 0xEBEBEB);
        send(SCI_STYLECLEARALL, 0, 0);

        send(SCI_SETCARETFORE, 0xFFFFFF, 0);
        send(SCI_SETSELBACK, 1, 0x805A20); // BGR for #205A80
        
        // Margins
        send(SCI_SETMARGINWIDTHN, 0, 40); // Line number margin
        send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0x1A140E);
        send(SCI_STYLESETFORE, STYLE_LINENUMBER, 0x888888);
        
        // Hide margin 1 (usually symbols)
        send(SCI_SETMARGINWIDTHN, 1, 0);
        send(SCI_SETMARGINWIDTHN, 2, 0);

        // Word wrap off by default
        send(SCI_SETWRAPMODE, SC_WRAP_NONE);
    }

    ~BobScintilla() {
        if (m_sciHwnd) {
            DestroyWindow(m_sciHwnd);
        }
    }

    sptr_t send(unsigned int msg, uptr_t wp = 0, sptr_t lp = 0) const {
        if (!m_sciHwnd) return 0;
        return SendMessageW(m_sciHwnd, msg, wp, lp);
    }

    void setText(const QString& text) {
        QByteArray utf8 = text.toUtf8();
        send(SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(utf8.constData()));
    }

    QString text() const {
        sptr_t len = send(SCI_GETLENGTH);
        if (len == 0) return QString();
        
        QByteArray buffer;
        buffer.resize(static_cast<int>(len + 1));
        send(SCI_GETTEXT, len + 1, reinterpret_cast<sptr_t>(buffer.data()));
        buffer.resize(static_cast<int>(len)); // Remove null terminator
        
        return QString::fromUtf8(buffer);
    }

    void setWordWrap(bool wrap) {
        send(SCI_SETWRAPMODE, wrap ? SC_WRAP_WORD : SC_WRAP_NONE);
    }
    bool wordWrap() const {
        return send(SCI_GETWRAPMODE) != SC_WRAP_NONE;
    }

    void undo() { send(SCI_UNDO); }
    void redo() { send(SCI_REDO); }
    void cut()  { send(SCI_CUT); }
    void copy() { send(SCI_COPY); }
    void paste(){ send(SCI_PASTE); }
    void selectAll() { send(SCI_SELECTALL); }

    // Dummy properties for font compatibility with settings
    void setFont(const QFont& f) {
        QWidget::setFont(f);
        QByteArray name = f.family().toUtf8();
        send(SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<sptr_t>(name.constData()));
        send(SCI_STYLESETSIZE, STYLE_DEFAULT, f.pointSize());
        send(SCI_STYLECLEARALL);
        
        // Re-apply Liquid Glass text colors after clearall
        send(SCI_STYLESETBACK, STYLE_DEFAULT, 0x1A140E); // BGR dark navy
        send(SCI_STYLESETFORE, STYLE_DEFAULT, 0xEBEBEB); // BGR off-white
        send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0x1A140E);
        send(SCI_STYLESETFORE, STYLE_LINENUMBER, 0x888888);
    }

    std::function<void()> onCursorPositionChanged;
    std::function<void()> onContentsChanged;
    std::function<void(bool)> onModificationChanged;

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_NOTIFY) {
            SCNotification* scn = reinterpret_cast<SCNotification*>(msg->lParam);
            if (scn->nmhdr.hwndFrom == m_sciHwnd) {
                if (scn->nmhdr.code == SCN_MODIFIED) {
                    if (onContentsChanged) onContentsChanged();
                    if (onModificationChanged) onModificationChanged(true);
                } else if (scn->nmhdr.code == SCN_UPDATEUI) {
                    if (onCursorPositionChanged) onCursorPositionChanged();
                }
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }

    void resizeEvent(QResizeEvent* e) override {
        QWidget::resizeEvent(e);
        if (m_sciHwnd) {
            MoveWindow(m_sciHwnd, 0, 0, width(), height(), TRUE);
        }
    }

private:
    HWND m_sciHwnd = nullptr;
};

#endif // BOB_SCINTILLA_H
