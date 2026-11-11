cmake_minimum_required(VERSION 3.13)
project(poker_chip LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

set(CONFIG_LV_BUILD_DEMOS OFF CACHE INTERNAL "Build LVGL demos")
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE INTERNAL "Build LVGL examples")
set(CONFIG_LV_USE_THORVG OFF CACHE INTERNAL "Enable ThorVG")
add_subdirectory(lvgl)
target_link_libraries(lvgl PRIVATE SDL2::SDL2)

add_executable(main src/main.c src/score_ui.c)
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)
target_link_libraries(main PRIVATE lvgl SDL2::SDL2)

add_custom_target(run COMMAND $<TARGET_FILE:main> DEPENDS main VERBATIM)
