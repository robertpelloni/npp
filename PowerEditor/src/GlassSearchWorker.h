#ifndef GLASS_SEARCH_WORKER_H
#define GLASS_SEARCH_WORKER_H

#include <QThread>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QRegularExpression>
#include <QMutex>
#include <QMetaObject>
#include <QCoreApplication>
#include <functional>

struct SearchResultItem {
    QString filePath;
    int lineNum;
    QString text;
};

class GlassSearchWorker : public QThread {
public:
    GlassSearchWorker(const QString& needle, const QString& dir, const QString& filters, 
                      bool matchCase, bool wholeWord, bool subDirs, QObject* parent = nullptr)
        : QThread(parent), m_needle(needle), m_dir(dir), m_filters(filters), 
          m_matchCase(matchCase), m_wholeWord(wholeWord), m_subDirs(subDirs), m_cancel(false) {}

    ~GlassSearchWorker() override { cancel(); wait(); }

    void cancel() {
        QMutexLocker lock(&m_mutex);
        m_cancel = true;
    }

    std::function<void(const SearchResultItem&)> onResultFound;
    std::function<void(int, const QString&)> onProgressUpdated;
    std::function<void(int, int)> onSearchFinished;

protected:
    void run() override {
        QStringList nameFilters;
        for (const QString& f : m_filters.split(QRegularExpression("[\\s;]+"), Qt::SkipEmptyParts)) {
            nameFilters << f;
        }
        if (nameFilters.isEmpty()) nameFilters << "*";
        
        QDirIterator::IteratorFlags flags = m_subDirs ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
        QDirIterator it(m_dir, nameFilters, QDir::Files | QDir::NoSymLinks, flags);

        int filesScanned = 0;
        int totalMatches = 0;

        Qt::CaseSensitivity cs = m_matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;

        while (it.hasNext()) {
            {
                QMutexLocker lock(&m_mutex);
                if (m_cancel) break;
            }

            QString filePath = it.next();
            filesScanned++;
            
            if (filesScanned % 20 == 0 && onProgressUpdated) {
                QMetaObject::invokeMethod(qApp, [this, filesScanned, filePath](){
                    onProgressUpdated(filesScanned, filePath);
                }, Qt::QueuedConnection);
            }

            QFile f(filePath);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream stream(&f);
                int lineNum = 1;
                while (!stream.atEnd()) {
                    QString line = stream.readLine();
                    
                    bool match = false;
                    if (m_wholeWord) {
                        QRegularExpression rx(QString("\\b%1\\b").arg(QRegularExpression::escape(m_needle)), 
                            m_matchCase ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
                        match = line.contains(rx);
                    } else {
                        match = line.contains(m_needle, cs);
                    }
                    
                    if (match) {
                        totalMatches++;
                        if (onResultFound) {
                            SearchResultItem item = {filePath, lineNum, line.trimmed()};
                            QMetaObject::invokeMethod(qApp, [this, item](){
                                onResultFound(item);
                            }, Qt::QueuedConnection);
                        }
                    }
                    lineNum++;
                }
            }
        }
        
        if (onSearchFinished) {
            QMetaObject::invokeMethod(qApp, [this, filesScanned, totalMatches](){
                onSearchFinished(filesScanned, totalMatches);
            }, Qt::QueuedConnection);
        }
    }

private:
    QString m_needle;
    QString m_dir;
    QString m_filters;
    bool m_matchCase;
    bool m_wholeWord;
    bool m_subDirs;
    
    QMutex m_mutex;
    bool m_cancel;
};

#endif // GLASS_SEARCH_WORKER_H
