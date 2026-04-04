# Liquid Glass Phase 15-16 Analysis & Findings

## 1. UI Architecture Pivot (v2 vs v3)
- **v2 (Qt6/bobui)**: Currently the stable, feature-complete implementation. 
    - *Finding*: High-DPI scaling and DWM (Mica/Acrylic) are natively handled via `LiquidGlassDWM`.
    - *Improvement*: Vertical tab orientation was corrected from "sideways" to "stacked horizontal" by overriding `QTabBar` CSS selectors (`QTabBar::tab:left`).
- **v3 (GTK4/bobgui)**: Experimental branch added as a submodule.
    - *Status*: Entry point created in `NppLiquidGlass_Main_v3.cpp`. 
    - *Blocking Factor*: The build system currently points to Qt6 headers. Meson/MSVC integration for GTK4 is required for the next phase.

## 2. Input & Focus Diagnostics
- **Problem**: Users intermittently reported "cannot type in editor".
- **Analysis**: Scintilla is a native Win32 window (`HWND`) hosted inside a Qt `QWidget`. 
    - **Discovery 1**: `QTabWidget` switching does not automatically pass Win32 focus to the child `HWND`.
    - **Discovery 2**: Mouse clicks on the "glass" area of the widget were being consumed by Qt before reaching the Scintilla message loop.
- **Solution implemented**:
    - Explicit `::SetFocus(m_sciHwnd)` on `focusInEvent`, `mousePressEvent`, and `showEvent`.
    - `grabFocus()` helper added to ensure reliable redirection.

## 3. Visual Parity: Vertical Tabs
- **Legacy Issue**: Standard Win32/Qt vertical tabs rotate text 90 degrees.
- **Liquid Glass Implementation**:
    - Modified `LiquidGlass.h` style sheets to force `min-height` and `min-width` on vertical tabs.
    - Result: A modern "sidebar" look similar to VS Code or visionOS, where labels remain horizontal but tabs stack vertically.

## 4. Stability & Build System
- **MSB6001 Error**: Resolved via `build.py` environment cleanup (preventing `HOME`/`Home` collisions).
- **Process Locking**: Build target remains `npp_liquid_glass_v2.exe` to allow iterative building while the previous instance is open (provided it isn't the specific binary being replaced).

## 5. Next Strategic Goals
- [ ] **UDL Integration**: Fully map `userDefineLang.xml` to `GlassUDLManager`.
- [ ] **Native Event Filter**: Implement `QAbstractNativeEventFilter` to catch `WM_CHAR` directly and bypass Qt's keyboard interception for Scintilla.
- [ ] **Printing**: Audit missing `Qt6PrintSupport.lib` in the static `bobui` build.
