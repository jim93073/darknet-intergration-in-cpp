cmake_minimum_required (VERSION 3.9)

file(READ "VERSION.txt" VERSION)

project (DarknetIntergration VERSION ${VERSION} DESCRIPTION "A C++ YOLO Device Implementation based on Darknet.")

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package( OpenCV 3.4.14 REQUIRED )

add_subdirectory (libs/darknet) 

include(${CMAKE_CURRENT_SOURCE_DIR}/CUDA.cmake)

file(GLOB SOURCES "include/*.h" "src/*.cpp" "src/*.c")

# # for main executable
add_executable (main ${SOURCES} ${cuda_sources})
target_link_libraries (main darknet_static ${OpenCV_LIBS})
target_include_directories(main PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(main PRIVATE "${PROJECT_SOURCE_DIR}/libs/darknet/src")
target_include_directories(main PRIVATE "${PROJECT_SOURCE_DIR}/libs/darknet/include")
target_include_directories(main PRIVATE "${PROJECT_SOURCE_DIR}/libs/darknet/3rdparty/stb/include")


# for libyolotalk.so
add_library(yolotalk SHARED ${SOURCES} ${cuda_sources})
target_link_libraries(yolotalk darknet_static ${OpenCV_LIBS})
set_target_properties(yolotalk PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(yolotalk PROPERTIES SOVERSION 1)
set_target_properties(yolotalk PROPERTIES PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/include/clibyolotalk.h)
target_include_directories(yolotalk PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(yolotalk PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(yolotalk PRIVATE "${PROJECT_SOURCE_DIR}/libs/darknet/src")
target_include_directories(yolotalk PRIVATE "${PROJECT_SOURCE_DIR}/libs/darknet/include")
target_include_directories(yolotalk PRIVATE "${PROJECT_SOURCE_DIR}/libs/darknet/3rdparty/stb/include")
