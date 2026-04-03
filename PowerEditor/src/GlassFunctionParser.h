/**
 * GlassFunctionParser.h — XML-configurable Regex Function Parser
 */

#ifndef GLASS_FUNCTION_PARSER_H
#define GLASS_FUNCTION_PARSER_H

#include <QString>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QFile>
#include <QMap>

struct FunctionItem {
    QString name;
    QString type; // "Function", "Class", "Variable"
    int line;
};

class GlassFunctionParser {
public:
    static GlassFunctionParser& instance() {
        static GlassFunctionParser inst;
        return inst;
    }

    void loadConfig(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

        m_parsers.clear();
        QXmlStreamReader xml(&file);
        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == "parser") {
                QString id = xml.attributes().value("id").toString();
                while (!(xml.isEndElement() && xml.name() == "parser")) {
                    xml.readNext();
                    if (xml.isStartElement() && xml.name() == "function") {
                        m_parsers[id]["Function"] = xml.attributes().value("mainExpr").toString();
                    } else if (xml.isStartElement() && xml.name() == "class") {
                        m_parsers[id]["Class"] = xml.attributes().value("mainExpr").toString();
                    }
                }
            }
        }
    }

    QList<FunctionItem> parse(const QString& text, const QString& ext) {
        QList<FunctionItem> results;
        QString lang = ext.toLower();
        if (lang == "h" || lang == "hpp") lang = "cpp";
        if (lang == "py") lang = "python";

        if (!m_parsers.contains(lang)) return results;

        for (auto itMap = m_parsers[lang].begin(); itMap != m_parsers[lang].end(); ++itMap) {
            QString type = itMap.key();
            QRegularExpression rx(itMap.value());
            auto it = rx.globalMatch(text);
            
            while (it.hasNext()) {
                auto match = it.next();
                QString name = match.captured(1);
                int offset = match.capturedStart();
                int lineNum = text.left(offset).count('\n') + 1;
                results.append({name, type, lineNum});
            }
        }
        return results;
    }

private:
    GlassFunctionParser() = default;
    QMap<QString, QMap<QString, QString>> m_parsers; // lang -> (type -> regex)
};

#endif // GLASS_FUNCTION_PARSER_H
