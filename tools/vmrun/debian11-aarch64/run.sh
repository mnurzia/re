cd /home/vmbuild/re
mkdir -p build
cd build
cmake ..
cmake --build . --config Debug --target re_tests
./re_tests
