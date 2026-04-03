cmake_minimum_required(VERSION 3.16)
project(Lexilla)

include_directories(../include)
include_directories(../src)
include_directories(../lexlib)
include_directories(.)

# NPP-specific Lexilla includes
include_directories(../../scintilla/include)
include_directories(../../scintilla/src)

file(GLOB LEXILLA_CORE_SOURCES "*.cxx")
file(GLOB LEXILLA_LEXERS "../lexers/*.cxx")
file(GLOB LEXILLA_LEXLIB "../lexlib/*.cxx")

add_library(lexilla STATIC
    ${LEXILLA_CORE_SOURCES}
    ${LEXILLA_LEXERS}
    ${LEXILLA_LEXLIB}
)
target_compile_definitions(lexilla PRIVATE UNICODE _UNICODE SCI_LEXER)
