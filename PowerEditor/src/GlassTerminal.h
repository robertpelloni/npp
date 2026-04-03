/**
 * GlassTerminal.h — Integrated Command Terminal for Notepad++ BobUI
 */

#ifndef GLASS_TERMINAL_H
#define GLASS_TERMINAL_H

#include "BobScintilla.h"
#include "LiquidGlass.h"
#include <QProcess>
#include <QVBoxLayout>
#include <QScrollBar>

class GlassTerminal : public QWidget {
public:
    explicit GlassTerminal(QWidget* parent = nullptr) : QWidget(parent) {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        m_sci = new BobScintilla(this);
        m_sci->send(SCI_SETCARETSTYLE, CARETSTYLE_BLOCK);
        m_sci->send(SCI_SETCARETFORE, 0x00FF00); // Green caret
        
        // Terminal styling (Darker glass)
        m_sci->send(SCI_STYLESETBACK, STYLE_DEFAULT, 0x0A0A0A);
        m_sci->send(SCI_STYLESETFORE, STYLE_DEFAULT, 0xAAAAAA);
        m_sci->send(SCI_STYLECLEARALL);
        
        layout->addWidget(m_sci);

        m_process = new QProcess(this);
        m_process->setProcessChannelMode(QProcess::MergedChannels);

        connect(m_process, &QProcess::readyReadStandardOutput, this, [this](){
            QByteArray data = m_process->readAllStandardOutput();
            appendOutput(QString::fromLocal8Bit(data));
        });

        m_sci->onCharAdded = [this](int ch){
            if (m_process->state() == QProcess::Running) {
                m_process->write(QByteArray(1, (char)ch));
            }
        };

        // Trap Enter key specifically for commands
        // Actually for a simple terminal we just pipe everything
    }

    void startShell() {
#ifdef Q_OS_WIN
        m_process->start("cmd.exe", QStringList());
#else
        m_process->start("/bin/bash", QStringList());
#endif
    }

    void appendOutput(const QString& text) {
        sptr_t len = m_sci->send(SCI_GETLENGTH);
        m_sci->send(SCI_SETREADONLY, 0);
        m_sci->send(SCI_APPENDTEXT, text.length(), reinterpret_cast<sptr_t>(text.toUtf8().constData()));
        m_sci->send(SCI_GOTOPOS, m_sci->send(SCI_GETLENGTH));
        m_sci->send(SCI_SETREADONLY, 1);
    }

private:
    BobScintilla* m_sci;
    QProcess*     m_process;
};

#endif // GLASS_TERMINAL_H
