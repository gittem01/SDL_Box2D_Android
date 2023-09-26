
# Compilation of companion libraries
#add_subdirectory(SDL_image)
#add_subdirectory(SDL_mixer)
#add_subdirectory(SDL_ttf)

# Your game and its CMakeLists.txt are in a subfolder named "src"

#find_library(SDL2 SDL2)

include_directories(${CMAKE_SOURCE_DIR}/SDL/include)
include_directories(${CMAKE_SOURCE_DIR}/box2d/include)
include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_library(main SHARED)

file(GLOB sourceFiles ${CMAKE_SOURCE_DIR}/src/src/*.cpp)
file(GLOB headerFiles ${CMAKE_SOURCE_DIR}/src/include/*.h)

target_sources(main PRIVATE main.cpp ${sourceFiles} ${headerFiles})

target_link_libraries(main SDL2 box2d)