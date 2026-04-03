/**
 * GlassSettings.h — Persistent configuration for Notepad++ BobUI
 *
 * Uses QSettings to store and retrieve user preferences, window state,
 * and editor configurations (font, word wrap, themes, glass intensity).
 * Provides a centralized singleton manager.
 */

#ifndef GLASS_SETTINGS_H
#define GLASS_SETTINGS_H

#include <QSettings>
#include <QString>
#include <QFont>
#include <QColor>
#include <QSize>
#include <QPoint>

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "Scintilla.h"

struct MacroCommand {
    unsigned int msg;
    uptr_t wp;
    sptr_t lp;
};

class GlassSettings {
public:
    static GlassSettings& instance() {
        static GlassSettings instance;
        return instance;
    }

    // ── Window State ──
    QByteArray windowGeometry() const { return m_settings.value("Window/Geometry").toByteArray(); }
    void setWindowGeometry(const QByteArray& geom) { m_settings.setValue("Window/Geometry", geom); }

    QByteArray windowState() const { return m_settings.value("Window/State").toByteArray(); }
    void setWindowState(const QByteArray& state) { m_settings.setValue("Window/State", state); }

    // ── UI Preferences ──
    int glassIntensity() const { return m_settings.value("UI/GlassIntensity", 2).toInt(); } // 0=Mica, 1=MicaTabbed, 2=Acrylic
    void setGlassIntensity(int level) { m_settings.setValue("UI/GlassIntensity", level); }

    bool bubbleAnimations() const { return m_settings.value("UI/BubbleAnimations", true).toBool(); }
    void setBubbleAnimations(bool enable) { m_settings.setValue("UI/BubbleAnimations", enable); }

    bool darkMode() const { return m_settings.value("UI/DarkMode", true).toBool(); }
    void setDarkMode(bool dark) { m_settings.setValue("UI/DarkMode", dark); }

    // ── Backup / Auto-save ──
    bool backupEnabled() const { return m_settings.value("Backup/Enabled", true).toBool(); }
    void setBackupEnabled(bool enable) { m_settings.setValue("Backup/Enabled", enable); }
    int backupInterval() const { return m_settings.value("Backup/Interval", 30000).toInt(); }
    void setBackupInterval(int ms) { m_settings.setValue("Backup/Interval", ms); }

    // ── Build System (Geany Parity) ──
    QString buildCommand() const { return m_settings.value("Build/Build", "make").toString(); }
    void setBuildCommand(const QString& cmd) { m_settings.setValue("Build/Build", cmd); }
    
    QString executeCommand() const { return m_settings.value("Build/Execute", "./%e").toString(); }
    void setExecuteCommand(const QString& cmd) { m_settings.setValue("Build/Execute", cmd); }

    // ── View Modes ──
    bool distractionFree() const { return m_settings.value("View/DistractionFree", false).toBool(); }
    void setDistractionFree(bool enable) { m_settings.setValue("View/DistractionFree", enable); }
    bool showMinimap() const { return m_settings.value("View/ShowMinimap", true).toBool(); }
    void setShowMinimap(bool show) { m_settings.setValue("View/ShowMinimap", show); }

    // ── Editor Preferences ──
    QFont editorFont() const { 
        QFont defaultFont("Cascadia Code", 11);
        defaultFont.setStyleHint(QFont::Monospace);
        return m_settings.value("Editor/Font", defaultFont).value<QFont>(); 
    }
    void setEditorFont(const QFont& font) { m_settings.setValue("Editor/Font", font); }

    bool wordWrapDefault() const { return m_settings.value("Editor/WordWrap", false).toBool(); }
    void setWordWrapDefault(bool wrap) { m_settings.setValue("Editor/WordWrap", wrap); }

    QString defaultEncoding() const { return m_settings.value("Editor/Encoding", "UTF-8").toString(); }
    void setDefaultEncoding(const QString& enc) { m_settings.setValue("Editor/Encoding", enc); }

    // ── Find & Replace State ──
    bool findMatchCase() const { return m_settings.value("Find/MatchCase", false).toBool(); }
    void setFindMatchCase(bool val) { m_settings.setValue("Find/MatchCase", val); }

    bool findWholeWord() const { return m_settings.value("Find/WholeWord", false).toBool(); }
    void setFindWholeWord(bool val) { m_settings.setValue("Find/WholeWord", val); }

    bool findUseRegex() const { return m_settings.value("Find/UseRegex", false).toBool(); }
    void setFindUseRegex(bool val) { m_settings.setValue("Find/UseRegex", val); }

    // ── Session Management ──
    QStringList openFiles() const { return m_settings.value("Session/OpenFiles").toStringList(); }
    void setOpenFiles(const QStringList& files) { m_settings.setValue("Session/OpenFiles", files); }
    
    int activeTab() const { return m_settings.value("Session/ActiveTab", 0).toInt(); }
    void setActiveTab(int index) { m_settings.setValue("Session/ActiveTab", index); }

    QStringList recentFiles() const { return m_settings.value("Session/RecentFiles").toStringList(); }
    void setRecentFiles(const QStringList& files) { m_settings.setValue("Session/RecentFiles", files); }

    bool autoReload() const { return m_settings.value("Backup/AutoReload", false).toBool(); }
    void setAutoReload(bool enable) { m_settings.setValue("Backup/AutoReload", enable); }

private:
    GlassSettings() : m_settings("robertpelloni", "Notepad++ BobUI Liquid Glass") {}
    ~GlassSettings() = default;

    QSettings m_settings;
};

#endif // GLASS_SETTINGS_H
