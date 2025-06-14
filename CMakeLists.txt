# Cmake版本
cmake_minimum_required(VERSION 3.0.6)

# project name
project(EfficientGridMap)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加Eigen库
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# 添加头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加library
add_library(EfficientGridMap SHARED ${PROJECT_SOURCE_DIR}/src/efficient_grid_map.cpp)

target_link_libraries(EfficientGridMap Eigen3::Eigen)

# 添加可执行文件
add_executable(EfficientGridMapTest ${PROJECT_SOURCE_DIR}/test/efficient_grid_map_test.cpp)
target_link_libraries(EfficientGridMapTest EfficientGridMap)

# 添加安装规则,将库文件和可执行文件安装到指定目录
install(TARGETS EfficientGridMap EfficientGridMapTest
        LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/lib
        ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib
        RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin)


