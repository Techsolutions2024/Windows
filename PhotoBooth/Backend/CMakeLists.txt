cmake_minimum_required(VERSION 3.15)
project(PhotoBoothBackend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(EDSDK_LIB_DIR "${CMAKE_SOURCE_DIR}/../EDSDK_64/Library")
        set(EDSDK_DLL_DIR "${CMAKE_SOURCE_DIR}/../EDSDK_64/Dll")
    else()
        set(EDSDK_LIB_DIR "${CMAKE_SOURCE_DIR}/../EDSDK/Library")
        set(EDSDK_DLL_DIR "${CMAKE_SOURCE_DIR}/../EDSDK/Dll")
    endif()
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
    set(EDSDK_FRAMEWORK_DIR "${CMAKE_SOURCE_DIR}/../EDSDK/Framework")
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
    set(EDSDK_LIB_DIR "${CMAKE_SOURCE_DIR}/../EDSDK/Library")
endif()

message(STATUS "Building for ${PLATFORM_NAME}")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../EDSDK/Header
    ${CMAKE_SOURCE_DIR}/EDSDKWrapper
    ${CMAKE_SOURCE_DIR}/ImageProcessor
    ${CMAKE_SOURCE_DIR}/PrintManager
    ${CMAKE_SOURCE_DIR}/Server
)

# EDSDK Wrapper Library
add_library(EDSDKWrapper SHARED
    EDSDKWrapper/Camera.cpp
    EDSDKWrapper/Camera.h
)

# Link EDSDK
if(WIN32)
    target_link_libraries(EDSDKWrapper
        ${EDSDK_LIB_DIR}/EDSDK.lib
    )
    # Copy DLLs to output directory
    add_custom_command(TARGET EDSDKWrapper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${EDSDK_DLL_DIR}/EDSDK.dll"
        $<TARGET_FILE_DIR:EDSDKWrapper>
    )
    add_custom_command(TARGET EDSDKWrapper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${EDSDK_DLL_DIR}/EdsImage.dll"
        $<TARGET_FILE_DIR:EDSDKWrapper>
    )
elseif(APPLE)
    find_library(EDSDK_FRAMEWORK EDSDK PATHS ${EDSDK_FRAMEWORK_DIR})
    target_link_libraries(EDSDKWrapper ${EDSDK_FRAMEWORK})
elseif(UNIX)
    target_link_libraries(EDSDKWrapper
        ${EDSDK_LIB_DIR}/libEDSDK.so
    )
endif()

# Image Processor Library (requires OpenCV)
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    add_library(ImageProcessor SHARED
        ImageProcessor/Overlay.cpp
        ImageProcessor/Overlay.h
        ImageProcessor/GreenScreen.cpp
        ImageProcessor/GreenScreen.h
        ImageProcessor/Filter.cpp
        ImageProcessor/Filter.h
    )
    target_link_libraries(ImageProcessor ${OpenCV_LIBS})
else()
    message(WARNING "OpenCV not found. Image processing features will be limited.")
endif()

# Print Manager Library
if(WIN32)
    add_library(PrintManager SHARED
        PrintManager/Printer.cpp
        PrintManager/Printer.h
    )
    target_link_libraries(PrintManager winspool)
endif()

# IPC Server
add_library(IPCServer SHARED
    Server/IPCServer.cpp
    Server/IPCServer.h
)

# Main executable for testing
add_executable(PhotoBoothBackend
    main.cpp
)

target_link_libraries(PhotoBoothBackend
    EDSDKWrapper
)

if(OpenCV_FOUND)
    target_link_libraries(PhotoBoothBackend ImageProcessor)
endif()

if(WIN32)
    target_link_libraries(PhotoBoothBackend PrintManager IPCServer)
endif()

# Installation
install(TARGETS EDSDKWrapper
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(OpenCV_FOUND)
    install(TARGETS ImageProcessor
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

# Copy EDSDK DLLs to install directory
if(WIN32)
    install(FILES
        ${EDSDK_DLL_DIR}/EDSDK.dll
        ${EDSDK_DLL_DIR}/EdsImage.dll
        DESTINATION bin
    )
endif()
