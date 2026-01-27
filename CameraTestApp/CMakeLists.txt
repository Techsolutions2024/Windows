cmake_minimum_required(VERSION 3.16)

project(CameraTestApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
)

# Source files - Backend
set(BACKEND_SOURCES
    src/backend/EDSDKHelper.cpp
    src/backend/CameraDevice.cpp
    src/backend/CameraManager.cpp
    src/backend/PropertyHelper.cpp
)

set(BACKEND_HEADERS
    src/backend/EDSDKHelper.h
    src/backend/CameraDevice.h
    src/backend/CameraManager.h
    src/backend/PropertyHelper.h
)

# QML Resources
set(QML_RESOURCES
    qml/qml.qrc
)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${BACKEND_SOURCES}
    ${BACKEND_HEADERS}
    ${QML_RESOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/../EDSDK/Header
)

# Canon EDSDK library path (64-bit)
set(EDSDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../EDSDK_64)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    ${EDSDK_DIR}/Library/EDSDK.lib
)

# Windows specific settings
if(WIN32)
    # Set subsystem to Windows (no console)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    # Copy EDSDK DLLs to output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${EDSDK_DIR}/Dll/EDSDK.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${EDSDK_DIR}/Dll/EdsImage.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    # Deploy Qt runtime (for Release builds)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")
        if(WINDEPLOYQT_EXECUTABLE)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${WINDEPLOYQT_EXECUTABLE}
                    --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                    "$<TARGET_FILE:${PROJECT_NAME}>"
            )
        endif()
    endif()
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# Print configuration info
message(STATUS "Qt6 Version: ${Qt6_VERSION}")
message(STATUS "EDSDK Directory: ${EDSDK_DIR}")
message(STATUS "Output Directory: ${CMAKE_BINARY_DIR}/bin")
