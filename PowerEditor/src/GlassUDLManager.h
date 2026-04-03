/**
 * GlassUDLManager.h — User Defined Language (UDL) Parser for BobUI
 */

#ifndef GLASS_UDL_MANAGER_H
#define GLASS_UDL_MANAGER_H

#include <QString>
#include <QFile>
#include <QXmlStreamReader>
#include <QMap>
#include <QDebug>
#include <functional>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "Scintilla.h"
#include "GlassThemeManager.h"

struct UDLStyle {
    int id;
    QColor fg;
    QColor bg;
    bool bold = false;
    bool italic = false;
};

struct UDLLanguage {
    QString name;
    QString extension;
    QStringList keywords;
    QList<UDLStyle> styles;
};

class GlassUDLManager {
public:
    static GlassUDLManager& instance() {
        static GlassUDLManager inst;
        return inst;
    }

    void loadUDLs(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

        m_languages.clear();
        QXmlStreamReader xml(&file);
        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "UserLang") {
                parseUDL(xml);
            }
        }
    }

    void applyUDL(std::function<sptr_t(unsigned int, uptr_t, sptr_t)> send, const QString& ext) {
        UDLLanguage* lang = findByExtension(ext);
        if (!lang) return;

        // Reset to null lexer (id 0)
        send(4001, 0, 0); 
        
        for (const auto& s : lang->styles) {
            if (s.fg.isValid()) send(SCI_STYLESETFORE, s.id, GlassThemeManager::toSciColor(s.fg));
            if (s.bg.isValid()) send(SCI_STYLESETBACK, s.id, GlassThemeManager::toSciColor(s.bg));
            send(SCI_STYLESETBOLD, s.id, s.bold ? 1 : 0);
            send(SCI_STYLESETITALIC, s.id, s.italic ? 1 : 0);
        }

        // Apply keywords to list 0
        QString kw = lang->keywords.join(" ");
        send(SCI_SETKEYWORDS, 0, reinterpret_cast<sptr_t>(kw.toUtf8().constData()));
    }

    UDLLanguage* findByExtension(const QString& ext) {
        for (auto& lang : m_languages) {
            if (lang.extension.split(' ').contains(ext, Qt::CaseInsensitive)) return &lang;
        }
        return nullptr;
    }

private:
    void parseUDL(QXmlStreamReader& xml) {
        UDLLanguage lang;
        lang.name = xml.attributes().value("name").toString();
        lang.extension = xml.attributes().value("ext").toString();

        while (!(xml.isEndElement() && xml.name() == "UserLang")) {
            xml.readNext();
            if (xml.isStartElement()) {
                if (xml.name() == "Keywords") {
                    lang.keywords << xml.readElementText().split(' ', Qt::SkipEmptyParts);
                } else if (xml.name() == "WordsStyle") {
                    UDLStyle s;
                    s.id = xml.attributes().value("styleID").toInt();
                    s.fg = GlassThemeManager::parseHex(xml.attributes().value("fgColor").toString());
                    s.bg = GlassThemeManager::parseHex(xml.attributes().value("bgColor").toString());
                    int fontStyle = xml.attributes().value("fontStyle").toInt();
                    s.bold = (fontStyle & 1);
                    s.italic = (fontStyle & 2);
                    lang.styles.append(s);
                }
            }
        }
        m_languages.append(lang);
    }

    GlassUDLManager() = default;
    QList<UDLLanguage> m_languages;
};

#endif // GLASS_UDL_MANAGER_H
