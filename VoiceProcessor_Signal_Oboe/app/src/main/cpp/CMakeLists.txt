cmake_minimum_required(VERSION 3.22.1)

project("voiceprocessor_signal_oboe")

add_subdirectory(${CMAKE_SOURCE_DIR}/oboe)

add_library(voiceprocessor_signal_oboe SHARED
        SignalGenerator.h
        AudioEngine.cpp
        AudioEngine.h
        native-lib.cpp
)

target_include_directories(voiceprocessor_signal_oboe PRIVATE
        $CMAKE_SOURCE_DIR/oboe/include
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library

        oboe
        android
        log
)