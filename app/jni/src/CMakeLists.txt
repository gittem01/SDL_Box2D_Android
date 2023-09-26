include_directories(${CMAKE_SOURCE_DIR}/SDL/include)
include_directories(${CMAKE_SOURCE_DIR}/box2d/include)
include_directories(${CMAKE_SOURCE_DIR}/src/include)
include_directories(${CMAKE_SOURCE_DIR}/vulkan/include)
include_directories(${CMAKE_SOURCE_DIR}/glm/include)
include_directories(${CMAKE_SOURCE_DIR}/tiny_gltf)

add_library(main SHARED)

file(GLOB sourceFiles
        ${CMAKE_SOURCE_DIR}/src/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/src/vk/*.cpp
        ${CMAKE_SOURCE_DIR}/src/src/ShaderSources/*.cpp)

file(GLOB headerFiles
        ${CMAKE_SOURCE_DIR}/src/include/*.h
        ${CMAKE_SOURCE_DIR}/src/include/vk/*.h
        ${CMAKE_SOURCE_DIR}/src/include/ShaderIncludes/*.h)

target_sources(main PRIVATE main.cpp ${sourceFiles} ${headerFiles})

find_library(VULKAN_LIB NAMES "vulkan" "libvulkan" "vulkan-1")

target_link_libraries(main SDL2 box2d ${VULKAN_LIB})