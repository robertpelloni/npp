cmake_minimum_required(VERSION 3.16)
project(Lexilla)

include_directories(../include)
include_directories(../src)
include_directories(../lexlib)
include_directories(.)

# NPP-specific Lexilla includes
include_directories(../../scintilla/include)
include_directories(../../scintilla/src)

add_library(lexilla STATIC
    Lexilla.cxx
)
target_compile_definitions(lexilla PRIVATE UNICODE _UNICODE SCI_LEXER)
