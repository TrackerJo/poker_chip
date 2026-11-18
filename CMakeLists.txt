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

add_executable(main
    src/main.c
    src/home_ui.c
    src/game_select_ui.c
    src/add_player_ui.c
    src/score_ui.c
    src/scoreboard_podium_24.c
    src/card_suits_24.c
    src/card_suits_32.c
    src/card_suits_48.c
    src/game_state.c
    src/welcome_ui.c
    src/mountains.c
    src/point_counter_icon.c
    src/components.c
)
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)
target_link_libraries(main PRIVATE lvgl SDL2::SDL2)

add_custom_target(run COMMAND $<TARGET_FILE:main> DEPENDS main VERBATIM)
