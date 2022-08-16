mkdir InfiniSweeper-win64
cd InfiniSweeper-win64
copy ..\build\Release\bin\InfiniSweeper.exe *
copy ..\levels.toml *
copy ..\level_example.toml *

mkdir res
cd res
copy ..\..\res\loading.png *
copy ..\..\res\logo.png *
copy ..\..\res\number_0.png *
copy ..\..\res\number_1.png *
copy ..\..\res\ui.png *
copy ..\..\res\tile.png *

cd ..
mkdir licenses
cd licenses
copy ..\..\license *
copy ..\..\res\license_font_MINE_SWEEPER *
copy ..\..\vendor\imgui\LICENSE.txt license_imgui
copy ..\..\vendor\raylib\LICENSE license_raylib
copy ..\..\vendor\raylib-cpp\LICENSE license_raylib_cpp
copy ..\..\vendor\rlImGui\LICENSE license_rlImGui
copy ..\..\vendor\tomlplusplus\LICENSE "license_toml++"
