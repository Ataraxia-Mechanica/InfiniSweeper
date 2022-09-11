# InfiniSweeper
InfiniSweeper is a spin on the classic game Minesweeper. This game adds infinite recursion to the formula, providing a slightly surreal experience.
![Menu Screenshot](https://raw.githubusercontent.com/Ataraxia-Mechanica/InfiniSweeper/master/screenshots/Title.png)
## Features
- Infinite recursion of rectangular minesweeper boards, where each board can be positioned inside another arbitrarily.
- Cloned board mechanic. It's like Patrick's parabox's clone mechanic, but it works on camera instead: zoom into a clone board and zoom out, camera would be zoomed out from the original.
- Though a level editor is not included, it's easy to design levels due to them being written in toml, and can be hot reloaded. An example with comments is in level_example.toml.
## Compiling
InfiniSweeper uses CMake, and supports MSVC, GCC and Clang.
Follow the following steps to compile:
1. Setup CMake with your favorite compilier
2. Run the following command:
```
cmake build
cd build
cmake --build .
```
3. (Alternative:) Use VSCode's CMake plugin to choose compiler and build/compile.
4. (When building for windows release:) Run pack_game_windows.bat, and InfiniSweeper-win64 folder would be generated containing binary and assets
