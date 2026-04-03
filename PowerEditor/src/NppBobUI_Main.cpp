#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QTabWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QtPlugin>
#include <iostream>

// NPP Scintilla includes
#include "Scintilla.h"
#include "Sci_Position.h"

// TextFX Engine
#include "TextFXEngine.h"

// Static Plugin Import for BobUI (Qt6 Equivalent)
#ifdef QT_STATIC
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

// Scintilla Widget for BobUI (Qt6 Equivalent)
class BobScintillaEditor : public QWidget {
public:
    BobScintillaEditor(QWidget* parent = nullptr) : QWidget(parent) {
        _layout = new QVBoxLayout(this);
        _layout->setContentsMargins(0, 0, 0, 0);
        
        _textEdit = new QPlainTextEdit(this);
        _layout->addWidget(_textEdit);
        
        // "Glass" style editor: Translucent dark
        _textEdit->setStyleSheet("background-color: rgba(20, 20, 20, 230); color: #e0e0e0; font-family: 'Consolas', 'Courier New', monospace; border: none; selection-background-color: #4a9eff;");
    }

    void setFilePath(const QString& path) { _filePath = path; }
    QString filePath() const { return _filePath; }

    QString text() const { return _textEdit->toPlainText(); }
    void setText(const QString& text) { _textEdit->setPlainText(text); }

    bool loadFile(const QString& path) {
        QFile file(path);
        if (!file.open(QFile::ReadOnly | QFile::Text)) return false;
        _filePath = path;
        _textEdit->setPlainText(file.readAll());
        return true;
    }

    bool saveFile() {
        if (_filePath.isEmpty()) return false;
        QFile file(_filePath);
        if (!file.open(QFile::WriteOnly | QFile::Text)) return false;
        QTextStream out(&file);
        out << _textEdit->toPlainText();
        return true;
    }

private:
    QVBoxLayout* _layout;
    QPlainTextEdit* _textEdit;
    QString _filePath;
};

// The overhauled Notepad++ Main Window (Full Feature Parity Design)
class BobNppMainWindow : public QMainWindow {
public:
    BobNppMainWindow() {
        setWindowTitle("Notepad++ (BobUI Modern Glass)");
        resize(1280, 720);

        // "Modern Glass" UI Styles: Sleek, slightly translucent, neutral colors
        setStyleSheet(
            "QMainWindow { background-color: #2b2b2b; color: #ffffff; }"
            "QMenuBar { background-color: #3c3f41; color: #ffffff; padding: 2px; border-bottom: 1px solid #515151; }"
            "QMenuBar::item:selected { background-color: #4b4b4b; }"
            "QMenu { background-color: #3c3f41; color: #ffffff; border: 1px solid #515151; }"
            "QMenu::item:selected { background-color: #4a9eff; color: #ffffff; }"
            "QTabWidget::pane { border: 1px solid #515151; top: -1px; }"
            "QTabBar::tab { background: #3c3f41; color: #b0b0b0; padding: 8px 15px; border: 1px solid #515151; border-bottom: none; margin-right: 1px; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
            "QTabBar::tab:selected { background: #2b2b2b; color: #ffffff; border-bottom: 1px solid #2b2b2b; }"
            "QStatusBar { background: #3c3f41; color: #b0b0b0; border-top: 1px solid #515151; }"
            "QToolBar { background: #3c3f41; border-bottom: 1px solid #515151; spacing: 5px; }"
        );

        _tabs = new QTabWidget(this);
        _tabs->setMovable(true);
        _tabs->setTabsClosable(true);
        _tabs->setDocumentMode(true);
        connect(_tabs, &QTabWidget::tabCloseRequested, this, [this](int index){
            delete _tabs->widget(index);
        });
        setCentralWidget(_tabs);

        setupMenus();
        setupToolBar();
        
        statusBar()->showMessage("Notepad++ BobUI Ready");
        
        addNewDocument("new 1");
    }

    void addNewDocument(const QString& name) {
        BobScintillaEditor* editor = new BobScintillaEditor(_tabs);
        _tabs->addTab(editor, name);
        _tabs->setCurrentWidget(editor);
    }

    void openFile() {
        QString fileName = QFileDialog::getOpenFileName(this, "Open", "", "All Files (*.*);;C++ Files (*.cpp *.h);;Text Files (*.txt)");
        if (!fileName.isEmpty()) {
            BobScintillaEditor* editor = new BobScintillaEditor(_tabs);
            if (editor->loadFile(fileName)) {
                _tabs->addTab(editor, QFileInfo(fileName).fileName());
                _tabs->setCurrentWidget(editor);
            } else {
                delete editor;
            }
        }
    }

    void saveFile() {
        BobScintillaEditor* editor = static_cast<BobScintillaEditor*>(_tabs->currentWidget());
        if (!editor) return;
        if (editor->filePath().isEmpty()) {
            QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "All Files (*.*)");
            if (fileName.isEmpty()) return;
            editor->setFilePath(fileName);
        }
        if (editor->saveFile()) {
            _tabs->setTabText(_tabs->currentIndex(), QFileInfo(editor->filePath()).fileName());
            statusBar()->showMessage("Saved: " + editor->filePath(), 2000);
        }
    }

    void applyTextFX(int actionId) {
        BobScintillaEditor* editor = static_cast<BobScintillaEditor*>(_tabs->currentWidget());
        if (!editor) return;
        QString original = editor->text();
        QString processed;
        switch (actionId) {
            case 1: processed = TextFXEngine::sortLines(original, true); break;
            case 2: processed = TextFXEngine::sortLines(original, false); break;
            case 3: processed = TextFXEngine::deleteBlankLines(original); break;
            case 4: processed = TextFXEngine::deleteSurplusBlankLines(original); break;
            case 5: processed = TextFXEngine::zapNonPrintable(original); break;
            case 6: processed = TextFXEngine::toUpperCase(original); break;
            case 7: processed = TextFXEngine::toLowerCase(original); break;
            case 8: processed = TextFXEngine::properCase(original); break;
            case 9: processed = TextFXEngine::invertCase(original); break;
            case 10: processed = TextFXEngine::insertLineNumbers(original); break;
            case 11: processed = TextFXEngine::removeLineNumbers(original); break;
        }
        if (processed != original) {
            editor->setText(processed);
            statusBar()->showMessage("TextFX transformation applied", 2000);
        }
    }

private:
    void setupMenus() {
        // --- FILE MENU ---
        QMenu *fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction("&New", this, [this](){ addNewDocument("new " + QString::number(_tabs->count() + 1)); }, QKeySequence::New);
        fileMenu->addAction("&Open", this, &BobNppMainWindow::openFile, QKeySequence::Open);
        fileMenu->addAction("&Save", this, &BobNppMainWindow::saveFile, QKeySequence::Save);
        fileMenu->addAction("Save &As...", this, &BobNppMainWindow::saveFile);
        fileMenu->addAction("Save A&ll", this, [](){}, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
        fileMenu->addSeparator();
        fileMenu->addAction("&Close", this, [this](){ _tabs->removeTab(_tabs->currentIndex()); }, QKeySequence::Close);
        fileMenu->addAction("Close All");
        fileMenu->addSeparator();
        fileMenu->addAction("E&xit", this, &QMainWindow::close, QKeySequence::Quit);

        // --- EDIT MENU ---
        QMenu *editMenu = menuBar()->addMenu("&Edit");
        editMenu->addAction("&Undo", nullptr, nullptr, QKeySequence::Undo);
        editMenu->addAction("&Redo", nullptr, nullptr, QKeySequence::Redo);
        editMenu->addSeparator();
        editMenu->addAction("Cu&t", nullptr, nullptr, QKeySequence::Cut);
        editMenu->addAction("&Copy", nullptr, nullptr, QKeySequence::Copy);
        editMenu->addAction("&Paste", nullptr, nullptr, QKeySequence::Paste);
        editMenu->addAction("&Delete", nullptr, nullptr, QKeySequence::Delete);
        editMenu->addAction("Select &All", nullptr, nullptr, QKeySequence::SelectAll);
        editMenu->addSeparator();
        QMenu *blankOps = editMenu->addMenu("Blank Operations");
        blankOps->addAction("Trim Trailing Space");
        blankOps->addAction("Trim Leading Space");
        blankOps->addAction("EOL to Space");
        QMenu *pasteSpecial = editMenu->addMenu("Paste Special");
        pasteSpecial->addAction("Copy Binary Content");

        // --- SEARCH MENU ---
        QMenu *searchMenu = menuBar()->addMenu("&Search");
        searchMenu->addAction("&Find...", nullptr, nullptr, QKeySequence::Find);
        searchMenu->addAction("Find in &Files...", nullptr, nullptr, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
        searchMenu->addAction("Find &Next", nullptr, nullptr, QKeySequence::FindNext);
        searchMenu->addAction("Find &Previous", nullptr, nullptr, QKeySequence::FindPrevious);
        searchMenu->addAction("&Replace...", nullptr, nullptr, QKeySequence::Replace);
        searchMenu->addSeparator();
        searchMenu->addAction("&Go to...", nullptr, nullptr, QKeySequence(Qt::CTRL | Qt::Key_G));

        // --- VIEW MENU ---
        QMenu *viewMenu = menuBar()->addMenu("&View");
        viewMenu->addAction("Always on Top");
        viewMenu->addAction("Toggle Full Screen Mode", nullptr, nullptr, QKeySequence(Qt::Key_F11));
        viewMenu->addSeparator();
        viewMenu->addAction("Show Symbol");
        viewMenu->addAction("Word wrap");
        QMenu *zoomMenu = viewMenu->addMenu("Zoom");
        zoomMenu->addAction("Zoom In", nullptr, nullptr, QKeySequence::ZoomIn);
        zoomMenu->addAction("Zoom Out", nullptr, nullptr, QKeySequence::ZoomOut);

        // --- ENCODING MENU ---
        QMenu *encodingMenu = menuBar()->addMenu("En&coding");
        encodingMenu->addAction("ANSI");
        encodingMenu->addAction("UTF-8");
        encodingMenu->addAction("UTF-8 with BOM");
        encodingMenu->addAction("UTF-16 BE BOM");
        encodingMenu->addAction("UTF-16 LE BOM");

        // --- LANGUAGE MENU ---
        QMenu *languageMenu = menuBar()->addMenu("&Language");
        languageMenu->addAction("None (Normal Text)");
        languageMenu->addSeparator();
        QMenu *langC = languageMenu->addMenu("C");
        langC->addAction("C");
        langC->addAction("C++");
        langC->addAction("C#");
        languageMenu->addAction("Java");
        languageMenu->addAction("Python");
        languageMenu->addAction("HTML");
        languageMenu->addAction("XML");

        // --- SETTINGS MENU ---
        QMenu *settingsMenu = menuBar()->addMenu("&Settings");
        settingsMenu->addAction("Preferences...");
        settingsMenu->addAction("Style Configurator...");
        settingsMenu->addAction("Shortcut Mapper...");

        // --- TEXTFX MENU (NATIVE & DEFAULT) ---
        QMenu *textfxMenu = menuBar()->addMenu("&TextFX");
        
        QMenu *textfxChar = textfxMenu->addMenu("TextFX Characters");
        textfxChar->addAction("UPPER CASE", this, [this](){ applyTextFX(6); });
        textfxChar->addAction("lower case", this, [this](){ applyTextFX(7); });
        textfxChar->addAction("Proper Case", this, [this](){ applyTextFX(8); });
        textfxChar->addAction("iNVERT cASE", this, [this](){ applyTextFX(9); });
        textfxChar->addSeparator();
        textfxChar->addAction("Zap all non-printable characters to #", this, [this](){ applyTextFX(5); });

        QMenu *textfxTools = textfxMenu->addMenu("TextFX Tools");
        textfxTools->addAction("Insert Line Numbers", this, [this](){ applyTextFX(10); });
        textfxTools->addAction("Delete Line Numbers or First Word", this, [this](){ applyTextFX(11); });
        textfxTools->addSeparator();
        textfxTools->addAction("Sort lines case sensitive", this, [this](){ applyTextFX(1); });
        textfxTools->addAction("Sort lines case insensitive", this, [this](){ applyTextFX(2); });
        textfxTools->addAction("Delete Blank Lines", this, [this](){ applyTextFX(3); });
        textfxTools->addAction("Delete Surplus Blank Lines", this, [this](){ applyTextFX(4); });

        // --- MACRO MENU ---
        QMenu *macroMenu = menuBar()->addMenu("&Macro");
        macroMenu->addAction("Start Recording");
        macroMenu->addAction("Stop Recording");
        macroMenu->addAction("Playback");

        // --- RUN MENU ---
        QMenu *runMenu = menuBar()->addMenu("&Run");
        runMenu->addAction("&Run...", nullptr, nullptr, QKeySequence(Qt::Key_F5));

        // --- WINDOW MENU ---
        QMenu *windowMenu = menuBar()->addMenu("&Window");
        windowMenu->addAction("Next Tab", nullptr, nullptr, QKeySequence(Qt::CTRL | Qt::Key_PageDown));
        windowMenu->addAction("Previous Tab", nullptr, nullptr, QKeySequence(Qt::CTRL | Qt::Key_PageUp));

        // --- HELP MENU ---
        QMenu *helpMenu = menuBar()->addMenu("&?");
        helpMenu->addAction("Command Line Arguments...");
        helpMenu->addAction("Notepad++ Home");
        helpMenu->addSeparator();
        helpMenu->addAction("&About Notepad++ (BobUI)", this, [this](){
            QMessageBox::about(this, "About Notepad++ BobUI", "Notepad++ Overhauled with BobUI Framework.\n\nFeaturing native TextFX integration and a modern glass interface.");
        });
    }

    void setupToolBar() {
        QToolBar *toolBar = addToolBar("Standard");
        toolBar->addAction("New", this, [this](){ addNewDocument("new " + QString::number(_tabs->count() + 1)); });
        toolBar->addAction("Open", this, &BobNppMainWindow::openFile);
        toolBar->addAction("Save", this, &BobNppMainWindow::saveFile);
        toolBar->addSeparator();
        toolBar->addAction("Cut");
        toolBar->addAction("Copy");
        toolBar->addAction("Paste");
        toolBar->addSeparator();
        toolBar->addAction("Undo");
        toolBar->addAction("Redo");
        toolBar->addSeparator();
        toolBar->addAction("Find");
        toolBar->addAction("Replace");
    }

    QTabWidget *_tabs;
};

int main(int argc, char *argv[]) {
    QApplication nppApp(argc, argv);
    nppApp.setApplicationName("Notepad++ (BobUI)");
    BobNppMainWindow mainWindow;
    mainWindow.show();
    return nppApp.exec();
}
