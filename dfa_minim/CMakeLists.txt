cmake_minimum_required(VERSION 3.0)
project(dfa_minim) 

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_STANDARD 17)

link_directories(dfa_minim ${PROJECT_SOURCE_DIR}/)
add_executable(dfa_minim main.cpp task.cpp)

set_target_properties(dfa_minim PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
       target_link_libraries(dfa_minim RegexCheckerCore_Darwin)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
       target_link_libraries(dfa_minim RegexCheckerCore_Linux)
elseif(CMAKE_HOST_WIN32)
       target_link_libraries(dfa_minim RegexCheckerCore_Windows)
endif()

install(TARGETS dfa_minim DESTINATION .)
