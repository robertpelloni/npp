/**
 * GlassFunctionParser.h — High-performance Regex-based Function Discovery
 */

#ifndef GLASS_FUNCTION_PARSER_H
#define GLASS_FUNCTION_PARSER_H

#include <QString>
#include <QRegularExpression>
#include <QListWidget>
#include <QMap>

struct FunctionItem {
    QString name;
    int line;
};

class GlassFunctionParser {
public:
    static QList<FunctionItem> parse(const QString& text, const QString& ext) {
        QList<FunctionItem> results;
        QString pattern;

        if (ext == "cpp" || ext == "h" || ext == "hpp" || ext == "c") {
            // Basic C++ function regex
            pattern = R"((?:[a-zA-Z_][a-zA-Z0-9_*&]*\s+)+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*\{)";
        } else if (ext == "py") {
            // Python def regex
            pattern = R"(def\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\()";
        } else if (ext == "js" || ext == "ts") {
            pattern = R"(function\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\()";
        } else {
            return results;
        }

        QRegularExpression rx(pattern);
        auto it = rx.globalMatch(text);
        
        // Split text by lines to find line numbers
        QStringList lines = text.split('\n');

        while (it.hasNext()) {
            auto match = it.next();
            QString name = match.captured(1);
            int offset = match.capturedStart();
            
            // Calculate line number
            int lineNum = text.left(offset).count('\n') + 1;
            results.append({name, lineNum});
        }

        return results;
    }
};

#endif // GLASS_FUNCTION_PARSER_H
