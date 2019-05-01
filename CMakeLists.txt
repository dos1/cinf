cmake_minimum_required(VERSION 2.8)

project(cinf C)

SET(LIBSUPERDERPY_GAMENAME "cinf" CACHE INTERNAL "")
SET(LIBSUPERDERPY_GAMENAME_PRETTY "Computer Is (Not) Fine" CACHE INTERNAL "")

SET(LIBSUPERDERPY_APPID "net.dosowisko.cinf")
SET(LIBSUPERDERPY_ORIENTATION "LANDSCAPE" CACHE INTERNAL "")
SET(LIBSUPERDERPY_VERSION "1.2" CACHE INTERNAL "")

set(EMSCRIPTEN_TOTAL_MEMORY "128" CACHE INTERNAL "")

set(IMGTOWEBP OFF CACHE INTERNAL "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/libsuperderpy/cmake")

include(libsuperderpy)

add_subdirectory(libsuperderpy)
add_subdirectory(src)
add_subdirectory(data)
