cmake_minimum_required(VERSION 3.15)
project(PhotoboothPro VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent for downloading dependencies
include(FetchContent)

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

# ============================================================================
# Dependencies - Auto download using FetchContent
# ============================================================================

# SQLite - Download amalgamation
set(SQLITE_DIR "${CMAKE_BINARY_DIR}/_deps/sqlite")
if(NOT EXISTS "${SQLITE_DIR}/sqlite3.c")
    message(STATUS "Downloading SQLite amalgamation...")
    set(SQLITE_VERSION "3450000")
    set(SQLITE_URL "https://www.sqlite.org/2024/sqlite-amalgamation-${SQLITE_VERSION}.zip")
    set(SQLITE_ZIP "${CMAKE_BINARY_DIR}/sqlite.zip")

    file(DOWNLOAD ${SQLITE_URL} ${SQLITE_ZIP}
        STATUS SQLITE_DOWNLOAD_STATUS
        SHOW_PROGRESS
    )
    list(GET SQLITE_DOWNLOAD_STATUS 0 SQLITE_DOWNLOAD_ERROR)
    if(NOT SQLITE_DOWNLOAD_ERROR EQUAL 0)
        message(FATAL_ERROR "Failed to download SQLite: ${SQLITE_DOWNLOAD_STATUS}")
    endif()

    file(ARCHIVE_EXTRACT INPUT ${SQLITE_ZIP} DESTINATION ${CMAKE_BINARY_DIR}/_deps)
    file(RENAME "${CMAKE_BINARY_DIR}/_deps/sqlite-amalgamation-${SQLITE_VERSION}" ${SQLITE_DIR})
    file(REMOVE ${SQLITE_ZIP})
    message(STATUS "SQLite downloaded successfully")
endif()

# cpp-httplib (header-only HTTP library)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(httplib)

# nlohmann/json (header-only JSON library)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

# spdlog (fast logging library)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)

# ASIO (standalone, required by websocketpp) - header-only, don't run its CMake
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-29-0
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()

# websocketpp - header-only, don't run its CMake (has outdated CMakeLists.txt)
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG 0.8.2
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(websocketpp)
if(NOT websocketpp_POPULATED)
    FetchContent_Populate(websocketpp)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EDSDK_INCLUDE_DIR}
    ${SQLITE_DIR}
    ${httplib_SOURCE_DIR}
    ${json_SOURCE_DIR}/include
    ${spdlog_SOURCE_DIR}/include
    ${websocketpp_SOURCE_DIR}
    ${asio_SOURCE_DIR}/asio/include
)

# ============================================================================
# OpenCV - Auto download pre-built MSVC binaries
# ============================================================================
set(OPENCV_VERSION "4.9.0")
set(OPENCV_DIR "${CMAKE_BINARY_DIR}/_deps/opencv")
set(OPENCV_EXTRACT_DIR "${OPENCV_DIR}/opencv")

# Check if OpenCV is already downloaded
if(NOT EXISTS "${OPENCV_EXTRACT_DIR}/build/x64/vc16/lib/opencv_world490.lib")
    message(STATUS "Downloading OpenCV ${OPENCV_VERSION} pre-built binaries...")

    set(OPENCV_URL "https://github.com/opencv/opencv/releases/download/${OPENCV_VERSION}/opencv-${OPENCV_VERSION}-windows.exe")
    set(OPENCV_EXE "${CMAKE_BINARY_DIR}/opencv-${OPENCV_VERSION}-windows.exe")

    # Download OpenCV self-extracting archive
    if(NOT EXISTS "${OPENCV_EXE}")
        file(DOWNLOAD ${OPENCV_URL} ${OPENCV_EXE}
            STATUS OPENCV_DOWNLOAD_STATUS
            SHOW_PROGRESS
        )
        list(GET OPENCV_DOWNLOAD_STATUS 0 OPENCV_DOWNLOAD_ERROR)
        if(NOT OPENCV_DOWNLOAD_ERROR EQUAL 0)
            message(WARNING "Failed to download OpenCV: ${OPENCV_DOWNLOAD_STATUS}")
            message(STATUS "Webcam will use simulation mode")
            set(OPENCV_AVAILABLE FALSE)
        else()
            set(OPENCV_AVAILABLE TRUE)
        endif()
    else()
        set(OPENCV_AVAILABLE TRUE)
    endif()

    # Extract OpenCV (it's a self-extracting 7z archive)
    if(OPENCV_AVAILABLE AND NOT EXISTS "${OPENCV_EXTRACT_DIR}")
        message(STATUS "Extracting OpenCV...")
        file(MAKE_DIRECTORY ${OPENCV_DIR})

        # Use 7z or PowerShell to extract
        find_program(7ZIP_EXECUTABLE 7z PATHS "C:/Program Files/7-Zip" "C:/Program Files (x86)/7-Zip")
        if(7ZIP_EXECUTABLE)
            execute_process(
                COMMAND ${7ZIP_EXECUTABLE} x -y -o${OPENCV_DIR} ${OPENCV_EXE}
                RESULT_VARIABLE EXTRACT_RESULT
                OUTPUT_QUIET
            )
        else()
            # Fallback: run the exe with -o flag (self-extracting)
            execute_process(
                COMMAND ${OPENCV_EXE} -o${OPENCV_DIR} -y
                RESULT_VARIABLE EXTRACT_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
                TIMEOUT 300
            )
        endif()

        if(NOT EXTRACT_RESULT EQUAL 0)
            message(WARNING "Failed to extract OpenCV, trying alternative method...")
            # Try PowerShell extraction
            execute_process(
                COMMAND powershell -Command "Start-Process -FilePath '${OPENCV_EXE}' -ArgumentList '-o\"${OPENCV_DIR}\"','-y' -Wait -NoNewWindow"
                RESULT_VARIABLE EXTRACT_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
                TIMEOUT 300
            )
        endif()

        if(EXISTS "${OPENCV_EXTRACT_DIR}/build/x64/vc16/lib/opencv_world490.lib")
            message(STATUS "OpenCV extracted successfully")
        else()
            message(WARNING "OpenCV extraction may have failed - checking alternative paths")
        endif()
    endif()
endif()

# Set OpenCV paths if available
if(EXISTS "${OPENCV_EXTRACT_DIR}/build/x64/vc16/lib/opencv_world490.lib")
    set(OpenCV_DIR "${OPENCV_EXTRACT_DIR}/build")
    set(OpenCV_FOUND TRUE)
    set(OpenCV_VERSION ${OPENCV_VERSION})
    set(OpenCV_INCLUDE_DIRS "${OPENCV_EXTRACT_DIR}/build/include")
    set(OpenCV_LIB_DIR "${OPENCV_EXTRACT_DIR}/build/x64/vc16/lib")
    set(OpenCV_BIN_DIR "${OPENCV_EXTRACT_DIR}/build/x64/vc16/bin")
    set(OpenCV_LIBS "${OpenCV_LIB_DIR}/opencv_world490.lib")

    include_directories(${OpenCV_INCLUDE_DIRS})
    message(STATUS "OpenCV ${OPENCV_VERSION} configured successfully")
    message(STATUS "  Include: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "  Library: ${OpenCV_LIBS}")
else()
    # Try to find system-installed OpenCV (MSVC version only)
    find_package(OpenCV QUIET)
    if(OpenCV_FOUND)
        # Check if this is from MSYS2/MinGW (not compatible with MSVC)
        if(OpenCV_DIR MATCHES "msys64|mingw")
            message(STATUS "OpenCV found but is MinGW/MSYS2 version - not compatible with MSVC")
            set(OpenCV_FOUND FALSE)
        else()
            message(STATUS "Using system OpenCV: ${OpenCV_VERSION}")
            include_directories(${OpenCV_INCLUDE_DIRS})
        endif()
    endif()

    if(NOT OpenCV_FOUND)
        message(STATUS "OpenCV not available - webcam will use simulation mode")
        message(STATUS "  (Auto-download may have failed, check internet connection)")
    endif()
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
    src/camera/CanonCamera.cpp
    src/camera/WebcamCamera.cpp
    src/api/HTTPServer.cpp
    src/api/WebSocketServer.cpp
    src/storage/DatabaseManager.cpp
    src/storage/FileManager.cpp
    src/image/LayoutAnalyzer.cpp
)

# Create executable
add_executable(photobooth-server ${SOURCES} ${SQLITE_SOURCES})

# Compile definitions
target_compile_definitions(photobooth-server PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_ENABLE_FTS5
    SQLITE_ENABLE_JSON1
    ASIO_STANDALONE
    _WEBSOCKETPP_CPP11_THREAD_
    $<$<BOOL:${OpenCV_FOUND}>:USE_OPENCV>
)

# Link libraries
target_link_libraries(photobooth-server
    ${EDSDK_LIB_DIR}/EDSDK.lib
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    ws2_32
    wsock32
)

if(OpenCV_FOUND)
    target_link_libraries(photobooth-server ${OpenCV_LIBS})

    # Copy OpenCV DLL to output directory
    if(EXISTS "${OpenCV_BIN_DIR}/opencv_world490.dll")
        add_custom_command(TARGET photobooth-server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OpenCV_BIN_DIR}/opencv_world490.dll"
            $<TARGET_FILE_DIR:photobooth-server>
        )
    endif()

    # Also copy opencv_videoio_ffmpeg DLL if exists (needed for video capture)
    file(GLOB OPENCV_FFMPEG_DLLS "${OpenCV_BIN_DIR}/opencv_videoio_ffmpeg*.dll")
    foreach(FFMPEG_DLL ${OPENCV_FFMPEG_DLLS})
        add_custom_command(TARGET photobooth-server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FFMPEG_DLL}"
            $<TARGET_FILE_DIR:photobooth-server>
        )
    endforeach()
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
