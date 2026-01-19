cmake_minimum_required(VERSION 3.15)
project(PhotoboothPro VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Canon EDSDK paths
set(EDSDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../EDSDK_64")
set(EDSDK_HEADER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../EDSDK")
set(EDSDK_INCLUDE_DIR "${EDSDK_HEADER_ROOT}/Header")
set(EDSDK_LIB_DIR "${EDSDK_ROOT}/Library")
set(EDSDK_DLL_DIR "${EDSDK_ROOT}/Dll")

# SQLite paths (download from https://www.sqlite.org/download.html)
set(SQLITE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/sqlite")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EDSDK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cpp-httplib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/websocketpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/json/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/spdlog/include
    ${SQLITE_DIR}
)

# Find OpenCV
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
    add_definitions(-DUSE_OPENCV)
endif()

# SQLite source (amalgamation)
set(SQLITE_SOURCES
    ${SQLITE_DIR}/sqlite3.c
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/camera/CameraManager.cpp
    src/api/HTTPServer.cpp
    src/api/WebSocketServer.cpp
    src/storage/DatabaseManager.cpp
)

# Create executable
add_executable(photobooth-server ${SOURCES} ${SQLITE_SOURCES})

# Compile definitions
target_compile_definitions(photobooth-server PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_ENABLE_FTS5
    SQLITE_ENABLE_JSON1
)

# Link libraries
target_link_libraries(photobooth-server
    ${EDSDK_LIB_DIR}/EDSDK.lib
    ws2_32
    wsock32
)

if(OpenCV_FOUND)
    target_link_libraries(photobooth-server ${OpenCV_LIBS})
endif()

# Copy EDSDK DLL to output directory
add_custom_command(TARGET photobooth-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${EDSDK_DLL_DIR}/EDSDK.dll"
    $<TARGET_FILE_DIR:photobooth-server>
)

# Copy EdsImage.dll if exists
if(EXISTS "${EDSDK_DLL_DIR}/EdsImage.dll")
    add_custom_command(TARGET photobooth-server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${EDSDK_DLL_DIR}/EdsImage.dll"
        $<TARGET_FILE_DIR:photobooth-server>
    )
endif()

# Create data directory for database
add_custom_command(TARGET photobooth-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:photobooth-server>/data
)

# Installation
install(TARGETS photobooth-server DESTINATION bin)
install(FILES "${EDSDK_LIB_DIR}/EDSDK.dll" DESTINATION bin)
install(DIRECTORY DESTINATION bin/data)
