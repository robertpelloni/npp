cmake_minimum_required(VERSION 3.16)

# ──────────────────────────────────────────────────────────────────────────────
# NotepadPlusPlus_BobUI — Liquid Glass Edition
#
# Build targets:
#   npp_bobui_v2        : Original Modern Glass prototype (QPlainTextEdit-based)
#   npp_liquid_glass    : Full Liquid Glass Edition (DWM blur + animated bubbles)
#
# The liquid glass target is the primary development target.
# Version is read from VERSION file at the repo root (single source of truth).
# ──────────────────────────────────────────────────────────────────────────────
project(NotepadPlusPlus_BobUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:__cplusplus /MP /permissive-)
endif()

# 1. Integrate BobUI (Qt6 Equivalent)
set(BOBUI_DIR ${CMAKE_SOURCE_DIR}/bobui)
set(BOBUI_LIB_DIR ${BOBUI_DIR}/lib)
set(BOBUI_PLUGINS_DIR ${BOBUI_DIR}/plugins)
set(BOBUI_INCLUDE_DIR ${BOBUI_DIR}/include)

include_directories(${BOBUI_INCLUDE_DIR})
include_directories(${BOBUI_INCLUDE_DIR}/QtCore)
include_directories(${BOBUI_INCLUDE_DIR}/QtGui)
include_directories(${BOBUI_INCLUDE_DIR}/QtWidgets)

# Manually find the built libs
find_library(BOBUI_CORE NAMES Qt6Core HINTS ${BOBUI_LIB_DIR} REQUIRED)
find_library(BOBUI_GUI NAMES Qt6Gui HINTS ${BOBUI_LIB_DIR} REQUIRED)
find_library(BOBUI_WIDGETS NAMES Qt6Widgets HINTS ${BOBUI_LIB_DIR} REQUIRED)
find_library(BOBUI_ZLIB NAMES Qt6BundledZLIB HINTS ${BOBUI_LIB_DIR} REQUIRED)
find_library(BOBUI_PCRE2 NAMES Qt6BundledPcre2 HINTS ${BOBUI_LIB_DIR} REQUIRED)
find_library(BOBUI_PNG NAMES Qt6BundledLibpng HINTS ${BOBUI_LIB_DIR} REQUIRED)
find_library(BOBUI_HARF NAMES Qt6BundledHarfbuzz HINTS ${BOBUI_LIB_DIR} REQUIRED)
find_library(BOBUI_FREE NAMES Qt6BundledFreetype HINTS ${BOBUI_LIB_DIR} REQUIRED)

# Find Platform Plugin
find_library(BOBUI_QWINDOWS NAMES qwindows HINTS ${BOBUI_PLUGINS_DIR}/platforms REQUIRED)

# 2. Integrate Scintilla & Lexilla
add_subdirectory(scintilla/win32)
add_subdirectory(lexilla/src)

# 3. PowerEditor Source Integration
include_directories(PowerEditor/src)
include_directories(PowerEditor/src/WinControls)
include_directories(PowerEditor/src/ScintillaComponent)
include_directories(scintilla/include)
include_directories(lexilla/include)

# 4. TextFX Algorithm Integration (Refactored)
include_directories(textfx/src)

# Link against BobUI Widgets and Core
# Note: Static linking requires many Windows system dependencies
set(WIN_LIBS 
    user32 shell32 gdi32 advapi32 ole32 uxtheme dwmapi imm32 winmm opengl32 
    version ws2_32 setupapi crypt32 d3d11 d3d12 dxgi dwrite authz netapi32
    synchronization # For WaitOnAddress
    ntdll # For NtQueryKey
    runtimeobject # For WINRT_IMPL
    userenv # For GetUserProfileDirectoryW
    dxguid # For WKPDID_D3DDebugObjectName
    wtsapi32 # For WTSQuerySessionInformationW
    d3d9 # For Direct3DCreate9
    shcore # For GetDpiForMonitor
    shlwapi # For AssocQueryStringW
    uiautomationcore # For UiaClientsAreListening
)

set(QT_STATIC_LIBS 
    ${BOBUI_WIDGETS} ${BOBUI_GUI} ${BOBUI_CORE} 
    ${BOBUI_ZLIB} ${BOBUI_PCRE2} ${BOBUI_PNG} ${BOBUI_HARF} ${BOBUI_FREE}
    ${BOBUI_QWINDOWS}
)

# ─────────────────────────────────────────────────────────────────────────────
# TARGET 1: Original Modern Glass prototype
# ─────────────────────────────────────────────────────────────────────────────
add_executable(npp_bobui_v2
    PowerEditor/src/NppBobUI_Main.cpp
)

# ─────────────────────────────────────────────────────────────────────────────
# TARGET 2: Liquid Glass Edition (primary target)
#
# Key source files:
#   NppLiquidGlass_Main.cpp  — Main window, editor, bubbles, glass dialogs
#   LiquidGlass.h            — Glass material system (DWM, painter, QSS, effect)
#   TextFXEngine.h           — All text transformation algorithms
#
# Windows-specific system libs required for DWM glass blur:
#   dwmapi  — DwmEnableBlurBehindWindow, DwmSetWindowAttribute
#   uxtheme — Theme APIs
# ─────────────────────────────────────────────────────────────────────────────
add_executable(npp_liquid_glass_v2
    PowerEditor/src/NppLiquidGlass_Main.cpp
)

# Input dialog (needed for Go to Line)
find_library(BOBUI_CORE_EXISTS NAMES Qt6Core HINTS ${BOBUI_LIB_DIR})

target_include_directories(npp_liquid_glass_v2 PRIVATE
    PowerEditor/src
    PowerEditor/src/WinControls
    PowerEditor/src/ScintillaComponent
    ${BOBUI_INCLUDE_DIR}
    ${BOBUI_INCLUDE_DIR}/QtCore
    ${BOBUI_INCLUDE_DIR}/QtGui
    ${BOBUI_INCLUDE_DIR}/QtWidgets
    scintilla/include
    lexilla/include
    textfx/src
)

target_link_libraries(npp_liquid_glass_v2
    ${QT_STATIC_LIBS}
    ${WIN_LIBS}
    dwmapi
    scintilla
    lexilla
)

target_compile_definitions(npp_liquid_glass_v2 PRIVATE
    BOBUI_THEME_LIQUID_GLASS
    NPP_BOBUI_OVERHAUL
    NPP_LIQUID_GLASS
    QT_STATIC
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _UNICODE
    UNICODE
)

target_link_libraries(npp_bobui_v2
    ${QT_STATIC_LIBS}
    ${WIN_LIBS}
    dwmapi
    scintilla
    lexilla
)

# ─────────────────────────────────────────────────────────────────────────────
# npp_bobui_v2 compile definitions
# ─────────────────────────────────────────────────────────────────────────────
target_compile_definitions(npp_bobui_v2 PRIVATE 
    BOBUI_THEME_CYBERPUNK
    NPP_BOBUI_OVERHAUL
    QT_STATIC
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# ─────────────────────────────────────────────────────────────────────────────
# Version propagation — both targets read from VERSION file
# ─────────────────────────────────────────────────────────────────────────────
file(READ "${CMAKE_SOURCE_DIR}/VERSION" NPP_BOBUI_VERSION)
string(STRIP "${NPP_BOBUI_VERSION}" NPP_BOBUI_VERSION)
message(STATUS "Building Notepad++ BobUI Liquid Glass v${NPP_BOBUI_VERSION}")

target_compile_definitions(npp_liquid_glass_v2 PRIVATE
    NPP_VERSION="${NPP_BOBUI_VERSION}"
)
target_compile_definitions(npp_bobui_v2 PRIVATE
    NPP_VERSION="${NPP_BOBUI_VERSION}"
)
