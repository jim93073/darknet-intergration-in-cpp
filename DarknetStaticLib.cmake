#--
##################################
## Build Darknet static library ##
##################################

ADD_LIBRARY(darknet_static STATIC ${CMAKE_CURRENT_LIST_DIR}/src/darknet.c ${sources} ${headers} ${cuda_sources})

if(BUILD_AS_CPP)
  set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/darknet.c PROPERTIES LANGUAGE CXX)
  set_target_properties(darknet_static PROPERTIES LINKER_LANGUAGE CXX)
endif()

target_include_directories(darknet_static PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src> $<INSTALL_INTERFACE:${DARKNET_INSTALL_INCLUDE_DIR}> $<BUILD_INTERFACE:${Stb_INCLUDE_DIR}>)

target_compile_definitions(darknet_static PRIVATE -DUSE_CMAKE_LIBS)

if(CUDNN_FOUND)
  target_link_libraries(darknet_static PRIVATE CuDNN::CuDNN)
  target_compile_definitions(darknet_static PRIVATE -DCUDNN)
  if(ENABLE_CUDNN_HALF)
    target_compile_definitions(darknet_static PRIVATE -DCUDNN_HALF)
  endif()
endif()

if(OpenCV_FOUND)
  target_link_libraries(darknet_static PRIVATE ${OpenCV_LINKED_COMPONENTS})
  target_compile_definitions(darknet_static PRIVATE -DOPENCV)
endif()

if(OPENMP_FOUND)
  target_link_libraries(darknet_static PRIVATE OpenMP::OpenMP_CXX)
  target_link_libraries(darknet_static PRIVATE OpenMP::OpenMP_C)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
  target_link_libraries(darknet_static PRIVATE m)
endif()

if(MSVC)
  target_link_libraries(darknet_static PRIVATE PThreads_windows::PThreads_windows)
  target_link_libraries(darknet_static PRIVATE wsock32)
  target_compile_definitions(darknet_static PRIVATE -D_CRT_RAND_S -DNOMINMAX -D_USE_MATH_DEFINES)
endif()

if(MSVC OR MINGW)
  target_link_libraries(darknet_static PRIVATE ws2_32)
endif()

target_link_libraries(darknet_static PRIVATE Threads::Threads)

if(ENABLE_ZED_CAMERA)
  target_link_libraries(darknet_static PRIVATE ${ZED_LIBRARIES})
  target_compile_definitions(darknet_static PRIVATE -DZED_STEREO)
endif()

if(ENABLE_CUDA)
  target_include_directories(darknet_static PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
  target_link_libraries(darknet_static PRIVATE curand cublas cuda)
  target_compile_definitions(darknet_static PRIVATE -DGPU)
endif()

if(USE_INTEGRATED_LIBS)
  target_compile_definitions(darknet_static PRIVATE -D_TIMESPEC_DEFINED)
endif()
