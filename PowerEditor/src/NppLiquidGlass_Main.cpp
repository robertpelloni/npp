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
#include <QDockWidget>
#include <QTreeView>
#include <QListWidget>
#include <QSystemTrayIcon>
#include <QVariantList>
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
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QtPlugin>
#include <random>
#include <cmath>

// BobUI platform plugin (static link)
#ifdef QT_STATIC
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

// Our glass system
#include "LiquidGlass.h"
#include "GlassSettings.h"
#include "GlassPreferencesDialog.h"
#include "GlassSearchWorker.h"
#include "GlassFindInFilesDialog.h"
#include "GlassCommandPalette.h"
#include "GlassStyleConfigurator.h"
#include "GlassTerminal.h"
#include "GlassConfigXML.h"
#include "GlassShortcutMapper.h"

// TextFX algorithm engine
#include "TextFXEngine.h"

// Scintilla API (for future real editor integration)
#include "Scintilla.h"
#include "Sci_Position.h"
#include "BobScintilla.h"
#include "GlassThemeManager.h"
#include "GlassSplashScreen.h"

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

        m_mainLayout = new QHBoxLayout(this);
        m_mainLayout->setContentsMargins(1, 1, 1, 1);
        m_mainLayout->setSpacing(0);

        m_leftSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        m_rightSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        m_mainLayout->addSpacerItem(m_leftSpacer);

        // The native Scintilla text editor
        m_editor = new BobScintilla(this);
        
        // Apply settings
        m_editor->setFont(GlassSettings::instance().editorFont());
        setWordWrap(GlassSettings::instance().wordWrapDefault());
        
        m_mainLayout->addWidget(m_editor, 1);

        // Minimap
        m_minimap = new BobScintilla(this);
        m_minimap->setFixedWidth(100);
        m_minimap->send(SCI_SETZOOM, -10); // zoom way out
        m_minimap->send(SCI_SETREADONLY, 1);
        m_minimap->send(SCI_SETVSCROLLBAR, 0);
        m_minimap->send(SCI_SETHSCROLLBAR, 0);
        m_minimap->send(SCI_SETMARGINWIDTHN, 0, 0);
        m_minimap->send(SCI_SETMARGINWIDTHN, 1, 0);
        m_minimap->send(SCI_SETMARGINWIDTHN, 2, 0);
        m_minimap->setVisible(GlassSettings::instance().showMinimap());
        m_mainLayout->addWidget(m_minimap, 0);
        m_mainLayout->addSpacerItem(m_rightSpacer);

        // Sync Minimap
        m_editor->onContentsChanged = [this]() {
            m_minimap->send(SCI_SETREADONLY, 0);
            m_minimap->setText(m_editor->text());
            m_minimap->send(SCI_SETREADONLY, 1);
            if (m_onContentsChanged) m_onContentsChanged();
        };
        m_editor->onCursorPositionChanged = [this]() {
            sptr_t line = m_editor->send(SCI_LINEFROMPOSITION, m_editor->send(SCI_GETCURRENTPOS));
            m_minimap->send(SCI_GOTOLINE, line);
            if (m_onCursorPositionChanged) m_onCursorPositionChanged();
        };
    }

    void setDistractionFree(bool on) {
        int margin = on ? 200 : 0;
        m_leftSpacer->changeSize(margin, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        m_rightSpacer->changeSize(margin, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        m_mainLayout->invalidate();
        m_minimap->setVisible(!on && GlassSettings::instance().showMinimap());
    }

    std::function<void()> m_onCursorPositionChanged;
    std::function<void()> m_onContentsChanged;

    BobScintilla* editor() { return m_editor; }

    QString text() const     { return m_editor->text(); }
    void setText(const QString& t) { m_editor->setText(t); }
    void setFilePath(const QString& p) { m_filePath = p; }
    QString filePath() const { return m_filePath; }

    bool loadFile(const QString& path) {
        QFile f(path);
        if (!f.open(QFile::ReadOnly | QFile::Text)) return false;
        m_filePath = path;
        
        // Setup lexer based on extension
        m_editor->applyLexer(QFileInfo(path).suffix());
        
        m_editor->setText(QString::fromUtf8(f.readAll()));
        m_modified = false;
        m_editor->onModificationChanged = [this](bool mod){ m_modified = mod; };
        return true;
    }

    bool saveFile() {
        if (m_filePath.isEmpty()) return false;
        QFile f(m_filePath);
        if (!f.open(QFile::WriteOnly | QFile::Text)) return false;
        QTextStream s(&f);
        s << m_editor->text();
        m_modified = false;
        // Tell Scintilla to save state
        m_editor->send(SCI_SETSAVEPOINT);
        return true;
    }

    bool isModified() const { return m_modified; }

    bool wordWrap() const { return m_editor->wordWrap(); }
    void setWordWrap(bool wrap) {
        m_editor->setWordWrap(wrap);
    }

protected:
    void paintEvent(QPaintEvent* ev) override {
        // Draw the glass material as the border/frame of the editor area.
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Draw a glass rim around the editor
        QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        LiquidGlassPainter::drawGlassBackground(p, r,
            LiquidGlassPalette::kCornerRadius * 0.7, true);

        QFrame::paintEvent(ev);
    }

private:
    BobScintilla* m_editor;
    BobScintilla* m_minimap;
    QHBoxLayout*  m_mainLayout;
    QSpacerItem*  m_leftSpacer;
    QSpacerItem*  m_rightSpacer;
    QString       m_filePath;
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

    void updateStats(BobScintilla* editor) {
        if (!editor) return;
        sptr_t pos = editor->send(SCI_GETCURRENTPOS);
        sptr_t ln  = editor->send(SCI_LINEFROMPOSITION, pos) + 1;
        sptr_t col = editor->send(SCI_GETCOLUMN, pos) + 1;
        sptr_t sel = editor->send(SCI_GETSELECTIONEND) - editor->send(SCI_GETSELECTIONSTART);
        
        QString stats = QString("Ln %1, Col %2").arg(ln).arg(col);
        if (editor->wordWrap()) {
            sptr_t displayLn = editor->send(SCI_VISIBLEFROMDOCLINE, ln - 1) + 1;
            stats += QString(" (Display Ln %1)").arg(displayLn);
        }
        m_lnColLabel->setText(stats);
        
        QString text = editor->text();
        QStringList words = text.split(QRegularExpression(R"(\s+)"),
                                       Qt::SkipEmptyParts);
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
        
        // Load settings
        m_caseCheck->setChecked(GlassSettings::instance().findMatchCase());
        m_wholeCheck->setChecked(GlassSettings::instance().findWholeWord());
        m_regexCheck->setChecked(GlassSettings::instance().findUseRegex());
        
        connect(m_caseCheck, &QCheckBox::toggled, [](bool v){ GlassSettings::instance().setFindMatchCase(v); });
        connect(m_wholeCheck, &QCheckBox::toggled, [](bool v){ GlassSettings::instance().setFindWholeWord(v); });
        connect(m_regexCheck, &QCheckBox::toggled, [](bool v){ GlassSettings::instance().setFindUseRegex(v); });

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
        m_markAllBtn   = new QPushButton("Mark All",     this);
        m_closeBtn2    = new QPushButton("Close",        this);
        auto* btnLay   = new QHBoxLayout;
        btnLay->addWidget(m_findNextBtn);
        btnLay->addWidget(m_replaceBtn);
        btnLay->addWidget(m_replAllBtn);
        btnLay->addWidget(m_markAllBtn);
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
    QPushButton* markAllButton()   { return m_markAllBtn; }

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
    QPushButton* m_markAllBtn;
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

        // ── Main Splitter ──
        m_mainSplitter = new QSplitter(Qt::Horizontal, central);
        m_mainSplitter->setStyleSheet("QSplitter::handle { background: rgba(255,255,255,10); }");

        // ── Tab widget 1 ──
        m_tabs = new QTabWidget(m_mainSplitter);
        m_tabs->setMovable(true);
        m_tabs->setTabsClosable(true);
        m_tabs->setDocumentMode(true);
        m_tabs->setStyleSheet(LiquidGlassStyleSheet::kTabWidget);
        connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int idx) {
            onTabCloseRequested(m_tabs, idx);
        });
        connect(m_tabs, &QTabWidget::currentChanged, this, [this](int idx) {
            onCurrentTabChanged(m_tabs, idx);
        });

        // ── Tab widget 2 ──
        m_tabsSecondary = new QTabWidget(m_mainSplitter);
        m_tabsSecondary->setMovable(true);
        m_tabsSecondary->setTabsClosable(true);
        m_tabsSecondary->setDocumentMode(true);
        m_tabsSecondary->setStyleSheet(LiquidGlassStyleSheet::kTabWidget);
        m_tabsSecondary->hide(); // Hidden by default
        connect(m_tabsSecondary, &QTabWidget::tabCloseRequested, this, [this](int idx) {
            onTabCloseRequested(m_tabsSecondary, idx);
        });
        connect(m_tabsSecondary, &QTabWidget::currentChanged, this, [this](int idx) {
            onCurrentTabChanged(m_tabsSecondary, idx);
        });

        m_mainSplitter->addWidget(m_tabs);
        m_mainSplitter->addWidget(m_tabsSecondary);
        mainLay->addWidget(m_mainSplitter, 1);

        // ── Glass status widget (replaces QStatusBar) ──
        m_statusWidget = new GlassStatusWidget(central);
        mainLay->addWidget(m_statusWidget, 0);

        // ── Menus & toolbar ──────────────────────────────────────────────────
        setupMenuBar();
        setupToolBar();
        setupDockWidgets();
        setupSystemTray();

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

        // ── Backup Timer ─────────────────────────────────────────────────────
        m_backupTimer = new QTimer(this);
        connect(m_backupTimer, &QTimer::timeout, this, &BobNppGlassWindow::performBackup);
        if (GlassSettings::instance().backupEnabled()) {
            m_backupTimer->start(GlassSettings::instance().backupInterval());
        }

        // ── Restore Window State ─────────────────────────────────────────────
        auto& s = GlassSettings::instance();
        QByteArray geom = s.windowGeometry();
        if (!geom.isEmpty()) restoreGeometry(geom);
        QByteArray state = s.windowState();
        if (!state.isEmpty()) restoreState(state);

        // ── Restore Session ──────────────────────────────────────────────────
        QStringList sessionFiles = s.openFiles();
        if (!sessionFiles.isEmpty()) {
            // Remove the default "new 1" if we have a session
            if (m_tabs->count() > 0 && static_cast<GlassEditorPanel*>(m_tabs->widget(0))->filePath().isEmpty()) {
                m_tabs->removeTab(0);
            }
            for (const QString& path : sessionFiles) {
                if (QFileInfo::exists(path)) {
                    auto* panel = new GlassEditorPanel(m_tabs);
                    if (panel->loadFile(path)) {
                        m_tabs->addTab(panel, QFileInfo(path).fileName());
                        connectEditorSignals(panel);
                    } else {
                        delete panel;
                    }
                }
            }
            if (m_tabs->count() > 0) {
                m_tabs->setCurrentIndex(qBound(0, s.activeTab(), m_tabs->count() - 1));
            } else {
                addNewDocument("new 1");
            }
        }
    }

protected:
    void showEvent(QShowEvent* e) override {
        QMainWindow::showEvent(e);
        // DWM activation must happen AFTER the HWND is created (after show).
        // We use a deferred call to ensure the native handle exists.
        QTimer::singleShot(0, this, [this]() {
            auto& s = GlassSettings::instance();
            static const int modes[] = { 1, 3, 2 }; // Mica, MicaTabbed, Acrylic
            LiquidGlassDWM::enableBlurBehind(this, modes[s.glassIntensity()]);
            LiquidGlassDWM::setDarkMode(this, s.darkMode());
            LiquidGlassDWM::setRoundedCorners(this, 2); // 2 = DWMWCP_ROUND
            LiquidGlassDWM::setBorderColor(this, QColor(80, 130, 255, 180));
            
            // Initial bubbles state
            if (m_bubbles) m_bubbles->setVisible(s.bubbleAnimations());

            // Initialize Plugin Bridge
            NppData data;
            data._nppHandle = reinterpret_cast<HWND>(winId());
            data._scintillaMainHandle = nullptr;
            if (auto* panel = currentPanel()) {
                data._scintillaMainHandle = panel->editor()->getHwnd();
            }
            data._scintillaSecondHandle = nullptr;
            
            GlassPluginManager::instance().loadPlugins("plugins", data);
            
            // Update Plugins Menu with loaded plugins
            updatePluginsMenu();
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

    void closeEvent(QCloseEvent* ev) override {
        // Save window state
        auto& s = GlassSettings::instance();
        s.setWindowGeometry(saveGeometry());
        s.setWindowState(saveState());
        
        // Save session
        QStringList files;
        for (int i = 0; i < m_tabs->count(); ++i) {
            if (auto* p = static_cast<GlassEditorPanel*>(m_tabs->widget(i))) {
                if (!p->filePath().isEmpty()) files << p->filePath();
            }
        }
        s.setOpenFiles(files);
        s.setActiveTab(m_tabs->currentIndex());
        bool hasUnsaved = false;
        for (int i = 0; i < m_tabs->count(); ++i) {
            if (auto* p = static_cast<GlassEditorPanel*>(m_tabs->widget(i))) {
                if (p->isModified()) hasUnsaved = true;
            }
        }
        
        if (hasUnsaved) {
            auto btn = QMessageBox::question(this, "Unsaved Changes",
                "There are unsaved documents. Save them before exiting?",
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
                
            if (btn == QMessageBox::Cancel) {
                ev->ignore();
                return;
            } else if (btn == QMessageBox::Yes) {
                actionSaveAll();
                // Check if any are still unsaved (e.g. cancelled save dialog)
                for (int i = 0; i < m_tabs->count(); ++i) {
                    if (auto* p = static_cast<GlassEditorPanel*>(m_tabs->widget(i))) {
                        if (p->isModified()) {
                            ev->ignore();
                            return;
                        }
                    }
                }
            }
        }
        
        QMainWindow::closeEvent(ev);
    }

    void toggleDistractionFree() {
        bool on = !GlassSettings::instance().distractionFree();
        GlassSettings::instance().setDistractionFree(on);
        
        menuBar()->setVisible(!on);
        // Toolbars
        for (auto* tb : findChildren<QToolBar*>()) tb->setVisible(!on);
        // Status bar
        m_statusWidget->setVisible(!on);
        // Docks
        if (on) {
            m_folderDock->hide();
            m_functionDock->hide();
            m_searchDock->hide();
        } else {
            // Restore based on some logic or just keep hidden
        }
        
        // Centering the editor panel (add margins)
        for (auto* tw : {m_tabs, m_tabsSecondary}) {
            for (int i = 0; i < tw->count(); ++i) {
                auto* p = static_cast<GlassEditorPanel*>(tw->widget(i));
                p->setDistractionFree(on);
            }
        }
    }

    void showCommandPalette() {
        GlassCommandPalette dlg(this);
        
        // Collect all actions from the menu bar
        QList<QAction*> actions;
        for (auto* menu : menuBar()->findChildren<QMenu*>()) {
            actions.append(menu->actions());
        }
        
        dlg.populate(actions);
        
        // Center on screen
        dlg.move(rect().center().x() - dlg.width() / 2, rect().top() + 100);
        
        dlg.exec();
    }

private:
    void setupSystemTray() {
        m_trayIcon = new QSystemTrayIcon(this);
        // Create a basic pixel map for the icon if no icon is available
        QPixmap pix(32, 32);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor(80, 140, 255));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(4, 4, 24, 24, 6, 6);
        p.setPen(QPen(Qt::white, 2));
        p.drawText(pix.rect(), Qt::AlignCenter, "N+");
        m_trayIcon->setIcon(QIcon(pix));
        
        QMenu* trayMenu = new QMenu(this);
        trayMenu->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        trayMenu->setAttribute(Qt::WA_TranslucentBackground);
        
        trayMenu->addAction("Restore", this, &QMainWindow::showNormal);
        trayMenu->addSeparator();
        trayMenu->addAction("New File", this, &BobNppGlassWindow::actionNew);
        trayMenu->addAction("Open File...", this, &BobNppGlassWindow::actionOpen);
        trayMenu->addSeparator();
        trayMenu->addAction("Exit", this, &QMainWindow::close);
        
        m_trayIcon->setContextMenu(trayMenu);
        connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason){
            if (reason == QSystemTrayIcon::Trigger) {
                if (isVisible()) hide();
                else { show(); raise(); activateWindow(); }
            }
        });
        m_trayIcon->show();
    }

    void setupDockWidgets() {
        // ── Folder as Workspace ─────────────────────────────────────────────
        m_folderDock = new QDockWidget("Folder as Workspace", this);
        m_folderDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
        
        auto* tree = new QTreeView(m_folderDock);
        tree->setHeaderHidden(true);
        tree->setContextMenuPolicy(Qt::CustomContextMenu);
        
        m_folderModel = new QStandardItemModel(tree);
        tree->setModel(m_folderModel);
        
        m_folderDock->setWidget(tree);
        addDockWidget(Qt::LeftDockWidgetArea, m_folderDock);
        m_folderDock->hide();

        connect(tree, &QTreeView::customContextMenuRequested, this, [this, tree](const QPoint& pos){
            QMenu menu(this);
            menu.setStyleSheet(LiquidGlassStyleSheet::kMenu);
            menu.addAction("Add Root Folder...", this, [this](){
                QString dir = QFileDialog::getExistingDirectory(this, "Select Folder");
                if (!dir.isEmpty()) addWorkspaceRoot(dir);
            });
            menu.exec(tree->viewport()->mapToGlobal(pos));
        });

        // Connect double-click to open file
        connect(tree, &QTreeView::doubleClicked, this, [this](const QModelIndex& index){
            QString path = m_folderModel->data(index, Qt::UserRole).toString();
            if (!path.isEmpty() && QFileInfo(path).isFile()) {
                auto* p = findPanelByPath(path);
                if (!p) {
                    p = new GlassEditorPanel(m_tabs);
                    if (p->loadFile(path)) {
                        m_tabs->setCurrentIndex(m_tabs->addTab(p, QFileInfo(path).fileName()));
                        connectEditorSignals(p);
                    } else {
                        delete p;
                    }
                } else {
                    m_tabs->setCurrentWidget(p);
                }
            }
        });

        // ── Function List ───────────────────────────────────────────────────
        m_functionDock = new QDockWidget("Function List", this);
        m_functionDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
        auto* list = new QListWidget(m_functionDock);
        list->addItem("main(int argc, char* argv[])");
        list->addItem("drawGlassBackground(QPainter& p)");
        list->addItem("enableBlurBehind(QWidget* w)");
        m_functionDock->setWidget(list);
        addDockWidget(Qt::RightDockWidgetArea, m_functionDock);
        m_functionDock->hide();
        
        // ── Search Results ──────────────────────────────────────────────────
        m_searchDock = new QDockWidget("Search Results", this);
        m_searchDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
        m_searchTree = new QTreeView(m_searchDock);
        m_searchTree->setHeaderHidden(true);
        m_searchModel = new QStandardItemModel(m_searchTree);
        m_searchTree->setModel(m_searchModel);
        m_searchDock->setWidget(m_searchTree);
        addDockWidget(Qt::BottomDockWidgetArea, m_searchDock);
        m_searchDock->hide();

        // ── Glass Terminal ──────────────────────────────────────────────────
        m_terminalDock = new QDockWidget("Glass Terminal", this);
        m_terminalDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
        auto* term = new GlassTerminal(m_terminalDock);
        m_terminalDock->setWidget(term);
        addDockWidget(Qt::BottomDockWidgetArea, m_terminalDock);
        m_terminalDock->hide();
        term->startShell();

        // Connect search result clicking to open file and jump to line
        connect(m_searchTree, &QTreeView::doubleClicked, this, [this](const QModelIndex& idx){
            auto* item = m_searchModel->itemFromIndex(idx);
            if (item && item->parent()) {
                // It's a line number node
                QString path = item->parent()->text();
                QVariantList data = item->data(Qt::UserRole).toList();
                if (data.size() < 3) return;
                int line = data[0].toInt();
                int col  = data[1].toInt();
                int len  = data[2].toInt();
                
                // Open file
                auto* p = findPanelByPath(path);
                if (!p) {
                    p = new GlassEditorPanel(m_tabs);
                    if (p->loadFile(path)) {
                        m_tabs->setCurrentIndex(m_tabs->addTab(p, QFileInfo(path).fileName()));
                        connectEditorSignals(p);
                    } else {
                        delete p;
                        return;
                    }
                } else {
                    m_tabs->setCurrentWidget(p);
                }
                
                // Jump to line and highlight
                p->editor()->clearIndicator(8);
                sptr_t lineStart = p->editor()->send(SCI_POSITIONFROMLINE, line - 1);
                sptr_t pos = lineStart + col;
                p->editor()->send(SCI_GOTOPOS, pos);
                p->editor()->highlightRange(pos, len, 8);
                p->editor()->send(SCI_ENSUREVISIBLEENFORCEPOLICY, line - 1);
                p->editor()->send(SCI_GRABFOCUS);
            }
        });
    }

    // ── Document management ─────────────────────────────────────────────────
    void addNewDocument(const QString& name) {
        auto* tw = currentTabWidget();
        auto* panel = new GlassEditorPanel(tw);
        int idx = tw->addTab(panel, name);
        tw->setCurrentIndex(idx);
        connectEditorSignals(panel);
        
        // Setup macro recording for this panel
        panel->editor()->onMacroRecord = [this](unsigned int m, uptr_t w, sptr_t l){
            if (m_isRecordingMacro) {
                m_macroCommands.append({m, w, l});
            }
        };
    }

    void connectEditorSignals(GlassEditorPanel* panel) {
        // Update status bar on cursor movement / content change
        panel->m_onCursorPositionChanged = [this, panel]() { 
            if (currentTabWidget()->currentWidget() == panel) m_statusWidget->updateStats(panel->editor()); 
        };
        panel->m_onContentsChanged = [this, panel]() { 
            if (currentTabWidget()->currentWidget() == panel) m_statusWidget->updateStats(panel->editor()); 
        };
        
        // Synchronized Scrolling
        panel->editor()->onScroll = [this, panel]() {
            if (m_syncScrollingV || m_syncScrollingH) {
                auto* edSrc = panel->editor();
                sptr_t line = edSrc->send(SCI_GETFIRSTVISIBLELINE);
                sptr_t xoff = edSrc->send(SCI_GETXOFFSET);
                
                // Find panels in other view
                QTabWidget* otherTw = (qobject_cast<QTabWidget*>(panel->parentWidget()) == m_tabs) ? m_tabsSecondary : m_tabs;
                if (auto* otherP = static_cast<GlassEditorPanel*>(otherTw->currentWidget())) {
                    auto* edDst = otherP->editor();
                    // Avoid recursive loop
                    if (m_syncScrollingV && edDst->send(SCI_GETFIRSTVISIBLELINE) != line) {
                        edDst->send(SCI_SETFIRSTVISIBLELINE, line);
                    }
                    if (m_syncScrollingH && edDst->send(SCI_GETXOFFSET) != xoff) {
                        edDst->send(SCI_SETXOFFSET, xoff);
                    }
                }
            }
        };
    }

    GlassEditorPanel* currentPanel() {
        return static_cast<GlassEditorPanel*>(currentTabWidget()->currentWidget());
    }

    QTabWidget* currentTabWidget() {
        if (m_tabsSecondary->isVisible() && m_tabsSecondary->hasFocus()) return m_tabsSecondary;
        // Check which one has a child with focus
        if (m_tabsSecondary->isVisible() && m_tabsSecondary->findChildren<QWidget*>().contains(qApp->focusWidget())) return m_tabsSecondary;
        return m_tabs;
    }

    GlassEditorPanel* findPanelByPath(const QString& path) {
        for (auto* tw : {m_tabs, m_tabsSecondary}) {
            for (int i = 0; i < tw->count(); ++i) {
                auto* p = static_cast<GlassEditorPanel*>(tw->widget(i));
                if (p && p->filePath() == path) return p;
            }
        }
        return nullptr;
    }

    void onTabCloseRequested(QTabWidget* tw, int idx) {
        auto* panel = static_cast<GlassEditorPanel*>(tw->widget(idx));
        if (panel && panel->isModified()) {
            auto btn = QMessageBox::question(this, "Unsaved Changes",
                QString("'%1' has unsaved changes. Save before closing?")
                    .arg(tw->tabText(idx)),
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
        tw->removeTab(idx);
        delete panel;
        if (m_tabs->count() == 0 && m_tabsSecondary->count() == 0) addNewDocument("new 1");
        if (tw->count() == 0 && tw == m_tabsSecondary) tw->hide();
    }

    void onCurrentTabChanged(QTabWidget* tw, int idx) {
        auto* panel = static_cast<GlassEditorPanel*>(tw->widget(idx));
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
        auto* tw = currentTabWidget();
        for (const auto& path : files) {
            auto* panel = new GlassEditorPanel(tw);
            if (panel->loadFile(path)) {
                int idx = tw->addTab(panel, QFileInfo(path).fileName());
                tw->setCurrentIndex(idx);
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
        auto* tw = currentTabWidget();
        if (panel->filePath().isEmpty()) { saveFileAs(panel); return; }
        if (panel->saveFile()) {
            tw->setTabText(tw->currentIndex(),
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
        // Find which tab widget owns this panel
        QTabWidget* tw = qobject_cast<QTabWidget*>(panel->parentWidget());
        if (tw) {
            int idx = tw->indexOf(panel);
            if (idx != -1) tw->setTabText(idx, QFileInfo(path).fileName());
        }
        m_statusWidget->showMessage("Saved: " + path, 2000);
        return true;
    }

    void actionSaveAll() {
        for (auto* tw : {m_tabs, m_tabsSecondary}) {
            for (int i = 0; i < tw->count(); ++i) {
                auto* p = static_cast<GlassEditorPanel*>(tw->widget(i));
                if (p && p->isModified()) {
                    if (p->filePath().isEmpty()) {
                        tw->setCurrentIndex(i);
                        saveFileAs(p);
                    } else {
                        p->saveFile();
                        tw->setTabText(i, QFileInfo(p->filePath()).fileName());
                    }
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
            connect(m_findDialog->markAllButton(), &QPushButton::clicked, this, [this](){ doMarkAll(); });
        }
        m_findDialog->show();
        m_findDialog->raise();
        m_findDialog->activateWindow();
    }

    void doMarkAll() {
        if (!m_findDialog) return;
        auto* panel = currentPanel();
        if (!panel) return;
        QByteArray needle = m_findDialog->findText().toUtf8();
        if (needle.isEmpty()) return;

        auto* ed = panel->editor();
        ed->clearIndicator(9); // Clear green marks

        int flags = 0;
        if (m_findDialog->matchCase()) flags |= SCFIND_MATCHCASE;
        if (m_findDialog->wholeWord()) flags |= SCFIND_WHOLEWORD;
        if (m_findDialog->useRegex())  flags |= SCFIND_REGEXP;
        
        ed->send(SCI_SETSEARCHFLAGS, flags);
        ed->send(SCI_SETTARGETSTART, 0);
        ed->send(SCI_SETTARGETEND, ed->send(SCI_GETLENGTH));
        
        int count = 0;
        while (ed->send(SCI_SEARCHINTARGET, needle.length(), reinterpret_cast<sptr_t>(needle.constData())) != -1) {
            sptr_t matchStart = ed->send(SCI_GETTARGETSTART);
            sptr_t matchEnd = ed->send(SCI_GETTARGETEND);
            ed->highlightRange(matchStart, matchEnd - matchStart, 9);
            
            ed->send(SCI_SETTARGETSTART, matchEnd);
            ed->send(SCI_SETTARGETEND, ed->send(SCI_GETLENGTH));
            count++;
        }
        m_statusWidget->showMessage(QString("Marked %1 occurrence(s)").arg(count), 2500);
    }

    void doFind(bool backward) {
        if (!m_findDialog) return;
        auto* panel = currentPanel();
        if (!panel) return;
        QByteArray needle = m_findDialog->findText().toUtf8();
        if (needle.isEmpty()) return;

        int flags = 0;
        if (m_findDialog->matchCase()) flags |= SCFIND_MATCHCASE;
        if (m_findDialog->wholeWord()) flags |= SCFIND_WHOLEWORD;
        if (m_findDialog->useRegex())  flags |= SCFIND_REGEXP;
        
        auto* ed = panel->editor();
        ed->send(SCI_SETSEARCHFLAGS, flags);
        
        sptr_t startPos = ed->send(SCI_GETCURRENTPOS);
        sptr_t docEnd = ed->send(SCI_GETLENGTH);
        
        if (backward) {
            ed->send(SCI_SETTARGETSTART, startPos);
            ed->send(SCI_SETTARGETEND, 0);
        } else {
            ed->send(SCI_SETTARGETSTART, startPos);
            ed->send(SCI_SETTARGETEND, docEnd);
        }
        
        sptr_t pos = ed->send(SCI_SEARCHINTARGET, needle.length(), reinterpret_cast<sptr_t>(needle.constData()));
        if (pos == -1) {
            // Wrap around
            if (backward) {
                ed->send(SCI_SETTARGETSTART, docEnd);
                ed->send(SCI_SETTARGETEND, 0);
            } else {
                ed->send(SCI_SETTARGETSTART, 0);
                ed->send(SCI_SETTARGETEND, docEnd);
            }
            pos = ed->send(SCI_SEARCHINTARGET, needle.length(), reinterpret_cast<sptr_t>(needle.constData()));
        }
        
        if (pos != -1) {
            sptr_t tgtEnd = ed->send(SCI_GETTARGETEND);
            ed->send(SCI_SETSEL, pos, tgtEnd);
            m_statusWidget->showMessage("Found", 2000);
        } else {
            m_statusWidget->showMessage("Not found: " + m_findDialog->findText(), 2000);
        }
    }

    void doReplace(bool all) {
        if (!m_findDialog) return;
        auto* panel = currentPanel();
        if (!panel) return;
        QByteArray needle = m_findDialog->findText().toUtf8();
        QByteArray replacement = m_findDialog->replaceText().toUtf8();
        if (needle.isEmpty()) return;

        auto* ed = panel->editor();
        
        if (all) {
            int flags = 0;
            if (m_findDialog->matchCase()) flags |= SCFIND_MATCHCASE;
            if (m_findDialog->wholeWord()) flags |= SCFIND_WHOLEWORD;
            if (m_findDialog->useRegex())  flags |= SCFIND_REGEXP;
            
            ed->send(SCI_SETSEARCHFLAGS, flags);
            ed->send(SCI_SETTARGETSTART, 0);
            ed->send(SCI_SETTARGETEND, ed->send(SCI_GETLENGTH));
            
            int count = 0;
            ed->send(SCI_BEGINUNDOACTION);
            while (ed->send(SCI_SEARCHINTARGET, needle.length(), reinterpret_cast<sptr_t>(needle.constData())) != -1) {
                if (m_findDialog->useRegex()) {
                    ed->send(SCI_REPLACETARGETRE, replacement.length(), reinterpret_cast<sptr_t>(replacement.constData()));
                } else {
                    ed->send(SCI_REPLACETARGET, replacement.length(), reinterpret_cast<sptr_t>(replacement.constData()));
                }
                ed->send(SCI_SETTARGETSTART, ed->send(SCI_GETTARGETEND));
                ed->send(SCI_SETTARGETEND, ed->send(SCI_GETLENGTH));
                count++;
            }
            ed->send(SCI_ENDUNDOACTION);
            m_statusWidget->showMessage(QString("Replaced %1 occurrence(s)").arg(count), 2500);
        } else {
            sptr_t selStart = ed->send(SCI_GETSELECTIONSTART);
            sptr_t selEnd = ed->send(SCI_GETSELECTIONEND);
            if (selEnd > selStart) {
                ed->send(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(replacement.constData()));
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
    void addWorkspaceRoot(const QString& path) {
        auto* root = new QStandardItem(QIcon::fromTheme("folder"), QFileInfo(path).fileName());
        root->setData(path, Qt::UserRole);
        m_folderModel->appendRow(root);
        
        // Scan directory (one level deep for simplicity in this turn)
        QDir dir(path);
        for (const auto& entry : dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
            auto* item = new QStandardItem(entry.fileName());
            item->setData(entry.absoluteFilePath(), Qt::UserRole);
            root->appendRow(item);
        }
    }

    void performBackup() {
        QDir backupDir("backup");
        if (!backupDir.exists()) backupDir.mkpath(".");
        
        for (auto* tw : {m_tabs, m_tabsSecondary}) {
            for (int i = 0; i < tw->count(); ++i) {
                auto* p = static_cast<GlassEditorPanel*>(tw->widget(i));
                if (p && p->isModified()) {
                    QString name = p->filePath().isEmpty() ? QString("new_%1.bak").arg(i) : QFileInfo(p->filePath()).fileName() + ".bak";
                    QFile f(backupDir.filePath(name));
                    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&f);
                        out << p->text();
                        m_statusWidget->showMessage("Auto-backup performed", 1000);
                    }
                }
            }
        }
    }

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
            auto* tw = currentTabWidget();
            onTabCloseRequested(tw, tw->currentIndex()); }, QKeySequence::Close);
        addAct(file, "Close All",  [this](){
            for (auto* tw : {m_tabs, m_tabsSecondary}) {
                while (tw->count() > 0) {
                    onTabCloseRequested(tw, 0);
                    if (tw->count() == 0) break;
                }
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
            if (auto* p = currentPanel()) p->editor()->send(SCI_CLEAR);
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
            GlassFindInFilesDialog dlg(this);
            dlg.setFindCallback([&](){
                if (dlg.findText().isEmpty() || dlg.directory().isEmpty()) return;
                
                if (m_searchWorker) {
                    m_searchWorker->cancel();
                    m_searchWorker->wait();
                    delete m_searchWorker;
                }
                
                m_searchModel->clear();
                m_searchDock->show();
                m_searchDock->raise();
                
                auto* rootItem = m_searchModel->invisibleRootItem();
                auto* searchNode = new QStandardItem(QString("Find in '%1'").arg(dlg.directory()));
                rootItem->appendRow(searchNode);
                
                m_searchWorker = new GlassSearchWorker(dlg.findText(), dlg.directory(), dlg.filterText(),
                                                       dlg.matchCase(), dlg.wholeWord(), dlg.searchSubDirs(), this);
                
                m_searchWorker->onResultFound = [this, searchNode](const SearchResultItem& res){
                    QStandardItem* fileItem = nullptr;
                    for (int i = 0; i < searchNode->rowCount(); ++i) {
                        if (searchNode->child(i)->text() == res.filePath) {
                            fileItem = searchNode->child(i);
                            break;
                        }
                    }
                    if (!fileItem) {
                        fileItem = new QStandardItem(res.filePath);
                        searchNode->appendRow(fileItem);
                    }
                    auto* lineItem = new QStandardItem(QString("Line %1: %2").arg(res.lineNum).arg(res.text));
                    lineItem->setData(QVariantList({res.lineNum, res.column, res.length}), Qt::UserRole);
                    fileItem->appendRow(lineItem);
                    m_searchTree->expand(fileItem->index());
                    m_searchTree->expand(searchNode->index());
                };
                
                m_searchWorker->onProgressUpdated = [this](int count, const QString& path){
                    m_statusWidget->showMessage(QString("Searching... scanned %1 files").arg(count), 0);
                };
                
                m_searchWorker->onSearchFinished = [this, searchNode](int files, int matches){
                    m_statusWidget->showMessage(QString("Search complete: %1 matches in %2 files").arg(matches).arg(files), 4000);
                    searchNode->setText(searchNode->text() + QString(" - %1 matches").arg(matches));
                    m_searchWorker->deleteLater();
                    m_searchWorker = nullptr;
                };
                
                m_searchWorker->start();
            });
            dlg.exec();
        }, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
        addAct(search, "Find &Next",    [this](){ doFind(false); }, QKeySequence::FindNext);
        addAct(search, "Find &Previous",[this](){ doFind(true); },  QKeySequence::FindPrevious);
        search->addSeparator();
        addAct(search, "&Go to Line...", [this](){
            bool ok;
            int ln = QInputDialog::getInt(this, "Go to Line", "Line number:", 1, 1, 999999, 1, &ok);
            if (ok) {
                if (auto* p = currentPanel()) {
                    sptr_t pos = p->editor()->send(SCI_POSITIONFROMLINE, ln - 1);
                    p->editor()->send(SCI_GOTOPOS, pos);
                }
            }
        }, QKeySequence(Qt::CTRL | Qt::Key_G));
        
        search->addSeparator();
        addAct(search, "Toggle Bookmark", [this](){
            if (auto* p = currentPanel()) {
                sptr_t line = p->editor()->send(SCI_LINEFROMPOSITION, p->editor()->send(SCI_GETCURRENTPOS));
                if (p->editor()->send(SCI_MARKERGET, line) & (1 << 1)) {
                    p->editor()->send(SCI_MARKERDELETE, line, 1);
                } else {
                    p->editor()->send(SCI_MARKERADD, line, 1);
                }
            }
        }, QKeySequence(Qt::CTRL | Qt::Key_F2));
        
        addAct(search, "Next Bookmark", [this](){
            if (auto* p = currentPanel()) {
                sptr_t line = p->editor()->send(SCI_LINEFROMPOSITION, p->editor()->send(SCI_GETCURRENTPOS));
                sptr_t next = p->editor()->send(SCI_MARKERNEXT, line + 1, (1 << 1));
                if (next == -1) next = p->editor()->send(SCI_MARKERNEXT, 0, (1 << 1));
                if (next != -1) p->editor()->send(SCI_GOTOLINE, next);
            }
        }, QKeySequence(Qt::Key_F2));

        addAct(search, "Previous Bookmark", [this](){
            if (auto* p = currentPanel()) {
                sptr_t line = p->editor()->send(SCI_LINEFROMPOSITION, p->editor()->send(SCI_GETCURRENTPOS));
                sptr_t prev = p->editor()->send(SCI_MARKERPREVIOUS, line - 1, (1 << 1));
                if (prev == -1) prev = p->editor()->send(SCI_MARKERPREVIOUS, p->editor()->send(SCI_GETLINECOUNT), (1 << 1));
                if (prev != -1) p->editor()->send(SCI_GOTOLINE, prev);
            }
        }, QKeySequence(Qt::SHIFT | Qt::Key_F2));

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

        addAct(view, "Distraction Free Mode", [this](){
            toggleDistractionFree();
        }, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
        
        addAct(view, "Command Palette...", [this](){
            showCommandPalette();
        }, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
        
        view->addSeparator();
        auto* wrapAct = view->addAction("Word &Wrap");
        wrapAct->setCheckable(true);
        connect(wrapAct, &QAction::toggled, this, [this](bool on){
            if (auto* p = currentPanel()) {
                p->setWordWrap(on);
            }
        });
        view->addSeparator();
        auto* functionListAct = view->addAction("Function List");
        functionListAct->setCheckable(true);
        connect(functionListAct, &QAction::toggled, this, [this](bool on){
            if (m_functionDock) m_functionDock->setVisible(on);
        });
        connect(m_functionDock, &QDockWidget::visibilityChanged, functionListAct, &QAction::setChecked);
        
        auto* folderSpaceAct = view->addAction("Folder as Workspace");
        folderSpaceAct->setCheckable(true);
        connect(folderSpaceAct, &QAction::toggled, this, [this](bool on){
            if (m_folderDock) m_folderDock->setVisible(on);
        });
        connect(m_folderDock, &QDockWidget::visibilityChanged, folderSpaceAct, &QAction::setChecked);

        auto* terminalAct = view->addAction("Terminal");
        terminalAct->setCheckable(true);
        connect(terminalAct, &QAction::toggled, this, [this](bool on){
            if (m_terminalDock) m_terminalDock->setVisible(on);
        });
        connect(m_terminalDock, &QDockWidget::visibilityChanged, terminalAct, &QAction::setChecked);

        view->addSeparator();
        // Bubbles toggle
        auto* bubbleAct = view->addAction("Bubble Animations");
        bubbleAct->setCheckable(true);
        bubbleAct->setChecked(GlassSettings::instance().bubbleAnimations());
        connect(bubbleAct, &QAction::toggled, this, [this](bool on){
            if (m_bubbles) m_bubbles->setVisible(on);
            GlassSettings::instance().setBubbleAnimations(on);
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
        
        m_macroStartAct = macro->addAction("Start Recording", [this](){
            m_isRecordingMacro = true;
            m_macroCommands.clear();
            m_macroStartAct->setEnabled(false);
            m_macroStopAct->setEnabled(true);
            m_macroPlayAct->setEnabled(false);
            // Tell Scintilla to start recording
            if (auto* p = currentPanel()) p->editor()->send(SCI_STARTRECORD);
            m_statusWidget->showMessage("Macro recording started...", 0); 
        }, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
        
        m_macroStopAct = macro->addAction("Stop Recording", [this](){
            m_isRecordingMacro = false;
            m_macroStartAct->setEnabled(true);
            m_macroStopAct->setEnabled(false);
            m_macroPlayAct->setEnabled(!m_macroCommands.isEmpty());
            // Tell Scintilla to stop recording
            if (auto* p = currentPanel()) p->editor()->send(SCI_STOPRECORD);
            
            // Save to shortcuts.xml
            GlassConfigXML::saveMacros("shortcuts.xml", m_macroCommands);
            
            m_statusWidget->showMessage(QString("Macro recording stopped (%1 actions) - Saved to shortcuts.xml").arg(m_macroCommands.size()), 3000);
        }, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
        m_macroStopAct->setEnabled(false); // Default off
        
        m_macroPlayAct = macro->addAction("Playback", [this](){
            if (auto* p = currentPanel()) {
                p->editor()->send(SCI_BEGINUNDOACTION);
                for (const auto& cmd : m_macroCommands) {
                    p->editor()->send(cmd.msg, cmd.wp, cmd.lp);
                }
                p->editor()->send(SCI_ENDUNDOACTION);
            }
            m_statusWidget->showMessage("Macro played", 2000);
        }, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
        m_macroPlayAct->setEnabled(false); // Default off
        
        macro->addSeparator();
        macro->addAction("Save Macro...");
        macro->addAction("Load Macro...");

        // ─── RUN ──────────────────────────────────────────────────────────
        QMenu* run = menuBar()->addMenu("&Run");
        run->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        addAct(run, "&Run...", [this](){
            m_statusWidget->showMessage("Run — coming soon", 2000); }, QKeySequence(Qt::Key_F5));

        // ─── PLUGINS ──────────────────────────────────────────────────────
        m_pluginsMenu = menuBar()->addMenu("&Plugins");
        m_pluginsMenu->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        updatePluginsMenu();

        // ─── WINDOW ───────────────────────────────────────────────────────
        QMenu* win = menuBar()->addMenu("&Window");
        win->setStyleSheet(LiquidGlassStyleSheet::kMenu);
        win->addSeparator();
        addAct(win, "Move to Other View", [this](){
            auto* twSrc = currentTabWidget();
            auto* twDst = (twSrc == m_tabs) ? m_tabsSecondary : m_tabs;
            int idx = twSrc->currentIndex();
            if (idx != -1) {
                auto* p = static_cast<GlassEditorPanel*>(twSrc->widget(idx));
                QString name = twSrc->tabText(idx);
                twSrc->removeTab(idx);
                p->setParent(twDst);
                twDst->addTab(p, name);
                twDst->show();
                twDst->setCurrentWidget(p);
            }
        });
        addAct(win, "Clone to Other View", [this](){
            auto* p = currentPanel();
            if (!p) return;
            auto* twDst = (currentTabWidget() == m_tabs) ? m_tabsSecondary : m_tabs;
            auto* np = new GlassEditorPanel(twDst);
            np->setText(p->text());
            np->setFilePath(p->filePath());
            twDst->addTab(np, QFileInfo(p->filePath()).fileName());
            twDst->show();
            twDst->setCurrentWidget(np);
            connectEditorSignals(np);
        });
        win->addSeparator();
        addAct(win, "Next Tab",     [this](){ 
            auto* tw = currentTabWidget();
            tw->setCurrentIndex((tw->currentIndex()+1) % tw->count()); 
        }, QKeySequence(Qt::CTRL | Qt::Key_PageDown));
        addAct(win, "Previous Tab", [this](){
            auto* tw = currentTabWidget();
            int n = tw->count();
            tw->setCurrentIndex((tw->currentIndex()+n-1) % n);
        }, QKeySequence(Qt::CTRL | Qt::Key_PageUp));
        
        win->addSeparator();
        auto* syncScrollAct = win->addAction("Synchronize Vertical Scrolling");
        syncScrollAct->setCheckable(true);
        connect(syncScrollAct, &QAction::toggled, this, [this](bool on){
            m_syncScrollingV = on;
        });

        auto* syncScrollHAct = win->addAction("Synchronize Horizontal Scrolling");
        syncScrollHAct->setCheckable(true);
        connect(syncScrollHAct, &QAction::toggled, this, [this](bool on){
            m_syncScrollingH = on;
        });
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
            GlassPreferencesDialog dlg(this);
            dlg.setApplyCallback([this](){
                auto& s = GlassSettings::instance();
                // Apply glass intensity
                static const int modes[] = { 1, 3, 2 }; // Mica, MicaTabbed, Acrylic
                LiquidGlassDWM::enableBlurBehind(this, modes[s.glassIntensity()]);
                // Apply bubbles
                if (m_bubbles) m_bubbles->setVisible(s.bubbleAnimations());
                // Apply to all tabs
                for (int i = 0; i < m_tabs->count(); ++i) {
                    if (auto* p = static_cast<GlassEditorPanel*>(m_tabs->widget(i))) {
                        p->editor()->setFont(s.editorFont());
                        p->setWordWrap(s.wordWrapDefault());
                    }
                }
            });
            dlg.exec();
        });
        settings->addAction("Style Configurator...", [this](){
            GlassStyleConfigurator dlg(this);
            dlg.exec();
        });
        settings->addAction("Shortcut Mapper...",    [this](){
            GlassShortcutMapper dlg(this);
            dlg.exec();
        });
        settings->addSeparator();
        // Glass intensity toggle (quick access)
        auto* intensityAct = settings->addAction("Cycle Glass Intensity");
        connect(intensityAct, &QAction::triggered, this, [this](){
            auto& s = GlassSettings::instance();
            int level = (s.glassIntensity() + 1) % 3;
            s.setGlassIntensity(level);
            static const char* names[] = {"Low (Mica)","Medium (MicaTabbed)","High (Acrylic)"};
            static const int modes[] = { 1, 3, 2 };
            LiquidGlassDWM::enableBlurBehind(this, modes[level]);
            m_statusWidget->showMessage(QString("Glass intensity: %1").arg(names[level]), 2000);
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

    void updatePluginsMenu() {
        if (!m_pluginsMenu) return;
        m_pluginsMenu->clear();
        
        m_pluginsMenu->addAction("Plugin Admin...", [this](){
            m_statusWidget->showMessage("Plugin Admin — coming soon", 2000);
        });
        m_pluginsMenu->addSeparator();
        
        // List loaded plugins
        const auto& loaded = GlassPluginManager::instance().plugins();
        for (const auto& gp : loaded) {
            m_pluginsMenu->addAction(gp.name);
        }
        
        if (loaded.isEmpty()) {
            auto* a = m_pluginsMenu->addAction("No plugins loaded");
            a->setEnabled(false);
        }

        m_pluginsMenu->addSeparator();
        m_pluginsMenu->addAction("Open Plugins Folder...", [this](){
            m_statusWidget->showMessage("Plugins directory mapped.", 2000);
        });
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
    QTabWidget*      m_tabsSecondary;
    QSplitter*       m_mainSplitter;
    GlassStatusWidget* m_statusWidget;
    BubbleOverlay*   m_bubbles;
    GlassFindDialog* m_findDialog;

    // Macro state
    bool m_isRecordingMacro = false;
    QList<MacroCommand> m_macroCommands;
    QAction* m_macroStartAct = nullptr;
    QAction* m_macroStopAct = nullptr;
    QAction* m_macroPlayAct = nullptr;
    
    QDockWidget*     m_folderDock = nullptr;
    QDockWidget*     m_functionDock = nullptr;
    QDockWidget*     m_searchDock = nullptr;
    QTreeView*       m_searchTree = nullptr;
    QStandardItemModel* m_searchModel = nullptr;
    QStandardItemModel* m_folderModel = nullptr;
    GlassSearchWorker* m_searchWorker = nullptr;
    QDockWidget*     m_terminalDock = nullptr;
    QMenu*           m_pluginsMenu = nullptr;
    QSystemTrayIcon* m_trayIcon = nullptr;
    QTimer*          m_backupTimer = nullptr;
    bool             m_syncScrollingV = false;
    bool             m_syncScrollingH = false;
};


// ─────────────────────────────────────────────────────────────────────────────
// MAIN ENTRY POINT
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    // Register Native Scintilla Window Class
#ifdef Q_OS_WIN
    Scintilla_RegisterClasses(GetModuleHandle(NULL));
#endif

    // Load Default Liquid Glass Theme
    GlassThemeManager::instance().loadTheme("PowerEditor/src/stylers.xml");

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

    // Launch Splash Screen
    GlassSplashScreen splash;
    splash.show();
    splash.setProgress(10);
    app.processEvents();

    // Register Native Scintilla Window Class
#ifdef Q_OS_WIN
    splash.setMessage("Registering Scintilla components...");
    splash.setProgress(30);
    app.processEvents();
    Scintilla_RegisterClasses(GetModuleHandle(NULL));
#endif

    // Load Default Liquid Glass Theme
    splash.setMessage("Loading Liquid Glass themes...");
    splash.setProgress(60);
    app.processEvents();
    GlassThemeManager::instance().loadTheme("PowerEditor/src/stylers.xml");

    // Apply the global liquid glass stylesheet
    splash.setMessage("Finalizing UI layout...");
    splash.setProgress(90);
    app.processEvents();
    app.setStyleSheet(LiquidGlassStyleSheet::kFullGlassTheme());

    // Launch the main window
    BobNppGlassWindow win;
    win.show();

    splash.finish(&win);

    return app.exec();
}
