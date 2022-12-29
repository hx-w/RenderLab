@REM rmdir /s/q .\build
cmake -DPYTHON_EXECUTABLE="C:\Users\Administrator\AppData\Local\Microsoft\WindowsApps\python3.exe" -S . -B .\build
cmake --build .\build --target main -j 10 --config Release
move .\Release\main.exe .\