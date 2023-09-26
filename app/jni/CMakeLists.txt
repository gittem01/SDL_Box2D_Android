cmake_minimum_required(VERSION 3.6)

project(GAME)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${CMAKE_SOURCE_DIR}/SDL)
add_subdirectory(${CMAKE_SOURCE_DIR}/box2d/src)

# armeabi-v7a requires cpufeatures library
#include(AndroidNdkModules)
#android_ndk_import_module_cpufeatures()

add_subdirectory(src)