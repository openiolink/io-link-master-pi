# only for cmake --version >= 3.18.4
cmake_minimum_required(VERSION 3.18.4)

# project name
project(openiolink VERSION 1.0.0 LANGUAGES C CXX)

# set the C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Define Required libraries 
list(APPEND REQUIRED_LIBRARIES 
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
include(CMakePrintHelpers)
# handle 3rdparty dependencies
add_subdirectory(3rdparty)

#  include headers
file(GLOB headers include/*.h external/include/*.hpp)

# puts all .cpp files inside src to the SOURCES variable
file(GLOB sources src/*.cpp)

# Add includes
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/include/)

# compiles the files defined by SOURCES to generate the executable defined by EXEC
add_executable(openiolink ${sources} ${headers})

# Adding static libs for iodd parsing
find_library(IODD_Manager NAMES iodd-manager PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external PATH_SUFFIXES "libs/" "include/")
cmake_print_variables(IODD_Manager)

# add Library to Link
target_link_libraries(openiolink
    PUBLIC
        wiringPi
        pthread
        mosquitto
        nlohmann_json
        ${IODD_Manager}
)