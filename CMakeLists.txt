cmake_minimum_required(VERSION 3.16)
project(pomodoro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(pomodoro
    src/main.cpp
    src/PomodoroEngine.cpp
    src/PomodoroWidget.cpp
    src/SettingsDialog.cpp
    src/AudioPlayer.cpp
    src/SoundGenerator.cpp
    src/Config.cpp
)

target_include_directories(pomodoro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(pomodoro PRIVATE Qt6::Widgets)

# miniaudio backend dependencies (per-platform).
if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(pomodoro PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
elseif(APPLE)
    find_library(COREAUDIO CoreAudio)
    find_library(COREFOUNDATION CoreFoundation)
    find_library(AUDIOTOOLBOX AudioToolbox)
    find_library(AUDIOUNIT AudioUnit)
    target_link_libraries(pomodoro PRIVATE
        ${COREAUDIO} ${COREFOUNDATION} ${AUDIOTOOLBOX} ${AUDIOUNIT})
endif()
# On Windows miniaudio pulls in the WASAPI/ole libraries automatically.

install(TARGETS pomodoro RUNTIME DESTINATION bin)
