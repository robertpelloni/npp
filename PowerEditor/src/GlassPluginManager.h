/**
 * GlassPluginManager.h — Notepad++ Plugin Bridge for BobUI
 */

#ifndef GLASS_PLUGIN_MANAGER_H
#define GLASS_PLUGIN_MANAGER_H

#include <QString>
#include <QDir>
#include <QLibrary>
#include <QVector>
#include <QDebug>
#include <windows.h>
#include "Scintilla.h"

// Notepad++ SDK Structs
struct NppData {
    HWND _nppHandle;
    HWND _scintillaMainHandle;
    HWND _scintillaSecondHandle;
};

typedef const TCHAR * (__cdecl * PGetPluginName)();
typedef void (__cdecl * PSetInfo)(NppData);
typedef void (__cdecl * PBeNotified)(SCNotification *);

struct GlassPlugin {
    QString name;
    QString filePath;
    QLibrary* library;
    PSetInfo setInfo;
    PBeNotified beNotified;
};

class GlassPluginManager {
public:
    static GlassPluginManager& instance() {
        static GlassPluginManager inst;
        return inst;
    }

    void loadPlugins(const QString& path, NppData data) {
        QDir dir(path);
        QStringList filters;
        filters << "*.dll";
        
        for (const QString& fileName : dir.entryList(filters, QDir::Files)) {
            QString fullPath = dir.absoluteFilePath(fileName);
            QLibrary* lib = new QLibrary(fullPath);
            if (lib->load()) {
                GlassPlugin gp;
                gp.filePath = fullPath;
                gp.library = lib;
                
                auto getName = (PGetPluginName)lib->resolve("getName");
                if (getName) {
                    gp.name = QString::fromWCharArray(getName());
                } else {
                    gp.name = fileName;
                }
                
                gp.setInfo = (PSetInfo)lib->resolve("setInfo");
                gp.beNotified = (PBeNotified)lib->resolve("beNotified");
                
                if (gp.setInfo) {
                    gp.setInfo(data);
                }
                
                m_plugins.append(gp);
                qDebug() << "Loaded plugin:" << gp.name;
            } else {
                qWarning() << "Failed to load plugin:" << fullPath << lib->errorString();
                delete lib;
            }
        }
    }

    void notifyAll(SCNotification* scn) {
        for (auto& p : m_plugins) {
            if (p.beNotified) {
                p.beNotified(scn);
            }
        }
    }

    const QVector<GlassPlugin>& plugins() const { return m_plugins; }

private:
    GlassPluginManager() = default;
    QVector<GlassPlugin> m_plugins;
};

#endif // GLASS_PLUGIN_MANAGER_H
