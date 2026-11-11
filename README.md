# Poker Chip

An LVGL score screen for a 240 × 240 round display. The desktop simulator uses SDL2.

## Run the simulator

Install CMake, a C/C++ compiler, and SDL2. On macOS, run `brew install cmake sdl2`.

```sh
git submodule update --init lvgl
cmake -S . -B build
cmake --build build --target run
```

Edit `src/score_ui.c` to change the layout. The simulator opens with a placeholder score of `0`.

For the ESP32-S3, initialize LVGL and the board's display and touch drivers in your firmware, then call `score_ui_create()` with the active screen. Include `src/score_ui.c` and `src/score_ui.h` in that project and enable the Montserrat 14 and 48 fonts in its LVGL configuration.
