cmake_minimum_required(VERSION 3.0)
project(dfa2re) 

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_STANDARD 17)

link_directories(dfa2re ${PROJECT_SOURCE_DIR}/)
add_executable(dfa2re main.cpp task.cpp)

set_target_properties(dfa2re PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
       target_link_libraries(dfa2re RegexCheckerCore_Darwin)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
       target_link_libraries(dfa2re RegexCheckerCore_Linux)
elseif(CMAKE_HOST_WIN32)
       target_link_libraries(dfa2re RegexCheckerCore_Windows)
endif()

install(TARGETS dfa2re DESTINATION .)
