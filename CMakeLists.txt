cmake_minimum_required(VERSION 3.10)
project(mdsv2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
message(STATUS "Found Qt5: ${Qt5_VERSION}")


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(FFTW3 fftw3)
endif()


set(SOURCES
    main.cpp
    WaveletAnalyzer.cpp
    PlotWidgets.cpp
)

set(HEADERS
    WaveletAnalyzer.h
)


add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})


target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets)


if(FFTW3_FOUND)
    target_link_libraries(${PROJECT_NAME} ${FFTW3_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${FFTW3_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_FFTW3)
    message(STATUS "FFTW3 found and will be used for optimized FFT")
else()
    message(STATUS "FFTW3 not found - using built-in math functions")
endif()


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()


set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)


message(STATUS "")
message(STATUS "=== Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Qt5 Version: ${Qt5_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "FFTW3: ${FFTW3_FOUND}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "==============================")
message(STATUS "")