cmake_minimum_required(VERSION 3.16)
project(Scintilla)

include_directories(../include)
include_directories(../src)
include_directories(.)

file(GLOB SCINTILLA_CORE_SOURCES "../src/*.cxx")
file(GLOB SCINTILLA_WIN32_SOURCES "*.cxx")

add_library(scintilla STATIC ${SCINTILLA_CORE_SOURCES} ${SCINTILLA_WIN32_SOURCES})
target_compile_definitions(scintilla PRIVATE UNICODE _UNICODE SCI_LEXER)
