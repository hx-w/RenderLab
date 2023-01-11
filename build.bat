@REM rmdir /s/q .\build
cmake -S . -B .\build
cmake --build .\build --target main -j 10 --config Release
move .\Release\main.exe .\