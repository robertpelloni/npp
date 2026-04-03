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
#include "BobScintilla.h"

struct UDLStyle {
    int id;
    QColor fg;
    QColor bg;
    bool bold;
};

struct UDLLanguage {
    QString name;
    QString extension;
    QString keywords;
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

        QXmlStreamReader xml(&file);
        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "UserLang") {
                parseUDL(xml);
            }
        }
    }

    UDLLanguage* findByExtension(const QString& ext) {
        for (auto& lang : m_languages) {
            if (lang.extension.contains(ext, Qt::CaseInsensitive)) return &lang;
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
                    lang.keywords += xml.readElementText() + " ";
                } else if (xml.name() == "WordsStyle") {
                    UDLStyle s;
                    s.id = xml.attributes().value("styleID").toInt();
                    // ... parse colors ...
                    lang.styles.append(s);
                }
            }
        }
        m_languages.append(lang);
    }

    QList<UDLLanguage> m_languages;
};

#endif // GLASS_UDL_MANAGER_H
