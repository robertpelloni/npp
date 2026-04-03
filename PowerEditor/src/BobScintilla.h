#ifndef BOB_SCINTILLA_H
#define BOB_SCINTILLA_H

#include <QWidget>
#include <QResizeEvent>
#include <QWindow>
#include <QString>
#include <QByteArray>
#include <QDebug>
#include <windows.h>
#include "Scintilla.h"
#include "Sci_Position.h"
#include "ILexer.h"
#include "Lexilla.h"
#include "SciLexer.h"
#include "GlassThemeManager.h"
#include "GlassPluginManager.h"

// Note: To compile this, the target must link against Scintilla and Lexilla.

class BobScintilla : public QWidget {
public:
    explicit BobScintilla(QWidget* parent = nullptr) : QWidget(parent) {
        // We need a native window handle for the parent
        setAttribute(Qt::WA_NativeWindow);
        
        HINSTANCE hInstance = GetModuleHandle(NULL);
        
        m_sciHwnd = CreateWindowExW(
            0,
            L"Scintilla",
            L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN,
            0, 0, 100, 100,
            reinterpret_cast<HWND>(winId()),
            NULL,
            hInstance,
            NULL
        );

        if (!m_sciHwnd) {
            qWarning() << "Failed to create Scintilla HWND!";
            return;
        }

        // Set modern DirectWrite technology for better rendering
        send(SCI_SETTECHNOLOGY, SC_TECHNOLOGY_DIRECTWRITE);
        
        // Setup initial styling (Dark mode defaults to blend with glass)
        send(SCI_STYLESETBACK, STYLE_DEFAULT, 0x1A140E); // BGR format (dark navy)
        send(SCI_STYLESETFORE, STYLE_DEFAULT, 0xEBEBEB);
        send(SCI_STYLECLEARALL, 0, 0);

        send(SCI_SETCARETFORE, 0xFFFFFF, 0);
        send(SCI_SETSELBACK, 1, 0x805A20); // BGR for #205A80
        
        // Margins
        send(SCI_SETMARGINWIDTHN, 0, 45); // Line number margin
        send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0x1A140E);
        send(SCI_STYLESETFORE, STYLE_LINENUMBER, 0x888888);
        
        // Margin 1 (Bookmarks/Symbols)
        send(SCI_SETMARGINWIDTHN, 1, 14);
        send(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
        send(SCI_SETMARGINMASKN, 1, ~SC_MASK_FOLDERS);
        send(SCI_SETMARGINSENSITIVEN, 1, 1);

        // Margin 2 (Code Folding)
        send(SCI_SETMARGINWIDTHN, 2, 14);
        send(SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);
        send(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
        send(SCI_SETMARGINSENSITIVEN, 2, 1);
        send(SCI_SETFOLDMARGINCOLOUR, 1, 0x1A140E);
        send(SCI_SETFOLDMARGINHICOLOUR, 1, 0x1A140E);
        
        // Define Liquid Glass folder symbols (sleek, minimalistic box lines)
        send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
        send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
        send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
        send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
        send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
        send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
        send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
        
        // Bookmark marker (ID 1)
        send(SCI_MARKERDEFINE, 1, SC_MARK_CIRCLE);
        send(SCI_MARKERSETFORE, 1, 0xFFFFFF);
        send(SCI_MARKERSETBACK, 1, 0xFF8040); // Blue glass bookmark (BGR: FF 80 40)

        // Style them
        for (int i = SC_MARKNUM_FOLDEREND; i <= SC_MARKNUM_FOLDEROPEN; i++) {
            send(SCI_MARKERSETFORE, i, 0x1A140E); // Match background
            send(SCI_MARKERSETBACK, i, 0x88AAFF); // Subtle blue/cyan for glass aesthetic
            send(SCI_MARKERSETBACKSELECTED, i, 0xFFFFFF);
        }
        
        // Enable Multi-Selection and Rectangular Selection (Column Mode)
        send(SCI_SETMULTIPLESELECTION, 1);
        send(SCI_SETADDITIONALSELECTIONTYPING, 1);
        send(SCI_SETVIRTUALSPACEOPTIONS, SCVS_RECTANGULARSELECTION);

        // Caret line highlighting
        send(SCI_SETCARETLINEVISIBLE, 1);
        send(SCI_SETCARETLINEBACK, 0xFFFFFF); // White
        send(SCI_SETCARETLINEBACKALPHA, 20); // Very subtle
        
        // Enable folding in lexers
        send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), reinterpret_cast<sptr_t>("1"));
        send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold.compact"), reinterpret_cast<sptr_t>("0"));

        // Style Autocomplete and Calltips to match Liquid Glass
        send(SCI_CALLTIPUSESTYLE, 0); // Use Scintilla styles for calltips
        send(SCI_STYLESETBACK, STYLE_CALLTIP, 0x2A241E); // Slightly lighter navy
        send(SCI_STYLESETFORE, STYLE_CALLTIP, 0xEBEBEB);
        
        // Autocompletion lists
        send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_LIST, 0xFF1E242A); // ABGR (requires alpha for SC_ELEMENT)
        send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_LIST_BACK, 0xFF1E242A);
        send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_LIST_SELECTED, 0x80FFAA88); // Semi-transparent blue
        send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_LIST_SELECTED_BACK, 0x80FFAA88);

        // Word wrap off by default
        send(SCI_SETWRAPMODE, SC_WRAP_NONE);
    }

    ~BobScintilla() {
        if (m_sciHwnd) {
            DestroyWindow(m_sciHwnd);
        }
    }

    sptr_t send(unsigned int msg, uptr_t wp = 0, sptr_t lp = 0) const {
        if (!m_sciHwnd) return 0;
        return SendMessageW(m_sciHwnd, msg, wp, lp);
    }

    void setText(const QString& text) {
        QByteArray utf8 = text.toUtf8();
        send(SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(utf8.constData()));
    }

    QString text() const {
        sptr_t len = send(SCI_GETLENGTH);
        if (len == 0) return QString();
        
        QByteArray buffer;
        buffer.resize(static_cast<int>(len + 1));
        send(SCI_GETTEXT, len + 1, reinterpret_cast<sptr_t>(buffer.data()));
        buffer.resize(static_cast<int>(len)); // Remove null terminator
        
        return QString::fromUtf8(buffer);
    }

    void setWordWrap(bool wrap) {
        send(SCI_SETWRAPMODE, wrap ? SC_WRAP_WORD : SC_WRAP_NONE);
    }
    bool wordWrap() const {
        return send(SCI_GETWRAPMODE) != SC_WRAP_NONE;
    }

    void undo() { send(SCI_UNDO); }
    void redo() { send(SCI_REDO); }
    void cut()  { send(SCI_CUT); }
    void copy() { send(SCI_COPY); }
    void paste(){ send(SCI_PASTE); }
    void selectAll() { send(SCI_SELECTALL); }

    HWND getHwnd() const { return m_sciHwnd; }

    void highlightRange(sptr_t start, sptr_t length, int indicator = 8) {
        // Use specified indicator
        send(SCI_SETINDICATORCURRENT, indicator);
        if (indicator == 8) {
            send(SCI_INDICSETSTYLE, 8, INDIC_ROUNDBOX);
            send(SCI_INDICSETFORE, 8, 0x00A0FF); // Electric orange
            send(SCI_INDICSETALPHA, 8, 100);
            send(SCI_INDICSETOUTLINEALPHA, 8, 200);
        } else if (indicator == 9) {
            send(SCI_INDICSETSTYLE, 9, INDIC_STRAIGHTBOX);
            send(SCI_INDICSETFORE, 9, 0x00FF00); // Green for Mark All
            send(SCI_INDICSETALPHA, 9, 60);
        }
        
        // Fill new
        send(SCI_INDICATORFILLRANGE, start, length);
    }

    void clearIndicator(int indicator = 8) {
        send(SCI_SETINDICATORCURRENT, indicator);
        send(SCI_INDICATORCLEARRANGE, 0, send(SCI_GETLENGTH));
    }

    void selectAllOccurrences() {
        sptr_t selStart = send(SCI_GETSELECTIONSTART);
        sptr_t selEnd = send(SCI_GETSELECTIONEND);
        if (selStart == selEnd) return;

        sptr_t len = selEnd - selStart;
        QByteArray needle;
        needle.resize(static_cast<int>(len));
        
        Sci_TextRangeFull tr;
        tr.chrg.cpMin = static_cast<Sci_Position>(selStart);
        tr.chrg.cpMax = static_cast<Sci_Position>(selEnd);
        tr.lpstrText = needle.data();
        send(SCI_GETTEXTRANGEFULL, 0, reinterpret_cast<sptr_t>(&tr));

        send(SCI_SETTARGETSTART, 0);
        send(SCI_SETTARGETEND, send(SCI_GETLENGTH));
        send(SCI_SETSEARCHFLAGS, SCFIND_MATCHCASE);

        bool first = true;
        while (send(SCI_SEARCHINTARGET, needle.length(), reinterpret_cast<sptr_t>(needle.constData())) != -1) {
            sptr_t matchStart = send(SCI_GETTARGETSTART);
            sptr_t matchEnd = send(SCI_GETTARGETEND);
            if (first) {
                send(SCI_SETSEL, matchStart, matchEnd);
                first = false;
            } else {
                send(SCI_ADDSELECTION, matchEnd, matchStart);
            }
            send(SCI_SETTARGETSTART, matchEnd);
            send(SCI_SETTARGETEND, send(SCI_GETLENGTH));
        }
    }

    // Dummy properties for font compatibility with settings
    void setFont(const QFont& f) {
        QWidget::setFont(f);
        QByteArray name = f.family().toUtf8();
        send(SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<sptr_t>(name.constData()));
        send(SCI_STYLESETSIZE, STYLE_DEFAULT, f.pointSize());
        send(SCI_STYLECLEARALL);
        
        // Re-apply Liquid Glass text colors after clearall
        send(SCI_STYLESETBACK, STYLE_DEFAULT, 0x1A140E); // BGR dark navy
        send(SCI_STYLESETFORE, STYLE_DEFAULT, 0xEBEBEB); // BGR off-white
        send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0x1A140E);
        send(SCI_STYLESETFORE, STYLE_LINENUMBER, 0x888888);
        
        // Re-apply specific lexer colors if already set
        if (!m_currentExt.isEmpty()) {
            applyLexer(m_currentExt);
        }
    }

    void applyLexer(const QString& ext) {
        m_currentExt = ext.toLower();
        QString lexerName;
        
        if (m_currentExt == "cpp" || m_currentExt == "c" || m_currentExt == "h" || m_currentExt == "hpp") {
            lexerName = "cpp";
        } else if (m_currentExt == "py") {
            lexerName = "python";
        } else if (m_currentExt == "html" || m_currentExt == "htm" || m_currentExt == "xml") {
            lexerName = "html";
        } else if (m_currentExt == "js" || m_currentExt == "ts" || m_currentExt == "json") {
            lexerName = "cpp"; // cpp lexer is often used for JS in Scintilla
        } else if (m_currentExt == "md" || m_currentExt == "markdown") {
            lexerName = "markdown";
        } else if (m_currentExt == "css") {
            lexerName = "css";
        } else if (m_currentExt == "sh" || m_currentExt == "bash") {
            lexerName = "bash";
        } else {
            lexerName = "null";
        }

        ILexer5* lexer = CreateLexer(lexerName.toUtf8().constData());
        if (lexer) {
            send(SCI_SETILEXER, 0, reinterpret_cast<sptr_t>(lexer));
            
            // Try to load from Theme Manager
            auto styles = GlassThemeManager::instance().getStyles(lexerName);
            // Also get global styles for defaults
            auto global = GlassThemeManager::instance().getStyles("global");
            
            int bg = 0x1A140E;
            int fg = 0xEBEBEB;

            // Apply global first
            for (const auto& s : global) {
                if (s.fg.isValid()) send(SCI_STYLESETFORE, s.id, GlassThemeManager::toSciColor(s.fg));
                if (s.bg.isValid()) send(SCI_STYLESETBACK, s.id, GlassThemeManager::toSciColor(s.bg));
                send(SCI_STYLESETBOLD, s.id, s.bold ? 1 : 0);
                send(SCI_STYLESETITALIC, s.id, s.italic ? 1 : 0);
            }

            if (!styles.isEmpty()) {
                for (const auto& s : styles) {
                    if (s.fg.isValid()) send(SCI_STYLESETFORE, s.id, GlassThemeManager::toSciColor(s.fg));
                    if (s.bg.isValid()) send(SCI_STYLESETBACK, s.id, GlassThemeManager::toSciColor(s.bg));
                    send(SCI_STYLESETBOLD, s.id, s.bold ? 1 : 0);
                    send(SCI_STYLESETITALIC, s.id, s.italic ? 1 : 0);
                }
            } else {
                // Fallback for hardcoded mapping if XML is missing or doesn't have this lexer
                int comment = 0x608060; // green-ish
                int keyword = 0xFF9040; // orange/gold
                int string = 0x60C0FF;  // yellow-ish
                int number = 0xA0A0FF;  // cyan-ish
                int op = 0xFFD080;      // light blue-ish
                int preproc = 0xA060C0; // purple-ish
                
                if (lexerName == "cpp") {
                    send(SCI_STYLESETFORE, SCE_C_DEFAULT, fg);
                    send(SCI_STYLESETFORE, SCE_C_COMMENT, comment);
                    send(SCI_STYLESETFORE, SCE_C_COMMENTLINE, comment);
                    send(SCI_STYLESETFORE, SCE_C_COMMENTDOC, comment);
                    send(SCI_STYLESETFORE, SCE_C_NUMBER, number);
                    send(SCI_STYLESETFORE, SCE_C_WORD, keyword);
                    send(SCI_STYLESETBOLD, SCE_C_WORD, 1);
                    send(SCI_STYLESETFORE, SCE_C_STRING, string);
                    send(SCI_STYLESETFORE, SCE_C_CHARACTER, string);
                    send(SCI_STYLESETFORE, SCE_C_OPERATOR, op);
                    send(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, preproc);
                } else if (lexerName == "python") {
                    send(SCI_STYLESETFORE, SCE_P_DEFAULT, fg);
                    send(SCI_STYLESETFORE, SCE_P_COMMENTLINE, comment);
                    send(SCI_STYLESETFORE, SCE_P_NUMBER, number);
                    send(SCI_STYLESETFORE, SCE_P_STRING, string);
                    send(SCI_STYLESETFORE, SCE_P_CHARACTER, string);
                    send(SCI_STYLESETFORE, SCE_P_WORD, keyword);
                    send(SCI_STYLESETBOLD, SCE_P_WORD, 1);
                    send(SCI_STYLESETFORE, SCE_P_TRIPLE, string);
                    send(SCI_STYLESETFORE, SCE_P_TRIPLEDOUBLE, string);
                    send(SCI_STYLESETFORE, SCE_P_CLASSNAME, preproc);
                    send(SCI_STYLESETFORE, SCE_P_DEFNAME, op);
                    send(SCI_STYLESETFORE, SCE_P_OPERATOR, op);
                } else if (lexerName == "html") {
                    send(SCI_STYLESETFORE, SCE_H_DEFAULT, fg);
                    send(SCI_STYLESETFORE, SCE_H_TAG, keyword);
                    send(SCI_STYLESETFORE, SCE_H_TAGUNKNOWN, keyword);
                    send(SCI_STYLESETFORE, SCE_H_ATTRIBUTE, op);
                    send(SCI_STYLESETFORE, SCE_H_ATTRIBUTEUNKNOWN, op);
                    send(SCI_STYLESETFORE, SCE_H_NUMBER, number);
                    send(SCI_STYLESETFORE, SCE_H_DOUBLESTRING, string);
                    send(SCI_STYLESETFORE, SCE_H_SINGLESTRING, string);
                    send(SCI_STYLESETFORE, SCE_H_OTHER, fg);
                    send(SCI_STYLESETFORE, SCE_H_COMMENT, comment);
                }
            }

            // Set keywords for C++/Python if not handled by XML
            if (lexerName == "cpp") {
                send(SCI_SETKEYWORDS, 0, reinterpret_cast<sptr_t>(
                    "auto break case char const continue default do double else enum extern float for goto if int long register return short signed sizeof static struct switch typedef union unsigned void volatile while "
                    "asm bool catch class const_cast delete dynamic_cast explicit export false friend inline mutable namespace new operator private protected public reinterpret_cast static_cast template this throw true try typeid typename using virtual wchar_t "
                    "alignas alignof char16_t char32_t constexpr decltype noexcept nullptr static_assert thread_local"
                ));
            } else if (lexerName == "python") {
                send(SCI_SETKEYWORDS, 0, reinterpret_cast<sptr_t>(
                    "False None True and as assert async await break class continue def del elif else except finally for from global if import in is lambda nonlocal not or pass raise return try while with yield"
                ));
            }

            // For all lexers, ensure background is consistent
            for (int i = 0; i <= STYLE_MAX; ++i) {
                send(SCI_STYLESETBACK, i, bg);
            }
        }
    }

    std::function<void()> onCursorPositionChanged;
    std::function<void()> onContentsChanged;
    std::function<void(bool)> onModificationChanged;

    std::function<void(unsigned int, uptr_t, sptr_t)> onMacroRecord;

    std::function<void()> onScroll;
    std::function<void(int)> onCharAdded;

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_NOTIFY) {
            SCNotification* scn = reinterpret_cast<SCNotification*>(msg->lParam);
            if (scn->nmhdr.hwndFrom == m_sciHwnd) {
                // Notify Plugins
                GlassPluginManager::instance().notifyAll(scn);

                // Macro recording
                if (onMacroRecord && scn->nmhdr.code == SCN_MACRORECORD) {
                    onMacroRecord(scn->message, scn->wParam, scn->lParam);
                }

                if (scn->nmhdr.code == SCN_MODIFIED) {
                    if (onContentsChanged) onContentsChanged();
                    if (onModificationChanged) onModificationChanged(true);
                } else if (scn->nmhdr.code == SCN_UPDATEUI) {
                    if (onCursorPositionChanged) onCursorPositionChanged();
                    if (onScroll) onScroll();
                } else if (scn->nmhdr.code == SCN_CHARADDED) {
                    if (onCharAdded) onCharAdded(scn->ch);
                    
                    // Auto-insert matching braces/quotes
                    char ch = static_cast<char>(scn->ch);
                    sptr_t pos = send(SCI_GETCURRENTPOS);
                    if (ch == '(') { send(SCI_INSERTTEXT, pos, reinterpret_cast<sptr_t>(")")); }
                    else if (ch == '{') { send(SCI_INSERTTEXT, pos, reinterpret_cast<sptr_t>("}")); }
                    else if (ch == '[') { send(SCI_INSERTTEXT, pos, reinterpret_cast<sptr_t>("]")); }
                    else if (ch == '"') { send(SCI_INSERTTEXT, pos, reinterpret_cast<sptr_t>("\"")); }
                    else if (ch == '\'') { send(SCI_INSERTTEXT, pos, reinterpret_cast<sptr_t>("'")); }
                    else if (ch == '>') {
                        // Auto-close HTML/XML tags
                        if (m_currentExt == "html" || m_currentExt == "xml") {
                            sptr_t start = send(SCI_WORDSTARTPOSITION, pos - 1, true);
                            Sci_TextRangeFull tr;
                            QByteArray tag; tag.resize(pos - 1 - start);
                            tr.chrg.cpMin = start; tr.chrg.cpMax = pos - 1; tr.lpstrText = tag.data();
                            send(SCI_GETTEXTRANGEFULL, 0, reinterpret_cast<sptr_t>(&tr));
                            if (!tag.isEmpty()) {
                                QString closeTag = "</" + QString::fromUtf8(tag) + ">";
                                send(SCI_INSERTTEXT, pos, reinterpret_cast<sptr_t>(closeTag.toUtf8().constData()));
                            }
                        }
                    }

                    // Simple C++ / Python Autocomplete Trigger
                    if (isalpha(ch) || ch == '_') {
                        sptr_t pos = send(SCI_GETCURRENTPOS);
                        sptr_t start = send(SCI_WORDSTARTPOSITION, pos, true);
                        sptr_t len = pos - start;
                        if (len == 3) {
                            if (m_currentExt == "cpp" || m_currentExt == "h") {
                                send(SCI_AUTOCSHOW, len, reinterpret_cast<sptr_t>("alignas alignof auto bool break case catch char class const constexpr continue decltype default delete double else enum explicit export extern false float for friend goto if inline int long mutable namespace new noexcept nullptr operator private protected public register reinterpret_cast return short signed sizeof static static_assert static_cast struct switch template this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while"));
                            } else if (m_currentExt == "py") {
                                send(SCI_AUTOCSHOW, len, reinterpret_cast<sptr_t>("False None True and as assert async await break class continue def del elif else except finally for from global if import in is lambda nonlocal not or pass raise return try while with yield"));
                            }
                        }
                    }
                } else if (scn->nmhdr.code == SCN_MARGINCLICK) {
                    if (scn->margin == 2) {
                        sptr_t line = send(SCI_LINEFROMPOSITION, scn->position);
                        send(SCI_TOGGLEFOLD, line);
                    }
                }
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }

    void resizeEvent(QResizeEvent* e) override {
        QWidget::resizeEvent(e);
        if (m_sciHwnd) {
            MoveWindow(m_sciHwnd, 0, 0, width(), height(), TRUE);
        }
    }

private:
    HWND m_sciHwnd = nullptr;
    QString m_currentExt;
};

#endif // BOB_SCINTILLA_H
