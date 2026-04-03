/**
 * GlassConfigXML.h — Notepad++ XML Interoperability (config.xml, shortcuts.xml)
 */

#ifndef GLASS_CONFIG_XML_H
#define GLASS_CONFIG_XML_H

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QStringList>
#include "GlassSettings.h"

class GlassConfigXML {
public:
    static void saveConfig(const QString& path) {
        auto& s = GlassSettings::instance();
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

        QXmlStreamWriter xml(&file);
        xml.setAutoFormatting(true);
        xml.writeStartDocument();
        xml.writeStartElement("NotepadPlus");
        
        xml.writeStartElement("GUIConfigs");
        
        auto writeCfg = [&](const QString& name, const QString& val) {
            xml.writeStartElement("GUIConfig");
            xml.writeAttribute("name", name);
            xml.writeCharacters(val);
            xml.writeEndElement();
        };

        writeCfg("AppGlassIntensity", QString::number(s.glassIntensity()));
        writeCfg("AppBubbleAnimations", s.bubbleAnimations() ? "yes" : "no");
        writeCfg("AppDarkMode", s.darkMode() ? "yes" : "no");
        writeCfg("EditorFontName", s.editorFont().family());
        writeCfg("EditorFontSize", QString::number(s.editorFont().pointSize()));
        writeCfg("WordWrap", s.wordWrapDefault() ? "yes" : "no");
        
        xml.writeEndElement(); // GUIConfigs
        xml.writeEndElement(); // NotepadPlus
        xml.writeEndDocument();
    }

    static void loadConfig(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

        auto& s = GlassSettings::instance();
        QXmlStreamReader xml(&file);
        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "GUIConfig") {
                QString name = xml.attributes().value("name").toString();
                QString val = xml.readElementText();
                
                if (name == "AppGlassIntensity") s.setGlassIntensity(val.toInt());
                else if (name == "AppBubbleAnimations") s.setBubbleAnimations(val == "yes");
                else if (name == "AppDarkMode") s.setDarkMode(val == "yes");
                else if (name == "EditorFontSize") {
                    QFont f = s.editorFont();
                    f.setPointSize(val.toInt());
                    s.setEditorFont(f);
                }
                else if (name == "WordWrap") s.setWordWrapDefault(val == "yes");
            }
        }
    }

    static void saveMacros(const QString& path, const QList<MacroCommand>& commands) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

        QXmlStreamWriter xml(&file);
        xml.setAutoFormatting(true);
        xml.writeStartDocument();
        xml.writeStartElement("NotepadPlus");
        xml.writeStartElement("Macros");
        
        xml.writeStartElement("Macro");
        xml.writeAttribute("name", "Last Recorded Macro");
        
        for (const auto& cmd : commands) {
            xml.writeStartElement("Action");
            xml.writeAttribute("type", "0"); // Scintilla
            xml.writeAttribute("message", QString::number((qlonglong)cmd.msg));
            xml.writeAttribute("wParam", QString::number((qlonglong)cmd.wp));
            xml.writeAttribute("lParam", QString::number((qlonglong)cmd.lp));
            xml.writeAttribute("sParam", "");
            xml.writeEndElement();
        }
        
        xml.writeEndElement(); // Macro
        xml.writeEndElement(); // Macros
        xml.writeEndElement(); // NotepadPlus
        xml.writeEndDocument();
    }
};

#endif // GLASS_CONFIG_XML_H
