# rm -rf build
# cmake -S . -B build/ -DPYTHON_EXECUTABLE="/Users/carol/miniconda3/bin/python3"
cmake -S . -B build/
cmake --build build/ --target main --config Release
