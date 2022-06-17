cd /home/build/re
mkdir -p build
cd build
/mingw64/bin/cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Debug --target re_tests
./Debug/re_tests.exe
