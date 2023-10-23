# InfiniSweeper
InfiniSweeper is a spin on the classic game Minesweeper. This game adds infinite recursion to the formula, providing a slightly surreal experience.
![Menu Screenshot](https://raw.githubusercontent.com/Ataraxia-Mechanica/InfiniSweeper/master/screenshots/Title.png)
## Features
- Infinite recursion of rectangular minesweeper boards, where each board can be positioned inside another arbitrarily.
- Cloned board mechanic. It's like Patrick's parabox's clone mechanic, but it works on camera instead: zoom into a clone board and zoom out, camera would be zoomed out from the original.
- Though a level editor is not included, it's easy to design levels due to them being written in TOML, and can be hot reloaded. An example with comments is in `level_example.toml`.
## Compiling
InfiniSweeper uses CMake, and supports MSVC, GCC and Clang.
Follow the following steps to compile:
1. Setup CMake with your favorite compiler.
2. Run the following command:
```bash
cmake build
cd build
cmake --build .
```
3. *Alternative:* Use VSCode's CMake plugin to choose compiler and build/compile.
4. *When building for Windows release:* Run `pack_game_windows.bat`, and `InfiniSweeper-win64` folder would be generated containing binary and assets.
5. *When Building for arm Macos* to build a universal (read: x86 and arm binary) game file, run `cmake -G "Xcode" {root_directory} -B build` to generate cmake files in the /build folder, then run `cmake --build . --config Release` to generate release binary (without the config it will build debug), you can then use the `pack_game_macos.sh` to generate a folder with binary and assets (may need to adjust the first copy to align with the folder structure in build folder
