cmake_minimum_required(VERSION 3.16)
project(CWTScalogramAnalyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Core Widgets)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(PkgConfig)
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


target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)


if(FFTW3_FOUND)
    target_link_libraries(${PROJECT_NAME} ${FFTW3_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${FFTW3_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_FFTW3)
endif()


set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)


file(COPY ${CMAKE_SOURCE_DIR}/examples
     DESTINATION ${CMAKE_BINARY_DIR})


message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME}")
message(STATUS "  Qt6 Version: ${Qt6_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
if(FFTW3_FOUND)
    message(STATUS "  FFTW3: Found")
else()
    message(STATUS "  FFTW3: Not found (using built-in implementation)")
endif()
message(STATUS "")