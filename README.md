# InfiniSweeper
InfiniSweeper is a spin on the classic game Minesweeper. This game adds infinite recursion to the formula, providing a slightly surreal experience.
![Menu Screenshot](https://raw.githubusercontent.com/Ataraxia-Mechanica/InfiniSweeper/master/screenshots/Title.png)
## Features
- Infinite recursion of rectangular minesweeper boards, where each board can be positioned inside another arbitrarily.
- Cloned board mechanic. It's like Patrick's parabox's clone mechanic, but it works on camera instead: zoom into a clone board and zoom out, camera would be zoomed out from the original.
- Though a level editor is not included, it's easy to design levels due to them being written in toml, and can be hot reloaded. An example with comments in in level_example.toml.
## Compiling
InfiniSweeper uses CMake, and supports MSVC, GCC and Clang.
Follow the following steps to compile:
1. Clone/Download Releases/Create Symbolic Links to the following libraries into the InfiniSweeper/vendor folder:
```
imgui - 1.88
rlImGui - commit 036fcad
raylib - 4.0.0
raylib-cpp - 4.2.3
toml++ - 3.1.0
```
2. Setup CMake with your favorite compilier
3. Run the following command:
```
cmake build
cd build
cmake --build .
```
3. (Alternative:)Use VSCode's CMake plugin to choose compiler and build/compile.
4. (When building for windows release:) Run pack_game_windows.bat, and InfiniSweeper-win64 folder would be generated containing binary and assets
