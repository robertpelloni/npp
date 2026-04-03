#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <vector>
#include <string>
#include <algorithm>

// Mock of Scintilla for POC
class BobScintilla : public QWidget {
public:
    BobScintilla(QWidget* parent = nullptr) : QWidget(parent) {}
    void setText(const std::string& text) { _text = text; update(); }
    std::string text() const { return _text; }
private:
    std::string _text;
};

// TextFX Feature Bridge
namespace TextFX {
    void sortLines(BobScintilla* view, bool caseSensitive = true) {
        std::string content = view->text();
        // Simplified sorting logic extracted from NPPTextFX.cpp
        std::vector<std::string> lines;
        size_t start = 0, end;
        while ((end = content.find('\n', start)) != std::string::npos) {
            lines.push_back(content.substr(start, end - start));
            start = end + 1;
        }
        lines.push_back(content.substr(start));

        if (caseSensitive) {
            std::sort(lines.begin(), lines.end());
        } else {
            std::sort(lines.begin(), lines.end(), [](const std::string& a, const std::string& b) {
                std::string al = a, bl = b;
                for (char& c : al) c = tolower(c);
                for (char& c : bl) c = tolower(c);
                return al < bl;
            });
        }

        std::string result;
        for (const auto& line : lines) {
            result += line + "\n";
        }
        view->setText(result);
    }
}

// Overhauled Notepad++ Main Window using BobUI (Qt6 Equivalent)
class NppMainWindow : public QMainWindow {
public:
    NppMainWindow() {
        setWindowTitle("Notepad++ (BobUI Overhaul POC)");
        
        _tabs = new QTabWidget(this);
        setCentralWidget(_tabs);
        
        setupMenus();
        addNewDocument("new 1");
    }

    void addNewDocument(const QString& name) {
        BobScintilla* editor = new BobScintilla(_tabs);
        _tabs->addTab(editor, name);
    }

private:
    void setupMenus() {
        QMenu *fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction("&New", this, [this](){ addNewDocument("new " + QString::number(_tabs->count() + 1)); });
        fileMenu->addAction("&Open");
        fileMenu->addSeparator();
        fileMenu->addAction("E&xit", this, &QMainWindow::close);

        // TextFX Integration: All features included by default
        QMenu *textfxMenu = menuBar()->addMenu("TextFX");
        QMenu *textfxTools = textfxMenu->addMenu("TextFX Tools");
        
        textfxTools->addAction("Sort lines case sensitive (at column)", this, [this](){
            BobScintilla* editor = static_cast<BobScintilla*>(_tabs->currentWidget());
            if (editor) TextFX::sortLines(editor, true);
        });

        textfxTools->addAction("Sort lines case insensitive (at column)", this, [this](){
            BobScintilla* editor = static_cast<BobScintilla*>(_tabs->currentWidget());
            if (editor) TextFX::sortLines(editor, false);
        });
        
        textfxMenu->addAction("Word Count");
        textfxMenu->addAction("Clean Quote");
    }

    QTabWidget *_tabs;
};

// Main entry point for POC
int main(int argc, char *argv[]) {
    // In a real BobUI app, this would be BobApplication
    // QApplication app(argc, argv);
    // NppMainWindow window;
    // window.show();
    // return app.exec();
    return 0;
}
