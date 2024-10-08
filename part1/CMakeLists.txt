cmake_minimum_required(VERSION 3.5.0)
project(part1 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)


if(WIN32) # Install dlls in the same directory as the executable on Windows
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# Add cpr - C++ Requests library
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 1.10.5) # The commit hash for 1.10.5. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

# Define source code content
set(SOURCES 
    src/main.cpp
    src/my_hash_table.cpp
)

#add_executable(part1 main.cpp main.h)
add_executable(${PROJECT_NAME} ${SOURCES}) 

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)



# ############################################
# Configuration (download libs...):
# cmake -DCMAKE_BUILD_TYPE=Debug -S. -Bbuild

# Debug Build:
# cmake --build build -j  

# Release Build:
# cmake --build build -j --config Release
