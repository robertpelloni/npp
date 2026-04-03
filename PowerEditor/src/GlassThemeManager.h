/**
 * GlassThemeManager.h — Notepad++ Style/Theme Parser for BobUI
 *
 * Ingests native stylers.xml and theme XML files to map colors to
 * Lexilla/Scintilla styles.
 */

#ifndef GLASS_THEME_MANAGER_H
#define GLASS_THEME_MANAGER_H

#include <QString>
#include <QMap>
#include <QColor>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

struct StyleSettings {
    int id;
    QColor fg;
    QColor bg;
    bool bold = false;
    bool italic = false;
    QString fontName;
    int fontSize = 0;
};

class GlassThemeManager {
public:
    static GlassThemeManager& instance() {
        static GlassThemeManager inst;
        return inst;
    }

    bool loadTheme(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

        m_styles.clear();
        QXmlStreamReader xml(&file);

        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();
            if (xml.isStartElement()) {
                if (xml.name() == "LexerType") {
                    QString lexName = xml.attributes().value("name").toString();
                    parseLexer(xml, lexName);
                } else if (xml.name() == "GlobalStyles") {
                    parseLexer(xml, "global");
                }
            }
        }

        return !xml.hasError();
    }

    QList<StyleSettings> getStyles(const QString& lexer) const {
        return m_styles.value(lexer);
    }

    // Convert hex string "RRGGBB" to QColor
    static QColor parseHex(const QString& hex) {
        if (hex.isEmpty()) return QColor();
        bool ok;
        unsigned int val = hex.toUInt(&ok, 16);
        if (!ok) return QColor();
        // stylers.xml uses RRGGBB
        return QColor((val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);
    }

    // Convert QColor to Scintilla BGR format
    static int toSciColor(const QColor& c) {
        return c.red() | (c.green() << 8) | (c.blue() << 16);
    }

private:
    void parseLexer(QXmlStreamReader& xml, const QString& lexerName) {
        while (!(xml.isEndElement() && (xml.name() == "LexerType" || xml.name() == "GlobalStyles"))) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "WordsStyle") {
                auto attr = xml.attributes();
                StyleSettings s;
                s.id = attr.value("styleID").toInt();
                s.fg = parseHex(attr.value("fgColor").toString());
                s.bg = parseHex(attr.value("bgColor").toString());
                int fontStyle = attr.value("fontStyle").toInt();
                s.bold = (fontStyle & 1);
                s.italic = (fontStyle & 2);
                s.fontName = attr.value("fontName").toString();
                s.fontSize = attr.value("fontSize").toInt();
                m_styles[lexerName].append(s);
            }
        }
    }

    QMap<QString, QList<StyleSettings>> m_styles;
};

#endif // GLASS_THEME_MANAGER_H
