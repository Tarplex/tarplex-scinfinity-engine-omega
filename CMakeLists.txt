cmake_minimum_required(VERSION 3.16)
project(TarplexScinfinityEngineOmega
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Advanced, optimized, and customized Browser Engine"
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Find required packages
find_package(Qt6 COMPONENTS Core Gui Widgets WebEngineWidgets REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Enable Qt automoc and autorcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add source files
file(GLOB_RECURSE ENGINE_SOURCES "src/engine/*.cpp")
file(GLOB_RECURSE RENDERER_SOURCES "src/renderer/*.cpp")
file(GLOB_RECURSE JS_SOURCES "src/js-interpreter/*.cpp")
file(GLOB_RECURSE NETWORK_SOURCES "src/network/*.cpp")
file(GLOB_RECURSE SECURITY_SOURCES "src/security/*.cpp")
file(GLOB_RECURSE UI_SOURCES "src/ui/*.cpp")

# Create library targets
add_library(engine_lib STATIC ${ENGINE_SOURCES})
add_library(renderer_lib STATIC ${RENDERER_SOURCES})
add_library(js_lib STATIC ${JS_SOURCES})
add_library(network_lib STATIC ${NETWORK_SOURCES})
add_library(security_lib STATIC ${SECURITY_SOURCES})
add_library(ui_lib STATIC ${UI_SOURCES})

# Link libraries
target_link_libraries(engine_lib PUBLIC ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})
target_link_libraries(renderer_lib PUBLIC engine_lib)
target_link_libraries(js_lib PUBLIC engine_lib)
target_link_libraries(network_lib PUBLIC engine_lib ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})
target_link_libraries(security_lib PUBLIC engine_lib ${OPENSSL_LIBRARIES})
target_link_libraries(ui_lib PUBLIC 
    engine_lib renderer_lib js_lib network_lib security_lib
    Qt6::Core Qt6::Gui Qt6::Widgets
)

# Create main executable
add_executable(tarplex_browser 
    src/main.cpp
)

target_link_libraries(tarplex_browser PRIVATE 
    ui_lib engine_lib renderer_lib js_lib network_lib security_lib
    Qt6::Core Qt6::Gui Qt6::Widgets
    ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES}
    ICU::uc ICU::i18n
)

# Enable testing
enable_testing()
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(run_tests ${TEST_SOURCES})
    target_link_libraries(run_tests PRIVATE 
        engine_lib renderer_lib js_lib network_lib security_lib
        ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES}
    )
    add_test(NAME AllTests COMMAND run_tests)
endif()

# Print configuration summary
message(STATUS "============================================")
message(STATUS "Tarplex Scinfinity Engine Omega")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "OpenSSL Version: ${OPENSSL_VERSION}")
message(STATUS "============================================")
