rm -rf build
cmake -S . -B build/
cmake --build build/ --target main --config Release