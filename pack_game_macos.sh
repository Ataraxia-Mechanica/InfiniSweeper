mkdir InfiniSweeper-macos_arm
cd InfiniSweeper-macos_arm
cp ../build/bin/Release/InfiniSweeper.exe .
cp  ../levels.toml .
cp  ../level_example.toml .

mkdir res
cd res
cp ../../res/loading.png .
cp ../../res/logo.png .
cp ../../res/number_0.png .
cp ../../res/number_1.png .
cp ../../res/ui.png .
cp ../../res/tile.png .

cd ..
mkdir licenses
cd licenses
cp ../../license .
cp ../../res/license_font_MINE_SWEEPER .
cp ../../vendor/imgui/LICENSE.txt license_imgui
cp ../../vendor/raylib/LICENSE license_raylib
cp ../../vendor/raylib-cpp/LICENSE license_raylib_cpp
cp ../../vendor/rlImGui/LICENSE license_rlImGui
cp ../../vendor/tomlplusplus/LICENSE "license_toml++"
