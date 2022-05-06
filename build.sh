# rm -rf build
cmake -S . -B build/
cmake --build build/
mv build/src/main ./