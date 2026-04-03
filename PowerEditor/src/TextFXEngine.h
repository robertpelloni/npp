#ifndef TEXTFX_ENGINE_H
#define TEXTFX_ENGINE_H

#include <QString>
#include <QStringList>
#include <algorithm>

// Native Engine for all TextFX features (NPP Overhaul)
class TextFXEngine {
public:
    // SUBSECTION: Sort routines
    static QString sortLines(const QString& text, bool caseSensitive = true) {
        QStringList lines = text.split('\n');
        if (caseSensitive) {
            std::sort(lines.begin(), lines.end());
        } else {
            std::sort(lines.begin(), lines.end(), [](const QString& a, const QString& b) {
                return a.toLower() < b.toLower();
            });
        }
        return lines.join('\n');
    }

    // SUBSECTION: Line transformations
    static QString deleteBlankLines(const QString& text) {
        QStringList lines = text.split('\n');
        QStringList result;
        for (const auto& line : lines) {
            if (!line.trimmed().isEmpty()) {
                result << line;
            }
        }
        return result.join('\n');
    }

    static QString deleteSurplusBlankLines(const QString& text) {
        QStringList lines = text.split('\n');
        QStringList result;
        bool lastWasBlank = false;
        for (const auto& line : lines) {
            bool isBlank = line.trimmed().isEmpty();
            if (!isBlank || !lastWasBlank) {
                result << line;
            }
            lastWasBlank = isBlank;
        }
        return result.join('\n');
    }

    // SUBSECTION: Character transformations
    static QString zapNonPrintable(const QString& text) {
        QString result = text;
        for (int i = 0; i < result.length(); ++i) {
            if (result[i].unicode() < 32 && result[i] != '\n' && result[i] != '\r' && result[i] != '\t') {
                result[i] = '#';
            }
        }
        return result;
    }

    // SUBSECTION: Case conversions
    static QString toUpperCase(const QString& text) {
        return text.toUpper();
    }

    static QString toLowerCase(const QString& text) {
        return text.toLower();
    }

    static QString invertCase(const QString& text) {
        QString result = text;
        for (int i = 0; i < result.length(); ++i) {
            if (result[i].isUpper()) result[i] = result[i].toLower();
            else if (result[i].isLower()) result[i] = result[i].toUpper();
        }
        return result;
    }

    static QString properCase(const QString& text) {
        QString result = text.toLower();
        bool nextIsUpper = true;
        for (int i = 0; i < result.length(); ++i) {
            if (result[i].isSpace() || result[i].isPunct()) {
                nextIsUpper = true;
            } else if (nextIsUpper && result[i].isLetter()) {
                result[i] = result[i].toUpper();
                nextIsUpper = false;
            }
        }
        return result;
    }

    // SUBSECTION: Line numbering
    static QString insertLineNumbers(const QString& text) {
        QStringList lines = text.split('\n');
        QStringList result;
        int maxDigits = QString::number(lines.size()).length();
        for (int i = 0; i < lines.size(); ++i) {
            QString num = QString::number(i + 1).rightJustified(maxDigits, '0');
            result << num + " " + lines[i];
        }
        return result.join('\n');
    }
    
    static QString removeLineNumbers(const QString& text) {
        QStringList lines = text.split('\n');
        QStringList result;
        for (const auto& line : lines) {
            int spaceIdx = line.indexOf(' ');
            if (spaceIdx > 0 && spaceIdx <= 8) { // Basic heuristic for numbered lines
                bool isNum;
                line.left(spaceIdx).toInt(&isNum);
                if (isNum) {
                    result << line.mid(spaceIdx + 1);
                    continue;
                }
            }
            result << line;
        }
        return result.join('\n');
    }
};

#endif // TEXTFX_ENGINE_H
