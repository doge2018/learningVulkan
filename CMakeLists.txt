cmake_minimum_required(VERSION 3.20)

project(learnVulkan LANGUAGES C CXX)
add_executable(${PROJECT_NAME})

include(source.cmake)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_include_directories(${PROJECT_NAME} PRIVATE include externals/glfw/include externals/loguru/include)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan ${CMAKE_SOURCE_DIR}/externals/glfw/libglfw3dll.a)