/**
 * LiquidGlass.h — BobUI Liquid Glass Rendering System
 *
 * This header provides the comprehensive "Liquid Glass" visual system for
 * Notepad++ powered by BobUI (Qt6). The design is inspired by Apple's
 * visionOS/macOS "liquid glass" material — a dynamic, translucent surface
 * with frosted-glass blur, specular light rim, refraction distortion, and
 * multi-layer compositing.
 *
 * Architecture:
 *   - LiquidGlassPalette    : Centralized color/opacity token system
 *   - LiquidGlassDWM        : Windows DWM blur-behind and HWND composition
 *   - LiquidGlassEffect     : QGraphicsEffect subclass for per-widget blur/tint
 *   - LiquidGlassPainter    : Static painter utilities (rim, specular, gradient)
 *   - LiquidGlassStyleSheet : Pre-built QSS strings for common widget types
 *
 * Design tokens are intentionally grouped here so any future theme engine
 * or QML binding can reference a single source of truth.
 *
 * Author: Antigravity (Google DeepMind Advanced Agentic Coding)
 * Date  : 2026-04-03
 * Version: integrated into npp BobUI overhaul
 */

#pragma once

#ifndef LIQUID_GLASS_H
#define LIQUID_GLASS_H

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <QStyleOption>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QPen>
#include <QBrush>
#include <QSizeF>
#include <QRectF>
#include <QPointF>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Windows DWM (Desktop Window Manager) integration.
// These allow us to extend glass into the non-client area (title bar) and
// enable real OS-level blur behind the window.
// ─────────────────────────────────────────────────────────────────────────────
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#endif

// ─────────────────────────────────────────────────────────────────────────────
// LIQUID GLASS PALETTE
// All color/opacity constants for the glass material system.
// Keep these in one place — themes are implemented by swapping the palette.
// ─────────────────────────────────────────────────────────────────────────────
namespace LiquidGlassPalette {

    // Base glass tint — semi-transparent cool white/blue.
    // In light mode use a brighter tint; dark mode uses the shadow variant.
    static const QColor kGlassTintLight     = QColor(255, 255, 255, 40);   // soft frosted white
    static const QColor kGlassTintDark      = QColor(20,  24,  35,  160);  // deep dark navy glass

    // Rim / specular highlight drawn along the top-left edge of glass panels.
    // Simulates light catching a curved glass edge.
    static const QColor kRimHighlight       = QColor(255, 255, 255, 90);   // bright specular rim
    static const QColor kRimShadow          = QColor(0,   0,   0,   60);   // shadow rim (bottom-right)

    // Gradient stops for the glass surface sheen (top→bottom sweep).
    static const QColor kSheenTop           = QColor(255, 255, 255, 55);
    static const QColor kSheenBottom        = QColor(255, 255, 255, 0);

    // Refraction tint: a subtle hue shift that simulates light bending.
    static const QColor kRefractionTint     = QColor(150, 180, 255, 18);

    // Blur strength (radius in pixels for the software Gaussian fallback).
    // Real DWM blur does not use this; it's for the QGraphicsBlurEffect path.
    static const int    kBlurRadius         = 28;

    // Opacity of the overall glass widget (0–255).
    // Note: WA_TranslucentBackground must be set for this to show the desktop.
    static const int    kGlassOpacity       = 210;

    // Corner radius for rounded glass panels (px).
    static const qreal  kCornerRadius       = 14.0;

    // Border opacity/width for the hairline glass border.
    static const QColor kBorderColor        = QColor(255, 255, 255, 50);
    static const qreal  kBorderWidth        = 1.0;

    // Tab bar active/inactive glass overrides.
    static const QColor kTabActive          = QColor(255, 255, 255, 30);
    static const QColor kTabInactive        = QColor(0,   0,   0,   80);

    // Menu/popup glass tint.
    static const QColor kMenuGlass          = QColor(20,  22,  30,  210);
    static const QColor kMenuHoverGlass     = QColor(90, 130, 255,  80);

    // Status bar glass.
    static const QColor kStatusBarGlass     = QColor(10,  12,  20,  180);

    // Accent glow color (used for focused widgets, active elements).
    static const QColor kAccentGlow         = QColor(80, 140, 255, 120);

    // Text on glass surfaces.
    static const QColor kTextPrimary        = QColor(240, 245, 255, 240);
    static const QColor kTextSecondary      = QColor(180, 190, 210, 190);
    static const QColor kTextDisabled       = QColor(120, 130, 150, 130);

} // namespace LiquidGlassPalette


// ─────────────────────────────────────────────────────────────────────────────
// LIQUID GLASS PAINTER  (static utilities)
// These helper functions can be called inside any paintEvent to draw the
// layered glass material. Order: blur → tint → sheen → refraction → rim.
// ─────────────────────────────────────────────────────────────────────────────
class LiquidGlassPainter {
public:
    /**
     * drawGlassBackground — primary glass surface render.
     *
     * Draws a layered, rounded glass rectangle.  Layers (back→front):
     *   1. Clear (transparent fill — the blur effect provides the actual
     *      background when WA_TranslucentBackground is active on the window).
     *   2. Dark/light tint overlay (kGlassTintDark).
     *   3. Vertical sheen gradient (kSheenTop → kSheenBottom).
     *   4. Refraction color smear (subtle hue tint).
     *   5. Specular rim (bright hairline on top-left edge).
     *   6. Shadow rim (subtle dark line on bottom-right edge).
     *   7. Hairline border (kBorderColor, 1px).
     *
     * @param painter  Active QPainter (called from paintEvent).
     * @param rect     The rectangle area to fill (widget's rect()).
     * @param radius   Corner radius (default: kCornerRadius).
     * @param dark     true = dark tint, false = light tint.
     */
    static void drawGlassBackground(
        QPainter&  painter,
        const QRectF& rect,
        qreal      radius = LiquidGlassPalette::kCornerRadius,
        bool       dark   = true)
    {
        painter.save();
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        // ── 1. Build the rounded rect clip path ──────────────────────────────
        QPainterPath clipPath;
        clipPath.addRoundedRect(rect, radius, radius);
        painter.setClipPath(clipPath);

        // ── 2. Base glass tint ───────────────────────────────────────────────
        // This tint sits on top of the blurred backdrop.
        painter.fillPath(clipPath, dark
            ? LiquidGlassPalette::kGlassTintDark
            : LiquidGlassPalette::kGlassTintLight);

        // ── 3. Vertical sheen gradient (simulates curved reflective surface) ─
        QLinearGradient sheen(rect.topLeft(), rect.bottomLeft());
        sheen.setColorAt(0.0,  LiquidGlassPalette::kSheenTop);
        sheen.setColorAt(0.45, LiquidGlassPalette::kSheenBottom);
        sheen.setColorAt(1.0,  Qt::transparent);
        painter.fillPath(clipPath, sheen);

        // ── 4. Refraction smear (radial, centered top-center) ────────────────
        // Simulates light bending as it passes through curved glass.
        QRadialGradient refraction(rect.center().x(), rect.top() + rect.height() * 0.2,
                                   rect.width() * 0.6);
        refraction.setColorAt(0.0, LiquidGlassPalette::kRefractionTint);
        refraction.setColorAt(1.0, Qt::transparent);
        painter.fillPath(clipPath, refraction);

        // ── 5. Specular rim — bright top edge ────────────────────────────────
        // Real glass has a bright specular highlight at the top-left rim where
        // light source catches the curved edge. We fake this with a thin gradient
        // that fades from bright white to transparent across ~20% of the height.
        QLinearGradient rimTop(rect.topLeft(), QPointF(rect.left(), rect.top() + rect.height() * 0.12));
        rimTop.setColorAt(0.0, LiquidGlassPalette::kRimHighlight);
        rimTop.setColorAt(1.0, Qt::transparent);
        painter.fillPath(clipPath, rimTop);

        // Left rim vertical highlight
        QLinearGradient rimLeft(rect.topLeft(), QPointF(rect.left() + rect.width() * 0.08, rect.top()));
        rimLeft.setColorAt(0.0, QColor(255, 255, 255, 45));
        rimLeft.setColorAt(1.0, Qt::transparent);
        painter.fillPath(clipPath, rimLeft);

        // ── 6. Shadow rim — subtle dark bottom-right ─────────────────────────
        QLinearGradient rimBottom(rect.bottomLeft(), QPointF(rect.left(), rect.bottom() - rect.height() * 0.08));
        rimBottom.setColorAt(0.0, LiquidGlassPalette::kRimShadow);
        rimBottom.setColorAt(1.0, Qt::transparent);
        painter.fillPath(clipPath, rimBottom);

        // ── 7. Hairline border ────────────────────────────────────────────────
        painter.setClipping(false);
        QPen border(LiquidGlassPalette::kBorderColor, LiquidGlassPalette::kBorderWidth);
        painter.setPen(border);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);

        painter.restore();
    }

    /**
     * drawAccentGlow — draws a soft colored glow beneath a focused widget.
     * Used for: focused input fields, active tab indicators, hover states.
     *
     * @param painter  Active QPainter.
     * @param rect     Widget bounds.
     * @param color    Glow color (default: kAccentGlow).
     * @param spread   How far the glow spreads outward (pixels).
     */
    static void drawAccentGlow(
        QPainter&     painter,
        const QRectF& rect,
        const QColor& color  = LiquidGlassPalette::kAccentGlow,
        qreal         spread = 16.0)
    {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);

        QRadialGradient glow(rect.center(), std::max(rect.width(), rect.height()) * 0.5 + spread);
        glow.setColorAt(0.0, color);
        QColor transparent = color;
        transparent.setAlpha(0);
        glow.setColorAt(1.0, transparent);

        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(rect.adjusted(-spread, -spread, spread, spread));
        painter.restore();
    }

    /**
     * drawLiquidDrop — draws a teardrop/bubble shape as a decorative accent.
     * Used for animated splash effects or decorative glass bubble overlays.
     */
    static void drawLiquidDrop(
        QPainter&     painter,
        const QPointF& center,
        qreal          radius,
        const QColor&  tint = QColor(100, 160, 255, 60))
    {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);

        // Outer bubble
        QRadialGradient bubble(center.x() - radius * 0.3,
                               center.y() - radius * 0.3,
                               radius * 1.2,
                               center.x(), center.y(), 0);
        bubble.setColorAt(0.0, QColor(255, 255, 255, 90));
        bubble.setColorAt(0.4, tint);
        bubble.setColorAt(1.0, QColor(0, 0, 0, 40));

        painter.setPen(QPen(QColor(255, 255, 255, 50), 0.8));
        painter.setBrush(bubble);
        painter.drawEllipse(center, radius, radius);

        // Inner specular catch-light
        QRadialGradient catchLight(center.x() - radius * 0.35, center.y() - radius * 0.35,
                                   radius * 0.45);
        catchLight.setColorAt(0.0, QColor(255, 255, 255, 180));
        catchLight.setColorAt(1.0, Qt::transparent);
        painter.setPen(Qt::NoPen);
        painter.setBrush(catchLight);
        painter.drawEllipse(QPointF(center.x() - radius * 0.35, center.y() - radius * 0.35),
                            radius * 0.45, radius * 0.45);

        painter.restore();
    }

}; // class LiquidGlassPainter


// ─────────────────────────────────────────────────────────────────────────────
// LIQUID GLASS DWM (Windows Desktop Window Manager integration)
// Activates real OS-level blur behind the window on Windows 10/11.
// On Windows 11 (build 22000+), uses DWM Acrylic/Mica for superior quality.
// On older Windows, falls back to "blur behind" (Vista+ DWM API).
// ─────────────────────────────────────────────────────────────────────────────
class LiquidGlassDWM {
public:
    /**
     * enableBlurBehind — enables OS-level blur behind the given window.
     *
     * Call this AFTER the window is shown (or in showEvent) so the HWND exists.
     * The caller must also set:
     *   setAttribute(Qt::WA_TranslucentBackground, true)
     *   setWindowFlags(windowFlags() | Qt::FramelessWindowHint)  [optional]
     *
     * On Windows 11 we use the undocumented DwmSetWindowAttribute with
     * DWMWA_SYSTEMBACKDROP_TYPE = DWMSBT_TRANSIENTWINDOW (Acrylic) or
     * DWMSBT_MAINWINDOW (Mica). Falls back gracefully on older Windows.
     *
     * @param widget  The top-level QWidget (or QMainWindow) to blur.
     * @param mode    0=Auto, 1=Mica, 2=Acrylic, 3=Tabbed Mica
     */
    static void enableBlurBehind(QWidget* widget, int mode = 2)
    {
#ifdef Q_OS_WIN
        if (!widget) return;

        // Ensure the window handle exists.
        HWND hwnd = reinterpret_cast<HWND>(widget->winId());
        if (!hwnd) return;

        // ── Windows 11 (22000+): Use DWM system backdrop ──────────────────
        // DWMWA_SYSTEMBACKDROP_TYPE = 38 (undocumented in older SDKs).
        // Values: 0=Auto, 1=None, 2=Mica, 3=Acrylic, 4=MicaTabbed
        constexpr DWORD DWMWA_SYSTEMBACKDROP_TYPE = 38;
        DWORD backdropType = static_cast<DWORD>(mode < 1 ? 3 : mode);  // default Acrylic
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE,
                              &backdropType, sizeof(backdropType));

        // Extend the frame into the client area (all sides = 0 means full glass).
        MARGINS margins = { -1, -1, -1, -1 };  // -1 = extend entire client
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        // ── Windows 10: Undocumented SetWindowCompositionAttribute ────────
        // This enables the "Acrylic" blur effect on Windows 10 (1703+).
        // It's the same technique used by Windows Terminal, Fluent apps, etc.
        enableWin10Acrylic(hwnd);

        // ── Legacy (Vista/7/8): DWM Blur Behind ───────────────────────────
        DWM_BLURBEHIND bb = {};
        bb.dwFlags    = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.fEnable    = TRUE;
        bb.hRgnBlur   = CreateRectRgn(0, 0, -1, -1);  // whole client
        DwmEnableBlurBehindWindow(hwnd, &bb);
        if (bb.hRgnBlur) DeleteObject(bb.hRgnBlur);

        // Make window layered for alpha compositing support
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_NOACTIVATE & ~WS_EX_NOACTIVATE);
        // Note: WS_EX_NOACTIVATE removed intentionally — we still want activation.
        SetLayeredWindowAttributes(hwnd, RGB(0,0,0), 0, LWA_COLORKEY);
#else
        Q_UNUSED(widget)
        Q_UNUSED(mode)
        // On macOS/Linux: NSVisualEffectView / KWin blur is handled differently.
        // TODO: macOS: use NSVisualEffectView via QMacCocoaViewContainer
        // TODO: Linux/KWin: use _KDE_NET_WM_BLUR_BEHIND_REGION X property
#endif
    }

    /**
     * setDarkMode — requests DWM dark title bar on Windows 10/11.
     * Matches our dark glass theme with the system caption buttons.
     */
    static void setDarkMode(QWidget* widget, bool dark = true)
    {
#ifdef Q_OS_WIN
        if (!widget) return;
        HWND hwnd = reinterpret_cast<HWND>(widget->winId());
        if (!hwnd) return;

        // DWMWA_USE_IMMERSIVE_DARK_MODE = 20 (Windows 11 SDK)
        // DWMWA_USE_IMMERSIVE_DARK_MODE = 19 (Windows 10 20H1 workaround)
        constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
        BOOL darkMode = dark ? TRUE : FALSE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
                              &darkMode, sizeof(darkMode));
        // Fallback for pre-20H1 Windows 10
        DwmSetWindowAttribute(hwnd, 19, &darkMode, sizeof(darkMode));
#else
        Q_UNUSED(widget) Q_UNUSED(dark)
#endif
    }

    /**
     * setWindowCornerPreference — use Windows 11 rounded window corners.
     * DWMWCP_ROUND = 2, DWMWCP_ROUNDSMALL = 3, DWMWCP_DONOTROUND = 1
     */
    static void setRoundedCorners(QWidget* widget, int pref = 2)
    {
#ifdef Q_OS_WIN
        if (!widget) return;
        HWND hwnd = reinterpret_cast<HWND>(widget->winId());
        if (!hwnd) return;

        constexpr DWORD DWMWA_WINDOW_CORNER_PREFERENCE = 33;
        DWORD cornerPref = static_cast<DWORD>(pref);
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                              &cornerPref, sizeof(cornerPref));
#else
        Q_UNUSED(widget) Q_UNUSED(pref)
#endif
    }

    /**
     * setBorderColor — tint the Windows 11 window border to match glass rim.
     */
    static void setBorderColor(QWidget* widget, QColor color)
    {
#ifdef Q_OS_WIN
        if (!widget) return;
        HWND hwnd = reinterpret_cast<HWND>(widget->winId());
        if (!hwnd) return;

        // DWMWA_BORDER_COLOR = 34  (Windows 11+)
        constexpr DWORD DWMWA_BORDER_COLOR = 34;
        // DWM uses COLORREF (0x00BBGGRR)
        COLORREF col = RGB(color.blue(), color.green(), color.red());
        DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &col, sizeof(col));
#else
        Q_UNUSED(widget) Q_UNUSED(color)
#endif
    }

private:
#ifdef Q_OS_WIN
    // Windows 10 Acrylic blur via undocumented SetWindowCompositionAttribute.
    // This is what WinUI 2/Windows Terminal use for the blur effect on Win10.
    struct AccentPolicy {
        int accentState;      // 3 = ACCENT_ENABLE_ACRYLICBLURBEHIND
        int accentFlags;
        int gradientColor;    // ABGR format
        int animationId;
    };
    struct WindowCompositionAttribData {
        int   attrib;         // 19 = WCA_ACCENT_POLICY
        void* pvData;
        ULONG cbData;
    };
    typedef BOOL (WINAPI* pfnSetWindowCompositionAttribute)(HWND, WindowCompositionAttribData*);

    static void enableWin10Acrylic(HWND hwnd) {
        // Dynamically load the undocumented function to avoid linker issues.
        static pfnSetWindowCompositionAttribute pSetWCA = nullptr;
        if (!pSetWCA) {
            HMODULE hUser = GetModuleHandleW(L"user32.dll");
            if (hUser) {
                pSetWCA = reinterpret_cast<pfnSetWindowCompositionAttribute>(
                    GetProcAddress(hUser, "SetWindowCompositionAttribute"));
            }
        }
        if (!pSetWCA) return;

        // Gradient color: AABBGGRR — semi-transparent dark navy
        // 0xCC = 204 alpha, 14,16,24 = deep dark navy (BGR reversed)
        AccentPolicy policy = {
            3,          // ACCENT_ENABLE_ACRYLICBLURBEHIND
            0,          // no flags
            0xCC181014, // ~80% opacity dark navy tint in ABGR
            0
        };
        WindowCompositionAttribData data = {
            19,         // WCA_ACCENT_POLICY
            &policy,
            sizeof(policy)
        };
        pSetWCA(hwnd, &data);
    }
#endif
}; // class LiquidGlassDWM


// ─────────────────────────────────────────────────────────────────────────────
// LIQUID GLASS EFFECT (QGraphicsEffect subclass)
// Software-side blur + tint for widgets that don't get OS-level blur.
// Used by: panels, dialogs, menus when the backdrop isn't the desktop.
// ─────────────────────────────────────────────────────────────────────────────
class LiquidGlassEffect : public QGraphicsEffect {
    Q_OBJECT
public:
    explicit LiquidGlassEffect(QObject* parent = nullptr)
        : QGraphicsEffect(parent)
        , m_blurRadius(LiquidGlassPalette::kBlurRadius)
        , m_tintColor(LiquidGlassPalette::kGlassTintDark)
        , m_cornerRadius(LiquidGlassPalette::kCornerRadius)
    {}

    void setBlurRadius(int r) { m_blurRadius = r; update(); }
    void setTintColor(const QColor& c) { m_tintColor = c; update(); }
    void setCornerRadius(qreal r) { m_cornerRadius = r; update(); }

protected:
    /**
     * draw — the core render pipeline:
     *   1. Capture the source (widget) into a pixmap.
     *   2. Grab the backdrop (pixels behind this widget) via sourcePixmap.
     *   3. Apply software Gaussian blur to the backdrop.
     *   4. Composite: blurred backdrop + tint + sheen + rim → final output.
     *
     * Note: true backdrop capture (rendering what's BEHIND the widget) in Qt
     * requires the widget chain above to be rendered first; this approximation
     * blurs the widget's own rendered pixels which produces a plausible glass
     * look for inner panels where the backdrop isn't the raw desktop.
     */
    void draw(QPainter* painter) override {
        // Get source pixmap at device coordinates
        QPoint offset;
        QPixmap src = sourcePixmap(Qt::DeviceCoordinates, &offset, QGraphicsEffect::NoPad);
        if (src.isNull()) {
            drawSource(painter);
            return;
        }

        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        // ── Step 1: Blur the source pixels (software Gaussian) ────────────
        // Convert to image for pixel manipulation
        QImage blurred = src.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
        blurred = applyGaussianBlur(blurred, m_blurRadius);

        // ── Step 2: Draw blurred backdrop ────────────────────────────────
        painter->setTransform(QTransform());
        painter->drawImage(offset, blurred);

        // ── Step 3: Draw glass layers on top ─────────────────────────────
        QRectF rect(offset, src.size());
        LiquidGlassPainter::drawGlassBackground(*painter, rect, m_cornerRadius, true);

        painter->restore();
    }

    QRectF boundingRectFor(const QRectF& src) const override {
        return src.adjusted(-m_blurRadius, -m_blurRadius, m_blurRadius, m_blurRadius);
    }

private:
    /**
     * applyGaussianBlur — software multi-pass box blur approximation.
     * A true Gaussian is O(n²) per pixel; three box blur passes approximate
     * it with O(n) complexity and are fast enough for UI compositing.
     */
    static QImage applyGaussianBlur(const QImage& src, int radius) {
        if (radius < 1) return src;
        QImage tmp = src;
        // Three box blur passes ≈ Gaussian (central limit theorem)
        tmp = boxBlur(tmp, radius);
        tmp = boxBlur(tmp, radius);
        tmp = boxBlur(tmp, radius);
        return tmp;
    }

    static QImage boxBlur(const QImage& src, int radius) {
        // Horizontal pass
        QImage h = blurH(src, radius);
        // Vertical pass
        return blurV(h, radius);
    }

    static QImage blurH(const QImage& src, int r) {
        QImage dst(src.size(), src.format());
        const int w = src.width(), h = src.height();
        for (int y = 0; y < h; ++y) {
            const QRgb* srcRow = reinterpret_cast<const QRgb*>(src.constScanLine(y));
            QRgb*       dstRow = reinterpret_cast<QRgb*>(dst.scanLine(y));
            int rSum = 0, gSum = 0, bSum = 0, aSum = 0, count = 0;
            for (int x = -r; x <= r && x < w; ++x) {
                int sx = qBound(0, x, w-1);
                QRgb px = srcRow[sx];
                rSum += qRed(px); gSum += qGreen(px); bSum += qBlue(px); aSum += qAlpha(px);
                ++count;
            }
            for (int x = 0; x < w; ++x) {
                dstRow[x] = qRgba(rSum/count, gSum/count, bSum/count, aSum/count);
                // Add next pixel
                int addX = qBound(0, x+r+1, w-1);
                QRgb add = srcRow[addX];
                rSum += qRed(add); gSum += qGreen(add); bSum += qBlue(add); aSum += qAlpha(add);
                ++count;
                // Remove leftmost pixel
                int remX = qBound(0, x-r, w-1);
                QRgb rem = srcRow[remX];
                rSum -= qRed(rem); gSum -= qGreen(rem); bSum -= qBlue(rem); aSum -= qAlpha(rem);
                --count;
            }
        }
        return dst;
    }

    static QImage blurV(const QImage& src, int r) {
        QImage dst(src.size(), src.format());
        const int w = src.width(), h = src.height();
        for (int x = 0; x < w; ++x) {
            int rSum = 0, gSum = 0, bSum = 0, aSum = 0, count = 0;
            for (int y = -r; y <= r && y < h; ++y) {
                int sy = qBound(0, y, h-1);
                QRgb px = reinterpret_cast<const QRgb*>(src.constScanLine(sy))[x];
                rSum += qRed(px); gSum += qGreen(px); bSum += qBlue(px); aSum += qAlpha(px);
                ++count;
            }
            for (int y = 0; y < h; ++y) {
                reinterpret_cast<QRgb*>(dst.scanLine(y))[x] =
                    qRgba(rSum/count, gSum/count, bSum/count, aSum/count);
                int addY = qBound(0, y+r+1, h-1);
                QRgb add = reinterpret_cast<const QRgb*>(src.constScanLine(addY))[x];
                rSum += qRed(add); gSum += qGreen(add); bSum += qBlue(add); aSum += qAlpha(add);
                ++count;
                int remY = qBound(0, y-r, h-1);
                QRgb rem = reinterpret_cast<const QRgb*>(src.constScanLine(remY))[x];
                rSum -= qRed(rem); gSum -= qGreen(rem); bSum -= qBlue(rem); aSum -= qAlpha(rem);
                --count;
            }
        }
        return dst;
    }

    int    m_blurRadius;
    QColor m_tintColor;
    qreal  m_cornerRadius;
}; // class LiquidGlassEffect


// ─────────────────────────────────────────────────────────────────────────────
// LIQUID GLASS STYLE SHEETS
// Pre-built QSS strings that implement the glass look for standard Qt widgets.
// These are the "paint" layer on top of the OS-level DWM blur.
// ─────────────────────────────────────────────────────────────────────────────
namespace LiquidGlassStyleSheet {

    // ─── Main Window QSS ──────────────────────────────────────────────────
    // The background is "transparent" so OS-level DWM blur shows through.
    // All child panels have their own glass tint.
    inline const char* kMainWindow = R"QSS(
        QMainWindow {
            background: transparent;
        }
        QMainWindow::separator {
            background: rgba(255,255,255,15);
            width: 1px;
            height: 1px;
        }
    )QSS";

    // ─── Menu Bar QSS ─────────────────────────────────────────────────────
    inline const char* kMenuBar = R"QSS(
        QMenuBar {
            background: rgba(14, 16, 26, 180);
            color: rgba(235, 240, 255, 230);
            padding: 3px 6px;
            border-bottom: 1px solid rgba(255,255,255,25);
            font-family: "Segoe UI", "SF Pro Display", "Helvetica Neue", sans-serif;
            font-size: 13px;
            letter-spacing: 0.3px;
        }
        QMenuBar::item {
            background: transparent;
            padding: 5px 12px;
            border-radius: 6px;
        }
        QMenuBar::item:selected {
            background: rgba(90,130,255,80);
            color: rgba(255,255,255,255);
        }
        QMenuBar::item:pressed {
            background: rgba(90,130,255,130);
        }
    )QSS";

    // ─── Menu / Popup QSS ─────────────────────────────────────────────────
    inline const char* kMenu = R"QSS(
        QMenu {
            background: rgba(16, 18, 30, 215);
            color: rgba(235, 240, 255, 230);
            border: 1px solid rgba(255,255,255,30);
            border-radius: 10px;
            padding: 5px 0px;
            font-family: "Segoe UI", "SF Pro Display", "Helvetica Neue", sans-serif;
            font-size: 13px;
        }
        QMenu::item {
            padding: 7px 22px 7px 22px;
            border-radius: 6px;
            margin: 1px 5px;
        }
        QMenu::item:selected {
            background: rgba(90, 130, 255, 100);
            color: rgba(255,255,255,255);
        }
        QMenu::item:disabled {
            color: rgba(130, 140, 160, 140);
        }
        QMenu::separator {
            height: 1px;
            background: rgba(255,255,255,20);
            margin: 4px 10px;
        }
        QMenu::indicator {
            width: 14px;
            height: 14px;
        }
    )QSS";

    // ─── Tab Bar / Tab Widget QSS ─────────────────────────────────────────
    inline const char* kTabWidget = R"QSS(
        QTabWidget::pane {
            background: rgba(12, 14, 22, 190);
            border: 1px solid rgba(255,255,255,18);
            border-radius: 0px;
            top: 0px;
        }
        QTabBar {
            background: rgba(10, 12, 20, 170);
        }
        QTabBar::tab {
            background: rgba(20, 22, 35, 160);
            color: rgba(180, 190, 215, 200);
            padding: 8px 18px;
            border: 1px solid rgba(255,255,255,12);
            border-bottom: none;
            margin-right: 2px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            font-family: "Segoe UI", "SF Pro Display", "Helvetica Neue", sans-serif;
            font-size: 12px;
            min-width: 80px;
        }
        QTabBar::tab:selected {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 rgba(255,255,255,30),
                stop:0.5 rgba(90,130,255,25),
                stop:1 rgba(12,14,22,200));
            color: rgba(240, 245, 255, 255);
            border-color: rgba(255,255,255,35);
            border-bottom: 2px solid rgba(90,130,255,200);
        }
        QTabBar::tab:hover:!selected {
            background: rgba(40, 44, 65, 150);
            color: rgba(210, 220, 240, 220);
        }
        QTabBar::close-button {
            image: none;
            subcontrol-position: right;
        }
    )QSS";

    // ─── Status Bar QSS ───────────────────────────────────────────────────
    inline const char* kStatusBar = R"QSS(
        QStatusBar {
            background: rgba(10, 12, 20, 200);
            color: rgba(160, 175, 210, 200);
            border-top: 1px solid rgba(255,255,255,18);
            font-family: "Segoe UI", "Consolas", monospace;
            font-size: 11px;
            padding: 2px 8px;
        }
        QStatusBar::item {
            border: none;
        }
    )QSS";

    // ─── Tool Bar QSS ─────────────────────────────────────────────────────
    inline const char* kToolBar = R"QSS(
        QToolBar {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 rgba(255,255,255,18),
                stop:1 rgba(10,12,20,200));
            border-bottom: 1px solid rgba(255,255,255,20);
            spacing: 3px;
            padding: 3px 5px;
        }
        QToolBar::separator {
            background: rgba(255,255,255,20);
            width: 1px;
            margin: 4px 5px;
        }
        QToolButton {
            background: transparent;
            color: rgba(210, 220, 240, 220);
            border: none;
            border-radius: 6px;
            padding: 5px 9px;
            font-size: 12px;
        }
        QToolButton:hover {
            background: rgba(90,130,255,70);
            color: rgba(255,255,255,255);
        }
        QToolButton:pressed {
            background: rgba(90,130,255,120);
        }
    )QSS";

    // ─── Editor (QPlainTextEdit) QSS ──────────────────────────────────────
    inline const char* kEditor = R"QSS(
        QPlainTextEdit {
            background: rgba(8, 10, 18, 220);
            color: rgba(220, 230, 250, 240);
            border: none;
            selection-background-color: rgba(90, 130, 255, 120);
            selection-color: rgba(255,255,255,255);
            font-family: "Cascadia Code", "Consolas", "JetBrains Mono", "Courier New", monospace;
            font-size: 14px;
            line-height: 1.6;
            padding: 8px;
        }
        QScrollBar:vertical {
            background: rgba(0,0,0,0);
            width: 10px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(255,255,255,40);
            border-radius: 5px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(90,130,255,120);
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            background: rgba(0,0,0,0);
            height: 10px;
        }
        QScrollBar::handle:horizontal {
            background: rgba(255,255,255,40);
            border-radius: 5px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background: rgba(90,130,255,120);
        }
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )QSS";

    // ─── Dialog QSS ───────────────────────────────────────────────────────
    inline const char* kDialog = R"QSS(
        QDialog {
            background: rgba(14, 16, 26, 230);
            color: rgba(235, 240, 255, 230);
            border: 1px solid rgba(255,255,255,30);
            border-radius: 14px;
        }
        QLineEdit {
            background: rgba(255,255,255,12);
            color: rgba(235, 240, 255, 230);
            border: 1px solid rgba(255,255,255,25);
            border-radius: 7px;
            padding: 6px 10px;
            font-family: "Segoe UI", "SF Pro Display", sans-serif;
            font-size: 13px;
        }
        QLineEdit:focus {
            border: 1px solid rgba(90,130,255,200);
            background: rgba(255,255,255,18);
        }
        QPushButton {
            background: rgba(90,130,255,100);
            color: rgba(255,255,255,240);
            border: 1px solid rgba(255,255,255,30);
            border-radius: 8px;
            padding: 7px 18px;
            font-size: 13px;
        }
        QPushButton:hover {
            background: rgba(90,130,255,160);
        }
        QPushButton:pressed {
            background: rgba(60,100,225,200);
        }
        QPushButton:default {
            border: 1.5px solid rgba(90,130,255,200);
        }
        QLabel {
            color: rgba(180, 190, 215, 210);
            background: transparent;
        }
        QCheckBox {
            color: rgba(200, 210, 235, 220);
            spacing: 7px;
        }
        QComboBox {
            background: rgba(255,255,255,12);
            color: rgba(235,240,255,230);
            border: 1px solid rgba(255,255,255,25);
            border-radius: 7px;
            padding: 5px 10px;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
    )QSS";

    // ─── Dock Widget QSS ──────────────────────────────────────────────────
    inline const char* kDockWidget = R"QSS(
        QDockWidget {
            color: rgba(220, 230, 250, 240);
            titlebar-close-icon: url();
            titlebar-normal-icon: url();
            font-family: "Segoe UI", "SF Pro Display", sans-serif;
            font-weight: bold;
            font-size: 13px;
        }
        QDockWidget::title {
            background: rgba(14, 16, 26, 180);
            padding: 6px 10px;
            border-top: 1px solid rgba(255,255,255,25);
            border-bottom: 1px solid rgba(255,255,255,10);
        }
        QTreeView, QListWidget {
            background: rgba(10, 12, 20, 150);
            color: rgba(220, 230, 250, 240);
            border: none;
            outline: none;
            padding: 5px;
            font-family: "Segoe UI", sans-serif;
            font-size: 13px;
        }
        QTreeView::item:selected, QListWidget::item:selected {
            background: rgba(90, 130, 255, 120);
            color: white;
            border-radius: 4px;
        }
        QTreeView::item:hover, QListWidget::item:hover {
            background: rgba(255, 255, 255, 15);
            border-radius: 4px;
        }
    )QSS";

    // ─── Full Combined Application QSS ────────────────────────────────────
    // Apply this to QApplication::setStyleSheet() for global glass theming.
    inline QString kFullGlassTheme() {
        return QString(kMainWindow)
             + QString(kMenuBar)
             + QString(kMenu)
             + QString(kTabWidget)
             + QString(kStatusBar)
             + QString(kToolBar)
             + QString(kEditor)
             + QString(kDialog)
             + QString(kDockWidget);
    }

} // namespace LiquidGlassStyleSheet


#endif // LIQUID_GLASS_H
