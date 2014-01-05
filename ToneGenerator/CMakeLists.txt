cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSING_GL11")

#find_package(BBCascades REQUIRED)

include_directories(
    /Applications/Momentics.app/target_10_2_0_1155/qnx6/usr/include/freetype2
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(ToneGenerator_SRCS
    src/main.c
    src/bbutil.c
    src/audiopcm.c
)

add_executable(Device-Debug/ToneGenerator ${ToneGenerator_SRCS} ${WikiReader_RCC_SRCS})

# -lbps -lm -laudio_manager -lasound -lEGL -lGLESv2 -lGLESv1_CL -lscreen -lfreetype -lpng
target_link_libraries(
    Device-Debug/ToneGenerator

    EGL
    GLESv2
    GLESv1_CL
    m
    png
    audio_manager
    asound
    screen
    bps
    freetype
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}

    ${BBCASCADES_LIBRARY}
)
