cmake_minimum_required(VERSION 3.16)
project(Scintilla)

include_directories(../include)
include_directories(../src)
include_directories(.)

set(SCINTILLA_SOURCES
    PlatWin.cxx
    ScintillaWin.cxx
    HanjaDic.cxx
    ListBox.cxx
    SurfaceGDI.cxx
    # Add more source files as needed
)

add_library(scintilla STATIC ${SCINTILLA_SOURCES})
target_compile_definitions(scintilla PRIVATE UNICODE _UNICODE SCI_LEXER)
