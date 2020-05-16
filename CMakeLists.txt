cmake_minimum_required(VERSION 2.8)

project(TarsBenchmark)

option(TARS_SSL "option for ssl" OFF)
option(TARS_MYSQL "option for mysql" OFF)
option(TARS_HTTP2 "option for http2" OFF)

if(WIN32)
    include (c:\\tars\\cpp\\makefile\\tars-tools.cmake)
else()
    include (/usr/local/tars/cpp/makefile/tars-tools.cmake)
endif()

add_subdirectory(src)

