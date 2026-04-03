/**
 * GlassSnippetsManager.h — Code Snippet Management for BobUI
 */

#ifndef GLASS_SNIPPETS_MANAGER_H
#define GLASS_SNIPPETS_MANAGER_H

#include <QString>
#include <QMap>
#include <QListWidget>

class GlassSnippetsManager {
public:
    static GlassSnippetsManager& instance() {
        static GlassSnippetsManager inst;
        return inst;
    }

    void loadDefaultSnippets() {
        m_snippets["cpp"]["for"] = "for (int i = 0; i < n; ++i) {\n\t\n}";
        m_snippets["cpp"]["if"] = "if (condition) {\n\t\n}";
        m_snippets["py"]["def"] = "def function_name(args):\n\t\"\"\"docstring\"\"\"\n\tpass";
        m_snippets["html"]["html5"] = "<!DOCTYPE html>\n<html>\n<head>\n\t<title></title>\n</head>\n<body>\n\n</body>\n</html>";
    }

    QString getSnippet(const QString& lang, const QString& key) {
        return m_snippets.value(lang).value(key);
    }

    QStringList getKeys(const QString& lang) {
        return m_snippets.value(lang).keys();
    }

private:
    GlassSnippetsManager() = default;
    QMap<QString, QMap<QString, QString>> m_snippets; // lang -> (key -> code)
};

#endif // GLASS_SNIPPETS_MANAGER_H
