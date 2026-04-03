/**
 * GlassConfigXML.h — Notepad++ XML Interoperability (config.xml, shortcuts.xml)
 */

#ifndef GLASS_CONFIG_XML_H
#define GLASS_CONFIG_XML_H

#include <QXmlStreamWriter>
#include <QFile>
#include <QStringList>
#include "GlassSettings.h"

class GlassConfigXML {
public:
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
