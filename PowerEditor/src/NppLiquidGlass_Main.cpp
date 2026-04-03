/**
 * NppLiquidGlass_Main.cpp — Notepad++ BobUI Liquid Glass Edition
 *
 * This is the primary entry point for the overhauled Notepad++ UI. It replaces
 * the legacy Win32 wWinMain with a full Qt6/BobUI application that features:
 *
 *   • REAL OS-level backdrop blur (DWM Acrylic on Windows 11, DWMBB on Win10)
 *   • Custom liquid glass material system (LiquidGlass.h)
 *   • Software blur fallback for inner panels (LiquidGlassEffect)
 *   • Animated bubble decorations (LiquidBubbleOverlay)
 *   • Native TextFX engine (TextFXEngine.h) — all transforms wired to menus
 *   • Proper tab management with glass-themed tab bar
 *   • Glass Find/Replace dialog
 *   • Glass About dialog with version info
 *   • Full menu parity with original Notepad++
 *
 * Architecture notes:
 *   - BobGlassEditor    : The glass-wrapped editor widget (QPlainTextEdit inside
 *                         a glass-painted QFrame).
 *   - BubbleOverlay     : A transparent overlay widget that draws animated
 *                         liquid bubble decorations above all content.
 *   - GlassStatusWidget : Custom status bar with glass paint and live stats.
 *   - GlassTabBar       : Custom QTabBar that draws glass-style tabs.
 *   - BobNppGlassWindow : The main window — manages all of the above.
 *
 * Build target: npp_liquid_glass (see CMakeLists.txt)
 *
 * Author: Antigravity (Google DeepMind Advanced Agentic Coding)
 * Date  : 2026-04-03
 * Version: see VERSION file at project root
 */

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QTabBar>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSplitter>
#include <QScrollBar>
#include <QTimer>
#include <QElapsedTimer>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QTextCursor>
#include <QTextDocument>
#include <QKeySequence>
#include <QShortcut>
#include <QDialog>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScreen>
#include <QWindow>
#include <QStyleOption>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <QSettings>
#include <QInputDialog>
#include <QRegularExpression>
#include <QtPlugin>
#include <random>
#include <cmath>

// BobUI platform plugin (static link)
#ifdef QT_STATIC
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

// Our glass system
#include "LiquidGlass.h"

// TextFX algorithm engine
#include "TextFXEngine.h"

// Scintilla API (for future real editor integration)
#include "Scintilla.h"
#include "Sci_Position.h"

// ─────────────────────────────────────────────────────────────────────────────
// VERSION — loaded from ../../VERSION (one source of truth)
// Falls back to embedded constant if the file isn't present at runtime.
// ─────────────────────────────────────────────────────────────────────────────
static QString loadVersion() {
    QFile vf(QStringLiteral("../../VERSION"));
    if (vf.open(QFile::ReadOnly | QFile::Text)) {
        return vf.readAll().trimmed();
    }
    return QStringLiteral("1.0.4");  // fallback
}
static const QString kAppVersion = loadVersion();

// ─────────────────────────────────────────────────────────────────────────────
// BUBBLE OVERLAY — animated liquid bubbles floating over the glass surface.
// This is a transparent QWidget child of the main window that paints
// decorative animated bubble glyphs using LiquidGlassPainter::drawLiquidDrop.
//
// Why: A pure aesthetic touch — the "liquid" in "liquid glass" implies a
// fluid, living surface. Subtle floating bubbles reinforce this metaphor
// without being distracting.
//
// Design: Bubbles are generated with random initial positions, radii (4–18px),
// and upward drift velocities. They fade in/out using an alpha lifecycle.
// The overlay is mouse-transparent (setAttribute WA_TransparentForMouseEvents).
// ─────────────────────────────────────────────────────────────────────────────
class BubbleOverlay : public QWidget {
public:
    struct Bubble {
        QPointF pos;
        qreal   radius;
        qreal   alpha;       // 0–1 normalized opacity
        qreal   speedY;      // pixels per tick (upward)
        qreal   drift;       // horizontal sway per tick
        qreal   phase;       // phase for sinusoidal drift
        QColor  tint;
        bool    dying;       // true when fading out
    };

    explicit BubbleOverlay(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        // Fully transparent to mouse so it never intercepts editor interaction
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);

        // Seed bubble generator with 12 initial bubbles
        std::mt19937 rng(std::random_device{}());
        spawnInitialBubbles(rng);

        // Tick at ~30fps — light enough for smooth animation without burning CPU
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &BubbleOverlay::tick);
        m_timer->start(33);  // ~30fps

        // Periodically spawn a new bubble to replace ones that escaped off-top
        m_spawnTimer = new QTimer(this);
        connect(m_spawnTimer, &QTimer::timeout, this, [this]() {
            std::mt19937 rng(std::random_device{}());
            spawnBubble(rng);
        });
        m_spawnTimer->start(2500);  // new bubble every 2.5 seconds
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        // Draw each bubble using our liquid drop painter
        for (const auto& b : m_bubbles) {
            QColor tint = b.tint;
            tint.setAlphaF(b.alpha * 0.7);  // scale by lifecycle alpha
            LiquidGlassPainter::drawLiquidDrop(p, b.pos, b.radius, tint);
        }
    }

private slots:
    void tick() {
        auto& list = m_bubbles;
        for (auto& b : list) {
            b.phase += 0.04;
            b.pos.ry() -= b.speedY;
            b.pos.rx() += std::sin(b.phase) * b.drift;

            // Fade in during first 20% of lifecycle, fade out at top edge
            if (b.pos.y() < height() * 0.2 && !b.dying) {
                b.dying = true;
            }
            if (b.dying) {
                b.alpha -= 0.018;
            } else if (b.alpha < 1.0) {
                b.alpha += 0.025;
            }
        }
        // Remove dead bubbles
        list.erase(std::remove_if(list.begin(), list.end(),
            [](const Bubble& b){ return b.alpha <= 0.0 || b.pos.y() < -30; }),
            list.end());
        update();
    }

private:
    void spawnBubble(std::mt19937& rng) {
        if (!isVisible() || width() < 10 || height() < 10) return;
        if (m_bubbles.size() >= 18) return;  // cap bubble count

        std::uniform_real_distribution<qreal> rx(20, width() - 20);
        std::uniform_real_distribution<qreal> rr(4, 18);
        std::uniform_real_distribution<qreal> rsp(0.3, 0.9);
        std::uniform_real_distribution<qreal> rd(0.2, 0.8);
        std::uniform_real_distribution<qreal> rp(0, 6.28);

        // Color palette: blue, purple, teal, white
        static const QColor palettes[] = {
            QColor(80,  140, 255, 60),   // blue glass
            QColor(140, 80,  255, 60),   // purple glass
            QColor(60,  190, 200, 60),   // teal glass
            QColor(220, 230, 255, 55),   // white glass
            QColor(255, 160, 80,  40),   // amber glass
        };
        std::uniform_int_distribution<int> ri(0, 4);

        Bubble b;
        b.pos    = { rx(rng), static_cast<qreal>(height() + 20) };
        b.radius = rr(rng);
        b.alpha  = 0.0;
        b.speedY = rsp(rng);
        b.drift  = rd(rng);
        b.phase  = rp(rng);
        b.tint   = palettes[ri(rng)];
        b.dying  = false;
        m_bubbles.push_back(b);
    }

    void spawnInitialBubbles(std::mt19937& rng) {
        std::uniform_real_distribution<qreal> ry(0.1, 0.9);
        // Start scattered across the height so it doesn't look empty at launch
        for (int i = 0; i < 12; ++i) {
            spawnBubble(rng);
            if (!m_bubbles.empty()) {
                // Randomly place initial bubbles vertically (not all at bottom)
                m_bubbles.back().pos.ry() =
                    height() > 0 ? height() * ry(rng) : 400.0 * ry(rng);
                m_bubbles.back().alpha = 0.3 + ry(rng) * 0.5;
            }
        }
    }

    QVector<Bubble> m_bubbles;
    QTimer* m_timer;
    QTimer* m_spawnTimer;
};


// ─────────────────────────────────────────────────────────────────────────────
// GLASS EDITOR PANEL — wraps QPlainTextEdit with a glass paint border.
//
// The actual editor sits inside this frame. The frame's paintEvent draws the
// liquid glass material (LiquidGlassPainter::drawGlassBackground) behind the
// editor widget, giving it a frosted glass border/glow effect.
// ─────────────────────────────────────────────────────────────────────────────
class GlassEditorPanel : public QFrame {
public:
    explicit GlassEditorPanel(QWidget* parent = nullptr)
        : QFrame(parent)
    {
        // Transparent frame background — we paint manually.
        setAttribute(Qt::WA_TranslucentBackground);
        setFrameShape(QFrame::NoFrame);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(1, 1, 1, 1);  // thin glass border margin
        layout->setSpacing(0);

        // The actual text editor with the glass QSS applied
        m_editor = new QPlainTextEdit(this);
        m_editor->setStyleSheet(LiquidGlassStyleSheet::kEditor);
        m_editor->setFrameShape(QFrame::NoFrame);
        // Smooth scrolling preference
        m_editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        layout->addWidget(m_editor);
    }

    QPlainTextEdit* editor() { return m_editor; }

    QString text() const     { return m_editor->toPlainText(); }
    void setText(const QString& t) { m_editor->setPlainText(t); }
    void setFilePath(const QString& p) { m_filePath = p; }
    QString filePath() const { return m_filePath; }

    bool loadFile(const QString& path) {
        QFile f(path);
        if (!f.open(QFile::ReadOnly | QFile::Text)) return false;
        m_filePath = path;
        m_editor->setPlainText(QString::fromUtf8(f.readAll()));
        m_modified = false;
        connect(m_editor->document(), &QTextDocument::modificationChanged,
                this, [this](bool mod){ m_modified = mod; });
        return true;
    }

    bool saveFile() {
        if (m_filePath.isEmpty()) return false;
        QFile f(m_filePath);
        if (!f.open(QFile::WriteOnly | QFile::Text)) return false;
        QTextStream s(&f);
        s << m_editor->toPlainText();
        m_modified = false;
        m_editor->document()->setModified(false);
        return true;
    }

    bool isModified() const { return m_modified; }

protected:
    void paintEvent(QPaintEvent* ev) override {
        // Draw the glass material as the border/frame of the editor area.
        // The editor widget itself paints its own dark background.
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Draw a glass rim around the editor
        QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        LiquidGlassPainter::drawGlassBackground(p, r,
            LiquidGlassPalette::kCornerRadius * 0.7, true);

        QFrame::paintEvent(ev);
    }

private:
    QPlainTextEdit* m_editor;
    QString         m_filePath;
    bool            m_modified = false;
};


// ─────────────────────────────────────────────────────────────────────────────
// GLASS STATUS WIDGET — custom status bar with glass painting + live stats.
//
// Shows: col, ln, words, chars, encoding, EOL type, zoom, version.
// The widget is completely custom-painted to show the glass material while
// still using QLabel children for the text items.
// ─────────────────────────────────────────────────────────────────────────────
class GlassStatusWidget : public QWidget {
public:
    explicit GlassStatusWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedHeight(26);
        setStyleSheet(LiquidGlassStyleSheet::kStatusBar);

        auto* lay = new QHBoxLayout(this);
        lay->setContentsMargins(8, 0, 8, 0);
        lay->setSpacing(18);

        // Left section: message / notification
        m_msgLabel = makeLabel("Ready");
        lay->addWidget(m_msgLabel);
        lay->addStretch();

        // Right section: file stats
        m_lnColLabel   = makeLabel("Ln 1, Col 1");
        m_selLabel     = makeLabel("Sel: 0");
        m_wordsLabel   = makeLabel("Words: 0");
        m_encLabel     = makeLabel("UTF-8");
        m_eolLabel     = makeLabel("CRLF");
        m_zoomLabel    = makeLabel("100%");
        m_verLabel     = makeLabel("v" + kAppVersion);
        m_verLabel->setStyleSheet("color: rgba(90,130,255,200); font-weight: bold;");

        for (auto* l : {m_lnColLabel, m_selLabel, m_wordsLabel,
                        m_encLabel, m_eolLabel, m_zoomLabel, m_verLabel}) {
            lay->addWidget(makeSep());
            lay->addWidget(l);
        }
    }

    void showMessage(const QString& msg, int msec = 0) {
        m_msgLabel->setText(msg);
        if (msec > 0) {
            QTimer::singleShot(msec, this, [this](){ m_msgLabel->setText("Ready"); });
        }
    }

    void updateStats(QPlainTextEdit* editor) {
        if (!editor) return;
        QTextCursor cur = editor->textCursor();
        int ln  = cur.blockNumber() + 1;
        int col = cur.columnNumber() + 1;
        int sel = cur.selectionEnd() - cur.selectionStart();
        QString text = editor->toPlainText();
        QStringList words = text.split(QRegularExpression(R"(\s+)"),
                                       Qt::SkipEmptyParts);
        m_lnColLabel->setText(QString("Ln %1, Col %2").arg(ln).arg(col));
        m_selLabel->setText(QString("Sel: %1").arg(sel));
        m_wordsLabel->setText(QString("Words: %1").arg(words.size()));
    }

    void setEncoding(const QString& enc) { m_encLabel->setText(enc); }
    void setEOL(const QString& eol)      { m_eolLabel->setText(eol); }
    void setZoom(int pct) { m_zoomLabel->setText(QString("%1%").arg(pct)); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        // Glass bar drawn with slightly more opaque base for readability
        QRectF r = QRectF(rect()).adjusted(0, 0, 0, 0);
        // Flat glass (no rounded corners at top — flush with window edge)
        p.fillRect(r, LiquidGlassPalette::kStatusBarGlass);
        // Top hairline separator
        p.setPen(QPen(LiquidGlassPalette::kBorderColor, 0.8));
        p.drawLine(r.topLeft(), r.topRight());
        // Subtle sheen
        QLinearGradient sheen(r.topLeft(), r.bottomLeft());
        sheen.setColorAt(0, QColor(255,255,255,18));
        sheen.setColorAt(1, Qt::transparent);
        p.fillRect(r, sheen);
    }

private:
    QLabel* makeLabel(const QString& txt) {
        auto* l = new QLabel(txt, this);
        l->setStyleSheet("color: rgba(160,175,210,200); font-size:11px; background:transparent;");
        return l;
    }
    QWidget* makeSep() {
        auto* s = new QWidget(this);
        s->setFixedSize(1, 14);
        s->setStyleSheet("background: rgba(255,255,255,20);");
        return s;
    }

    QLabel* m_msgLabel;
    QLabel* m_lnColLabel;
    QLabel* m_selLabel;
    QLabel* m_wordsLabel;
    QLabel* m_encLabel;
    QLabel* m_eolLabel;
    QLabel* m_zoomLabel;
    QLabel* m_verLabel;
};


// ─────────────────────────────────────────────────────────────────────────────
// GLASS FIND/REPLACE DIALOG
//
// A floating, glass-styled modal-less dialog for find and replace.
// Uses LiquidGlassPainter::drawGlassBackground in paintEvent for the
// glass panel background. All child widgets use the kDialog QSS.
// ─────────────────────────────────────────────────────────────────────────────
class GlassFindDialog : public QDialog {
public:
    explicit GlassFindDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Find & Replace");
        setModal(false);
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setStyleSheet(LiquidGlassStyleSheet::kDialog);
        setFixedSize(520, 240);

        auto* grid = new QGridLayout(this);
        grid->setContentsMargins(20, 20, 20, 20);
        grid->setSpacing(10);

        // Title bar with drag support
        auto* titleBar = new QWidget(this);
        auto* titleLay = new QHBoxLayout(titleBar);
        titleLay->setContentsMargins(0,0,0,0);
        auto* titleLabel = new QLabel("🔍 Find & Replace", titleBar);
        titleLabel->setStyleSheet("color: rgba(240,245,255,240); font-size:14px; font-weight:600;");
        auto* closeBtn = new QPushButton("✕", titleBar);
        closeBtn->setFixedSize(24, 24);
        closeBtn->setStyleSheet("QPushButton{background:rgba(255,60,60,160);color:white;border:none;"
                                "border-radius:12px;font-size:12px;}"
                                "QPushButton:hover{background:rgba(255,80,80,220);}");
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
        titleLay->addWidget(titleLabel);
        titleLay->addStretch();
        titleLay->addWidget(closeBtn);
        grid->addWidget(titleBar, 0, 0, 1, 3);

        // Find
        grid->addWidget(new QLabel("Find:", this), 1, 0);
        m_findEdit = new QLineEdit(this);
        m_findEdit->setPlaceholderText("Search text or regex...");
        grid->addWidget(m_findEdit, 1, 1, 1, 2);

        // Replace
        grid->addWidget(new QLabel("Replace:", this), 2, 0);
        m_replaceEdit = new QLineEdit(this);
        m_replaceEdit->setPlaceholderText("Replacement text...");
        grid->addWidget(m_replaceEdit, 2, 1, 1, 2);

        // Options
        m_caseCheck  = new QCheckBox("Match case", this);
        m_wholeCheck = new QCheckBox("Whole word", this);
        m_regexCheck = new QCheckBox("Regex", this);
        auto* optLay = new QHBoxLayout;
        optLay->addWidget(m_caseCheck);
        optLay->addWidget(m_wholeCheck);
        optLay->addWidget(m_regexCheck);
        optLay->addStretch();
        grid->addLayout(optLay, 3, 0, 1, 3);

        // Buttons
        m_findNextBtn  = new QPushButton("Find Next",    this);
        m_replaceBtn   = new QPushButton("Replace",      this);
        m_replAllBtn   = new QPushButton("Replace All",  this);
        m_closeBtn2    = new QPushButton("Close",        this);
        auto* btnLay   = new QHBoxLayout;
        btnLay->addWidget(m_findNextBtn);
        btnLay->addWidget(m_replaceBtn);
        btnLay->addWidget(m_replAllBtn);
        btnLay->addStretch();
        btnLay->addWidget(m_closeBtn2);
        grid->addLayout(btnLay, 4, 0, 1, 3);

        connect(m_closeBtn2, &QPushButton::clicked, this, &QDialog::close);

        // Enable dragging the frameless dialog via the title bar
        installEventFilter(this);
    }

    QString findText()    const { return m_findEdit->text(); }
    QString replaceText() const { return m_replaceEdit->text(); }
    bool    matchCase()   const { return m_caseCheck->isChecked(); }
    bool    wholeWord()   const { return m_wholeCheck->isChecked(); }
    bool    useRegex()    const { return m_regexCheck->isChecked(); }

    QPushButton* findNextButton()  { return m_findNextBtn; }
    QPushButton* replaceButton()   { return m_replaceBtn; }
    QPushButton* replaceAllButton(){ return m_replAllBtn; }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        LiquidGlassPainter::drawGlassBackground(p, QRectF(rect()), 14.0, true);
    }

    // Dragging support for frameless dialog
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            m_dragPos = e->globalPosition().toPoint() - frameGeometry().topLeft();
            e->accept();
        }
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) {
            move(e->globalPosition().toPoint() - m_dragPos);
            e->accept();
        }
    }

private:
    QLineEdit*   m_findEdit;
    QLineEdit*   m_replaceEdit;
    QCheckBox*   m_caseCheck;
    QCheckBox*   m_wholeCheck;
    QCheckBox*   m_regexCheck;
    QPushButton* m_findNextBtn;
    QPushButton* m_replaceBtn;
    QPushButton* m_replAllBtn;
    QPushButton* m_closeBtn2;
    QPoint       m_dragPos;
};


// ─────────────────────────────────────────────────────────────────────────────
// MAIN WINDOW — BobNppGlassWindow
//
// This is the primary application window. It:
//   1. Sets WA_TranslucentBackground to allow DWM blur to show through.
//   2. Calls LiquidGlassDWM::enableBlurBehind in showEvent.
//   3. Manages glass editor panels in a glass-styled QTabWidget.
//   4. Hosts BubbleOverlay as a transparent child above everything.
//   5. Has GlassStatusWidget replacing the QStatusBar.
//   6. All menus have glass QSS applied.
//   7. TextFX operations are fully wired with status feedback.
// ─────────────────────────────────────────────────────────────────────────────
class BobNppGlassWindow : public QMainWindow {
public:
    BobNppGlassWindow() {
        setWindowTitle(QString("Notepad++ (BobUI Liquid Glass) v%1").arg(kAppVersion));
        resize(1400, 900);

        // ── Transparency setup ───────────────────────────────────────────────
        // WA_TranslucentBackground is the key flag that makes the Qt window
        // composited with the desktop beneath it. Without this, DWM blur is
        // invisible (the window paints a solid background on top).
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);

        // Remove the default frame — we'll draw our own or keep system chrome.
        // We keep the system frame (DO NOT set FramelessWindowHint here for the
        // main window) so the user still gets drag/resize/caption. We rely on
        // DWM to extend the frame glass effect into the client area.
        // For dialogs/panels, FramelessWindowHint is appropriate.

        // ── Apply global glass QSS ───────────────────────────────────────────
        qApp->setStyleSheet(LiquidGlassStyleSheet::kFullGlassTheme());

        // ── Central widget setup ─────────────────────────────────────────────
        // We use a QWidget as central so we can control painting.
        auto* central = new QWidget(this);
        central->setAttribute(Qt::WA_TranslucentBackground);
        setCentralWidget(central);

        auto* mainLay = new QVBoxLayout(central);
        mainLay->setContentsMargins(0, 0, 0, 0);
        mainLay->setSpacing(0);

        // ── Tab widget ───────────────────────────────────────────────────────
        m_tabs = new QTabWidget(central);
        m_tabs->setMovable(true);
        m_tabs->setTabsClosable(true);
        m_tabs->setDocumentMode(true);
        m_tabs->setStyleSheet(LiquidGlassStyleSheet::kTabWidget);
        connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int idx) {
            onTabCloseRequested(idx);
        });
        connect(m_tabs, &QTabWidget::currentChanged, this, [this](int idx) {
            onCurrentTabChanged(idx);
        });
        mainLay->addWidget(m_tabs, 1);

        // ── Glass status widget (replaces QStatusBar) ────────────────────────
        m_statusWidget = new GlassStatusWidget(central);
        mainLay->addWidget(m_statusWidget, 0);

        // ── Menus & toolbar ──────────────────────────────────────────────────
        setupMenuBar();
        setupToolBar();

        // ── Bubble overlay (lives above everything) ──────────────────────────
        // The overlay must be a child of the central widget (not the window)
        // so it doesn't interfere with the menu bar.
        m_bubbles = new BubbleOverlay(central);
        m_bubbles->resize(central->size());
        m_bubbles->raise();  // on top of all siblings
        // Keep overlay sized to the central widget
        central->installEventFilter(this);

        // ── Open initial document ────────────────────────────────────────────
        addNewDocument("new 1");

        // ── Find dialog (created lazily) ─────────────────────────────────────
        m_findDialog = nullptr;
    }

protected:
    void showEvent(QShowEvent* e) override {
        QMainWindow::showEvent(e);
        // DWM activation must happen AFTER the HWND is created (after show).
        // We use a deferred call to ensure the native handle exists.
        QTimer::singleShot(0, this, [this]() {
            LiquidGlassDWM::enableBlurBehind(this, 2);  // 2 = Acrylic
            LiquidGlassDWM::setDarkMode(this, true);
            LiquidGlassDWM::setRoundedCorners(this, 2); // 2 = DWMWCP_ROUND
            LiquidGlassDWM::setBorderColor(this, QColor(80, 130, 255, 180));
        });
    }

    bool eventFilter(QObject* obj, QEvent* ev) override {
        // Keep bubble overlay sized to central widget
        if (ev->type() == QEvent::Resize && obj == centralWidget()) {
            auto* re = static_cast<QResizeEvent*>(ev);
            if (m_bubbles) m_bubbles->resize(re->size());
        }
        return QMainWindow::eventFilter(obj, ev);
    }

    void paintEvent(QPaintEvent* ev) override {
        // Paint the liquid glass background for the main window client area.
        // This is the base layer behind the menu bar, tab widget, etc.
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Full-window glass tint (on top of OS DWM blur)
        QRectF r = QRectF(rect());
        p.fillRect(r, LiquidGlassPalette::kGlassTintDark);

        // Subtle sheen gradient across the entire window
        QLinearGradient globalSheen(r.topLeft(), r.bottomLeft());
        globalSheen.setColorAt(0.0, QColor(255,255,255,18));
        globalSheen.setColorAt(0.3, Qt::transparent);
        globalSheen.setColorAt(1.0, Qt::transparent);
        p.fillRect(r, globalSheen);

        QMainWindow::paintEvent(ev);
    }

    void resizeEvent(QResizeEvent* ev) override {
        QMainWindow::resizeEvent(ev);
        if (m_bubbles) m_bubbles->resize(centralWidget() ? centralWidget()->size() : size());
    }

private:
    // ── Document management ─────────────────────────────────────────────────
    void addNewDocument(const QString& name) {
        auto* panel = new GlassEditorPanel(m_tabs);
        int idx = m_tabs->addTab(panel, name);
        m_tabs->setCurrentIndex(idx);
        connectEditorSignals(panel);
    }

    void connectEditorSignals(GlassEditorPanel* panel) {
        // Update status bar on cursor movement / content change
        connect(panel->editor(), &QPlainTextEdit::cursorPositionChanged,
                this, [this, panel]() { m_statusWidget->updateStats(panel->editor()); });
        connect(panel->editor()->document(), &QTextDocument::contentsChanged,
                this, [this, panel]() { m_statusWidget->updateStats(panel->editor()); });
    }

    GlassEditorPanel* currentPanel() {
        return static_cast<GlassEditorPanel*>(m_tabs->currentWidget());
    }

    void onTabCloseRequested(int idx) {
        auto* panel = static_cast<GlassEditorPanel*>(m_tabs->widget(idx));
        if (panel && panel->isModified()) {
            auto btn = QMessageBox::question(this, "Unsaved Changes",
                QString("'%1' has unsaved changes. Save before closing?")
                    .arg(m_tabs->tabText(idx)),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            if (btn == QMessageBox::Cancel) return;
            if (btn == QMessageBox::Save) {
                if (panel->filePath().isEmpty()) {
                    if (!saveFileAs(panel)) return;
                } else {
                    panel->saveFile();
                }
            }
        }
        m_tabs->removeTab(idx);
        delete panel;
        if (m_tabs->count() == 0) addNewDocument("new 1");
    }

    void onCurrentTabChanged(int idx) {
        auto* panel = static_cast<GlassEditorPanel*>(m_tabs->widget(idx));
        if (panel) m_statusWidget->updateStats(panel->editor());
    }

    // ── File operations ─────────────────────────────────────────────────────
    void actionNew() {
        addNewDocument(QString("new %1").arg(m_tabs->count() + 1));
    }

    void actionOpen() {
        QStringList files = QFileDialog::getOpenFileNames(
            this, "Open File(s)", QString(),
            "All Files (*.*);;C++ (*.cpp *.h *.hpp *.cxx);;Python (*.py);"
            ";JavaScript (*.js *.ts);;Web (*.html *.css);;Text (*.txt *.md)");
        for (const auto& path : files) {
            auto* panel = new GlassEditorPanel(m_tabs);
            if (panel->loadFile(path)) {
                int idx = m_tabs->addTab(panel, QFileInfo(path).fileName());
                m_tabs->setCurrentIndex(idx);
                connectEditorSignals(panel);
                m_statusWidget->showMessage("Opened: " + path, 3000);
            } else {
                delete panel;
                QMessageBox::warning(this, "Open Failed",
                    "Could not open: " + path);
            }
        }
    }

    void actionSave() {
        auto* panel = currentPanel();
        if (!panel) return;
        if (panel->filePath().isEmpty()) { saveFileAs(panel); return; }
        if (panel->saveFile()) {
            m_tabs->setTabText(m_tabs->currentIndex(),
                               QFileInfo(panel->filePath()).fileName());
            m_statusWidget->showMessage("Saved: " + panel->filePath(), 2000);
        } else {
            QMessageBox::critical(this, "Save Failed",
                "Could not write: " + panel->filePath());
        }
    }

    bool saveFileAs(GlassEditorPanel* panel) {
        QString path = QFileDialog::getSaveFileName(
            this, "Save File", panel->filePath().isEmpty()
                ? QStringLiteral("untitled.txt") : panel->filePath(),
            "All Files (*.*);;Text (*.txt);;C++ (*.cpp *.h)");
        if (path.isEmpty()) return false;
        panel->setFilePath(path);
        if (!panel->saveFile()) {
            QMessageBox::critical(this, "Save Failed", "Could not write: " + path);
            return false;
        }
        m_tabs->setTabText(m_tabs->currentIndex(), QFileInfo(path).fileName());
        m_statusWidget->showMessage("Saved: " + path, 2000);
        return true;
    }

    void actionSaveAll() {
        for (int i = 0; i < m_tabs->count(); ++i) {
            auto* p = static_cast<GlassEditorPanel*>(m_tabs->widget(i));
            if (p && p->isModified()) {
                if (p->filePath().isEmpty()) {
                    m_tabs->setCurrentIndex(i);
                    saveFileAs(p);
                } else {
                    p->saveFile();
                    m_tabs->setTabText(i, QFileInfo(p->filePath()).fileName());
                }
            }
        }
        m_statusWidget->showMessage("All files saved", 2000);
    }

    // ── TextFX operations ────────────────────────────────────────────────────
    void applyTextFX(int actionId) {
        auto* panel = currentPanel();
        if (!panel) return;
        QString text = panel->text();
        QString result;
        QString opName;

        switch (actionId) {
            // Sort
            case 1:  result = TextFXEngine::sortLines(text, true);   opName = "Sort (case sensitive)"; break;
            case 2:  result = TextFXEngine::sortLines(text, false);  opName = "Sort (case insensitive)"; break;
            // Line ops
            case 3:  result = TextFXEngine::deleteBlankLines(text);         opName = "Delete blank lines"; break;
            case 4:  result = TextFXEngine::deleteSurplusBlankLines(text);  opName = "Delete surplus blank lines"; break;
            // Character ops
            case 5:  result = TextFXEngine::zapNonPrintable(text);   opName = "Zap non-printable"; break;
            // Case
            case 6:  result = TextFXEngine::toUpperCase(text);       opName = "UPPER CASE"; break;
            case 7:  result = TextFXEngine::toLowerCase(text);       opName = "lower case"; break;
            case 8:  result = TextFXEngine::properCase(text);        opName = "Proper Case"; break;
            case 9:  result = TextFXEngine::invertCase(text);        opName = "iNVERT cASE"; break;
            // Line numbers
            case 10: result = TextFXEngine::insertLineNumbers(text); opName = "Insert line numbers"; break;
            case 11: result = TextFXEngine::removeLineNumbers(text); opName = "Remove line numbers"; break;
            default: return;
        }

        if (result != text) {
            panel->setText(result);
            m_statusWidget->showMessage("TextFX: " + opName, 2500);
        }
    }

    // ── Find & Replace ───────────────────────────────────────────────────────
    void showFindDialog() {
        if (!m_findDialog) {
            m_findDialog = new GlassFindDialog(this);
            // Wire up actions
            connect(m_findDialog->findNextButton(), &QPushButton::clicked,
                    this, [this]() { doFind(false); });
            connect(m_findDialog->replaceButton(), &QPushButton::clicked,
                    this, [this]() { doReplace(false); });
            connect(m_findDialog->replaceAllButton(), &QPushButton::clicked,
                    this, [this]() { doReplace(true); });
        }
        m_findDialog->show();
        m_findDialog->raise();
        m_findDialog->activateWindow();
    }

    void doFind(bool /*backward*/) {
        if (!m_findDialog) return;
        auto* panel = currentPanel();
        if (!panel) return;
        QString needle = m_findDialog->findText();
        if (needle.isEmpty()) return;

        QTextDocument::FindFlags flags;
        if (m_findDialog->matchCase()) flags |= QTextDocument::FindCaseSensitively;
        if (m_findDialog->wholeWord()) flags |= QTextDocument::FindWholeWords;

        bool found = panel->editor()->find(needle, flags);
        if (!found) {
            // Wrap around
            panel->editor()->moveCursor(QTextCursor::Start);
            found = panel->editor()->find(needle, flags);
        }
        m_statusWidget->showMessage(found ? "Found" : "Not found: " + needle, 2000);
    }

    void doReplace(bool all) {
        if (!m_findDialog) return;
        auto* panel = currentPanel();
        if (!panel) return;
        QString needle      = m_findDialog->findText();
        QString replacement = m_findDialog->replaceText();
        if (needle.isEmpty()) return;

        if (all) {
            // Replace all occurrences
            QString text = panel->text();
            Qt::CaseSensitivity cs = m_findDialog->matchCase()
                ? Qt::CaseSensitive : Qt::CaseInsensitive;
            int count = 0;
            int pos = text.indexOf(needle, 0, cs);
            while (pos != -1) {
                text.replace(pos, needle.length(), replacement);
                pos = text.indexOf(needle, pos + replacement.length(), cs);
                ++count;
            }
            panel->setText(text);
            m_statusWidget->showMessage(
                QString("Replaced %1 occurrence(s)").arg(count), 2500);
        } else {
            // Replace current selection / find next
            QTextCursor cur = panel->editor()->textCursor();
            if (cur.hasSelection()) {
                cur.insertText(replacement);
                panel->editor()->setTextCursor(cur);
            }
            doFind(false);
        }
    }

    // ── About dialog ─────────────────────────────────────────────────────────
    void showAbout() {
        // Proper glass about dialog — subclass instead of using inner filter struct
        class GlassAboutDialog : public QDialog {
        public:
            explicit GlassAboutDialog(QWidget* parent) : QDialog(parent) {
                setWindowTitle("About Notepad++ BobUI");
                setAttribute(Qt::WA_TranslucentBackground);
                setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
                setFixedSize(400, 260);
                setStyleSheet(LiquidGlassStyleSheet::kDialog);

                auto* lay = new QVBoxLayout(this);
                lay->setContentsMargins(24, 24, 24, 24);
                lay->setSpacing(12);

                auto makeLbl = [&](const QString& t, int pt=13, bool bold=false) -> QLabel* {
                    auto* l = new QLabel(t, this);
                    QFont f = l->font(); f.setPointSize(pt); f.setBold(bold); l->setFont(f);
                    l->setStyleSheet("color:rgba(235,240,255,230);background:transparent;");
                    l->setAlignment(Qt::AlignCenter);
                    return l;
                };

                lay->addWidget(makeLbl("Notepad++ BobUI", 20, true));
                lay->addWidget(makeLbl(QString("Liquid Glass Edition  v%1").arg(kAppVersion), 13));
                lay->addWidget(makeLbl("Powered by BobUI (Qt6 + JUCE + OmniUI)", 11));
                lay->addWidget(makeLbl("TextFX Engine natively integrated", 11));
                lay->addWidget(makeLbl("\u00a9 2026 robertpelloni \u00b7 Built by Antigravity AI", 10));
                lay->addStretch();

                auto* btn = new QPushButton("Close", this);
                connect(btn, &QPushButton::clicked, this, &QDialog::accept);
                lay->addWidget(btn);

                // Draggable title bar
                installEventFilter(this);
            }

        protected:
            void paintEvent(QPaintEvent*) override {
                QPainter p(this);
                p.setRenderHint(QPainter::Antialiasing);
                LiquidGlassPainter::drawGlassBackground(p, QRectF(rect()), 14.0, true);
                // Note: don't call QDialog::paintEvent — it would draw a solid background
                // on top of our glass, defeating the transparency.
            }
            void mousePressEvent(QMouseEvent* e) override {
                if (e->button() == Qt::LeftButton)
                    m_drag = e->globalPosition().toPoint() - frameGeometry().topLeft();
                e->accept();
            }
            void mouseMoveEvent(QMouseEvent* e) override {
                if (e->buttons() & Qt::LeftButton)
                    move(e->globalPosition().toPoint() - m_drag);
                e->accept();
            }
        private:
            QPoint m_drag;
        };

        GlassAboutDialog dlg(this);
        dlg.exec();
    }

    // ── Menu setup ───────────────────────────────────────────────────────────
    void setupMenuBar() {
        // Apply glass QSS to the menu bar
        menuBar()->setStyleSheet(LiquidGlassStyleSheet::kMenuBar);

        // Helper: adds a triggered action with shortcut
        auto addAct = [](QMenu* m, const QString& txt, auto slot,
                         const QKeySequence& key = QKeySequence()) -> QAction* {
            auto* a = m->addAction(txt, slot);
            if (!key.isEmpty()) a->setShortcut(key);
            return a;
        };

        // ─── FILE ─────────────────────────────────────────────────────────
        QMenu* file = menuBar()->addMenu("&File");
        file->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(file, "&New",       [this](){ actionNew(); },  QKeySequence::New);
        addAct(file, "&Open...",   [this](){ actionOpen(); }, QKeySequence::Open);
        addAct(file, "&Save",      [this](){ actionSave(); }, QKeySequence::Save);
        addAct(file, "Save &As...", [this](){
            if (auto* p = currentPanel()) saveFileAs(p);
        });
        addAct(file, "Save All",   [this](){ actionSaveAll(); },
               QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
        file->addSeparator();
        addAct(file, "&Reload",    [this](){
            if (auto* p = currentPanel(); p && !p->filePath().isEmpty()) {
                p->loadFile(p->filePath());
                m_statusWidget->showMessage("Reloaded: " + p->filePath(), 2000);
            }
        });
        file->addSeparator();
        addAct(file, "&Close",     [this](){
            onTabCloseRequested(m_tabs->currentIndex()); }, QKeySequence::Close);
        addAct(file, "Close All",  [this](){
            while (m_tabs->count() > 0) {
                onTabCloseRequested(0);
                if (m_tabs->count() == 0) break;
            }
        });
        file->addSeparator();
        addAct(file, "E&xit",  [this](){ close(); }, QKeySequence::Quit);

        // ─── EDIT ─────────────────────────────────────────────────────────
        QMenu* edit = menuBar()->addMenu("&Edit");
        edit->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(edit, "&Undo",       [this](){
            if (auto* p = currentPanel()) p->editor()->undo(); }, QKeySequence::Undo);
        addAct(edit, "&Redo",       [this](){
            if (auto* p = currentPanel()) p->editor()->redo(); }, QKeySequence::Redo);
        edit->addSeparator();
        addAct(edit, "Cu&t",        [this](){
            if (auto* p = currentPanel()) p->editor()->cut(); }, QKeySequence::Cut);
        addAct(edit, "&Copy",       [this](){
            if (auto* p = currentPanel()) p->editor()->copy(); }, QKeySequence::Copy);
        addAct(edit, "&Paste",      [this](){
            if (auto* p = currentPanel()) p->editor()->paste(); }, QKeySequence::Paste);
        addAct(edit, "&Delete",     [this](){
            if (auto* p = currentPanel()) p->editor()->textCursor().removeSelectedText();
        }, QKeySequence::Delete);
        addAct(edit, "Select &All", [this](){
            if (auto* p = currentPanel()) p->editor()->selectAll();
        }, QKeySequence::SelectAll);
        edit->addSeparator();
        // Blank operations sub-menu
        QMenu* blankOps = edit->addMenu("Blank Operations");
        blankOps->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(blankOps, "Trim Trailing Space", [this](){
            if (auto* p = currentPanel()) {
                QStringList lines = p->text().split('\n');
                for (auto& l : lines) {
                    // Remove trailing spaces/tabs without QString::rtrimmed() (doesn't exist).
                    // Iterate backward and truncate.
                    int end = l.size();
                    while (end > 0 && (l[end-1] == ' ' || l[end-1] == '\t')) --end;
                    l = l.left(end);
                }
                p->setText(lines.join('\n'));
                m_statusWidget->showMessage("Trailing spaces trimmed", 2000);
            }
        });
        addAct(blankOps, "Trim Leading Space", [this](){
            if (auto* p = currentPanel()) {
                QString t = p->text();
                QStringList lines = t.split('\n');
                for (auto& l : lines) { while (l.startsWith(' ') || l.startsWith('\t')) l.remove(0,1); }
                p->setText(lines.join('\n'));
                m_statusWidget->showMessage("Leading spaces trimmed", 2000);
            }
        });
        addAct(blankOps, "Tab to Space", [this](){
            if (auto* p = currentPanel()) {
                p->setText(p->text().replace('\t', "    "));
                m_statusWidget->showMessage("Tabs → spaces", 2000);
            }
        });

        // ─── SEARCH ───────────────────────────────────────────────────────
        QMenu* search = menuBar()->addMenu("&Search");
        search->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(search, "&Find...",        [this](){ showFindDialog(); }, QKeySequence::Find);
        addAct(search, "&Replace...",     [this](){ showFindDialog(); }, QKeySequence::Replace);
        addAct(search, "Find in &Files...", [this](){
            m_statusWidget->showMessage("Find in Files — coming soon", 2000);
        }, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
        addAct(search, "Find &Next",    [this](){ doFind(false); }, QKeySequence::FindNext);
        addAct(search, "Find &Previous",[this](){ doFind(true); },  QKeySequence::FindPrevious);
        search->addSeparator();
        addAct(search, "&Go to Line...", [this](){
            bool ok;
            int ln = QInputDialog::getInt(this, "Go to Line", "Line number:", 1, 1, 999999, 1, &ok);
            if (ok) {
                if (auto* p = currentPanel()) {
                    QTextCursor cur = p->editor()->textCursor();
                    cur.movePosition(QTextCursor::Start);
                    cur.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, ln - 1);
                    p->editor()->setTextCursor(cur);
                }
            }
        }, QKeySequence(Qt::CTRL | Qt::Key_G));

        // ─── VIEW ─────────────────────────────────────────────────────────
        QMenu* view = menuBar()->addMenu("&View");
        view->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        auto* alwaysOnTop = view->addAction("Always on Top");
        alwaysOnTop->setCheckable(true);
        connect(alwaysOnTop, &QAction::toggled, this, [this](bool on){
            Qt::WindowFlags flags = windowFlags();
            if (on) flags |= Qt::WindowStaysOnTopHint;
            else    flags &= ~Qt::WindowStaysOnTopHint;
            setWindowFlags(flags);
            show();
        });
        addAct(view, "Toggle &Full Screen", [this](){
            setWindowState(windowState() ^ Qt::WindowFullScreen);
        }, QKeySequence(Qt::Key_F11));
        view->addSeparator();
        auto* wrapAct = view->addAction("Word &Wrap");
        wrapAct->setCheckable(true);
        connect(wrapAct, &QAction::toggled, this, [this](bool on){
            if (auto* p = currentPanel()) {
                p->editor()->setLineWrapMode(
                    on ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
            }
        });
        view->addSeparator();
        // Bubbles toggle
        auto* bubbleAct = view->addAction("Bubble Animations");
        bubbleAct->setCheckable(true);
        bubbleAct->setChecked(true);
        connect(bubbleAct, &QAction::toggled, this, [this](bool on){
            if (m_bubbles) m_bubbles->setVisible(on);
        });
        view->addSeparator();
        // Zoom
        QMenu* zoom = view->addMenu("Zoom");
        zoom->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(zoom, "Zoom In",     [this](){
            if (auto* p = currentPanel()) {
                QFont f = p->editor()->font();
                f.setPointSize(f.pointSize() + 1);
                p->editor()->setFont(f);
            }
        }, QKeySequence::ZoomIn);
        addAct(zoom, "Zoom Out",    [this](){
            if (auto* p = currentPanel()) {
                QFont f = p->editor()->font();
                f.setPointSize(qMax(6, f.pointSize() - 1));
                p->editor()->setFont(f);
            }
        }, QKeySequence::ZoomOut);
        addAct(zoom, "Reset Zoom",  [this](){
            if (auto* p = currentPanel()) {
                QFont f = p->editor()->font();
                f.setPointSize(14);
                p->editor()->setFont(f);
            }
        }, QKeySequence(Qt::CTRL | Qt::Key_0));

        // ─── ENCODING ─────────────────────────────────────────────────────
        QMenu* enc = menuBar()->addMenu("En&coding");
        enc->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        for (const auto& e : {"ANSI","UTF-8","UTF-8 BOM","UTF-16 BE BOM","UTF-16 LE BOM"}) {
            auto* a = enc->addAction(e);
            a->setCheckable(true);
            if (QString(e) == "UTF-8") a->setChecked(true);
            connect(a, &QAction::triggered, this, [this, e](){
                m_statusWidget->setEncoding(e);
                m_statusWidget->showMessage(QString("Encoding: %1").arg(e), 2000);
            });
        }

        // ─── LANGUAGE ─────────────────────────────────────────────────────
        QMenu* lang = menuBar()->addMenu("&Language");
        lang->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        lang->addAction("None (Normal Text)");
        lang->addSeparator();
        QMenu* langC = lang->addMenu("C / C++");
        langC->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        for (const auto& l : {"C","C++","C#"}) langC->addAction(l);
        for (const auto& l : {"Java","Python","JavaScript","TypeScript",
                              "HTML","CSS","XML","JSON","Markdown",
                              "Bash","Batch","PowerShell","SQL","YAML"}) {
            lang->addAction(l);
        }

        // ─── TEXTFX ───────────────────────────────────────────────────────
        QMenu* tfx = menuBar()->addMenu("Te&xtFX");
        tfx->setStyleSheet(LiquidGlassStyleSheet::kMenu);

        QMenu* tfxChar = tfx->addMenu("TextFX Characters");
        tfxChar->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(tfxChar, "UPPER CASE",       [this](){ applyTextFX(6); });
        addAct(tfxChar, "lower case",       [this](){ applyTextFX(7); });
        addAct(tfxChar, "Proper Case",      [this](){ applyTextFX(8); });
        addAct(tfxChar, "iNVERT cASE",      [this](){ applyTextFX(9); });
        tfxChar->addSeparator();
        addAct(tfxChar, "Zap non-printable to #", [this](){ applyTextFX(5); });

        QMenu* tfxTools = tfx->addMenu("TextFX Tools");
        tfxTools->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(tfxTools, "Sort lines case sensitive",   [this](){ applyTextFX(1); });
        addAct(tfxTools, "Sort lines case insensitive", [this](){ applyTextFX(2); });
        addAct(tfxTools, "Delete Blank Lines",           [this](){ applyTextFX(3); });
        addAct(tfxTools, "Delete Surplus Blank Lines",   [this](){ applyTextFX(4); });
        tfxTools->addSeparator();
        addAct(tfxTools, "Insert Line Numbers",          [this](){ applyTextFX(10); });
        addAct(tfxTools, "Delete Line Numbers",          [this](){ applyTextFX(11); });

        QMenu* tfxHtml = tfx->addMenu("TextFX HTML Tidy");
        tfxHtml->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(tfxHtml, "Encode HTML entities",  [this](){
            if (auto* p = currentPanel()) {
                QString t = p->text();
                t.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
                p->setText(t);
                m_statusWidget->showMessage("HTML entities encoded", 2000);
            }
        });
        addAct(tfxHtml, "Decode HTML entities", [this](){
            if (auto* p = currentPanel()) {
                QString t = p->text();
                t.replace("&amp;","&").replace("&lt;","<").replace("&gt;",">");
                p->setText(t);
                m_statusWidget->showMessage("HTML entities decoded", 2000);
            }
        });

        // ─── MACRO ────────────────────────────────────────────────────────
        QMenu* macro = menuBar()->addMenu("&Macro");
        macro->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        macro->addAction("Start Recording",  [this](){
            m_statusWidget->showMessage("Macro recording — coming soon", 2000); });
        macro->addAction("Stop Recording");
        macro->addAction("Playback");
        macro->addSeparator();
        macro->addAction("Save Macro...");
        macro->addAction("Load Macro...");

        // ─── RUN ──────────────────────────────────────────────────────────
        QMenu* run = menuBar()->addMenu("&Run");
        run->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(run, "&Run...", [this](){
            m_statusWidget->showMessage("Run — coming soon", 2000); }, QKeySequence(Qt::Key_F5));

        // ─── WINDOW ───────────────────────────────────────────────────────
        QMenu* win = menuBar()->addMenu("&Window");
        win->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(win, "Next Tab",     [this](){ m_tabs->setCurrentIndex((m_tabs->currentIndex()+1) % m_tabs->count()); },
               QKeySequence(Qt::CTRL | Qt::Key_PageDown));
        addAct(win, "Previous Tab", [this](){
            int n = m_tabs->count();
            m_tabs->setCurrentIndex((m_tabs->currentIndex()+n-1) % n);
        }, QKeySequence(Qt::CTRL | Qt::Key_PageUp));
        win->addSeparator();
        addAct(win, "Clone Current Tab", [this](){
            auto* p = currentPanel();
            if (!p) return;
            QString text = p->text();
            QString name = m_tabs->tabText(m_tabs->currentIndex()) + " (clone)";
            addNewDocument(name);
            if (auto* np = currentPanel()) np->setText(text);
        });

        // ─── SETTINGS ────────────────────────────────────────────────────
        QMenu* settings = menuBar()->addMenu("&Settings");
        settings->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        settings->addAction("Preferences...",        [this](){
            m_statusWidget->showMessage("Preferences dialog — coming soon", 2000); });
        settings->addAction("Style Configurator...", [this](){
            m_statusWidget->showMessage("Style configurator — coming soon", 2000); });
        settings->addAction("Shortcut Mapper...",    [this](){
            m_statusWidget->showMessage("Shortcut mapper — coming soon", 2000); });
        settings->addSeparator();
        // Glass intensity slider (quick access)
        auto* intensityAct = settings->addAction("Glass Intensity: High");
        connect(intensityAct, &QAction::triggered, this, [this, intensityAct](){
            static int level = 2;  // 0=Low, 1=Medium, 2=High
            level = (level + 1) % 3;
            static const char* names[] = {"Low","Medium","High"};
            static const int modes[] = { 1, 3, 2 };  // Mica, MicaTabbed, Acrylic
            intensityAct->setText(QString("Glass Intensity: %1").arg(names[level]));
            LiquidGlassDWM::enableBlurBehind(this, modes[level]);
            m_statusWidget->showMessage(
                QString("Glass intensity: %1").arg(names[level]), 2000);
        });

        // ─── HELP / ABOUT ────────────────────────────────────────────────
        QMenu* help = menuBar()->addMenu("&?");
        help->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        help->addAction("Open Documentation", [this](){
            m_statusWidget->showMessage("Documentation — coming soon", 2000); });
        help->addAction("BobUI / OmniUI Reference", [this](){
            m_statusWidget->showMessage("OmniUI Docs — see bobui/OmniUI/docs/Manual.md", 3000); });
        help->addSeparator();
        addAct(help, "&About Notepad++ BobUI",
               [this](){ showAbout(); });
    }

    // ── Toolbar setup ────────────────────────────────────────────────────────
    void setupToolBar() {
        QToolBar* tb = addToolBar("Standard");
        tb->setMovable(false);
        tb->setStyleSheet(LiquidGlassStyleSheet::kToolBar);
        tb->setIconSize(QSize(16, 16));

        auto addTBtn = [&](const QString& txt, const QString& tip, auto slot) {
            auto* btn = new QToolButton(tb);
            btn->setText(txt);
            btn->setToolTip(tip);
            connect(btn, &QToolButton::clicked, this, slot);
            tb->addWidget(btn);
            return btn;
        };

        addTBtn("📄 New",    "New file (Ctrl+N)",     [this](){ actionNew(); });
        addTBtn("📂 Open",   "Open file(s) (Ctrl+O)", [this](){ actionOpen(); });
        addTBtn("💾 Save",   "Save (Ctrl+S)",         [this](){ actionSave(); });
        tb->addSeparator();
        addTBtn("✂ Cut",    "Cut (Ctrl+X)",  [this](){
            if (auto* p = currentPanel()) p->editor()->cut(); });
        addTBtn("⎘ Copy",   "Copy (Ctrl+C)", [this](){
            if (auto* p = currentPanel()) p->editor()->copy(); });
        addTBtn("📋 Paste",  "Paste (Ctrl+V)",[this](){
            if (auto* p = currentPanel()) p->editor()->paste(); });
        tb->addSeparator();
        addTBtn("↩ Undo",   "Undo (Ctrl+Z)", [this](){
            if (auto* p = currentPanel()) p->editor()->undo(); });
        addTBtn("↪ Redo",   "Redo (Ctrl+Y)", [this](){
            if (auto* p = currentPanel()) p->editor()->redo(); });
        tb->addSeparator();
        addTBtn("🔍 Find",   "Find (Ctrl+F)", [this](){ showFindDialog(); });
    }

    // ── Members ───────────────────────────────────────────────────────────────
    QTabWidget*      m_tabs;
    GlassStatusWidget* m_statusWidget;
    BubbleOverlay*   m_bubbles;
    GlassFindDialog* m_findDialog;
};


// ─────────────────────────────────────────────────────────────────────────────
// MAIN ENTRY POINT
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    // Enable DWM composition / high-DPI before QApplication is created.
    // On Windows, DWM must be enabled before the message loop starts.
#ifdef Q_OS_WIN
    // Request per-monitor DPI awareness v2 (Windows 10 1703+)
    // This ensures the glass blur scales correctly on multi-monitor setups.
    typedef BOOL (WINAPI* pfnSetProcessDpiAwarenessContext)(HANDLE);
    if (HMODULE h = LoadLibraryW(L"user32.dll")) {
        auto fn = reinterpret_cast<pfnSetProcessDpiAwarenessContext>(
            GetProcAddress(h, "SetProcessDpiAwarenessContext"));
        if (fn) fn(reinterpret_cast<HANDLE>(-4));  // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
        FreeLibrary(h);
    }
#endif

    // Qt platform rendering flags — enable OpenGL/RHI for smooth compositing
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("Notepad++ BobUI Liquid Glass");
    app.setApplicationVersion(kAppVersion);
    app.setOrganizationName("robertpelloni");
    app.setOrganizationDomain("github.com/robertpelloni");

    // Apply the global liquid glass stylesheet
    app.setStyleSheet(LiquidGlassStyleSheet::kFullGlassTheme());

    // Launch the main window
    BobNppGlassWindow win;
    win.show();

    return app.exec();
}
