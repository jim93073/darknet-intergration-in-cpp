#!/bin/bash

echo  "Download Darknet from GitHub"
git submodule init
git submodule update --recursive

echo "Insert Darknet static library CMake file"
cp DarknetStaticLib.cmake libs/darknet
NEW_LINE="include(\${CMAKE_CURRENT_SOURCE_DIR}/DarknetStaticLib.cmake)"
sed -i -r 's/include\(.*DarknetStaticLib.cmake\).*//gm' libs/darknet/CMakeLists.txt
echo ${NEW_LINE} >> libs/darknet/CMakeLists.txt

echo "Start compile"
cd builds
cmake ../
# make -j $(nproc --all)
make -j 1

echo "Cleanup Darknet static library CMake file"
cd ../
sed -i -r 's/include\(.*DarknetStaticLib.cmake\).*//gm' libs/darknet/CMakeLists.txt
rm libs/darknet/DarknetStaticLib.cmake

echo "Done. The compiled file is stored in './builds/main'."
