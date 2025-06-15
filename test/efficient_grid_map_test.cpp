
#include "efficient_grid_map.h"
#include <iostream>

int main(int argc, char** argv) {
    // 创建一个10x10的多图层地图
    CircularGridMap<float> grid_map(10, 10);

    // 添加新图层
    grid_map.addLayer("obstacle");
    grid_map.addLayer("navigation");

    // 在不同位置设置值
    grid_map.setCellValue(0.0, 0.0, 1.0);      // 默认图层
    grid_map.setCellValue(2.0, 2.0, 100.0, "obstacle"); // 障碍物图层
    
    // 打印初始状态下的地图值
    std::cout << "默认图层 (0,0) 值: " << grid_map.getCellValue(0.0, 0.0) << std::endl;
    std::cout << "障碍物图层 (2,2) 值: " << grid_map.getCellValue(2.0, 2.0, "obstacle") << std::endl;

    // 向右移动3个单位，向下移动2个单位
    std::cout << "\n执行移动操作..." << std::endl;
    grid_map.moveCenter(3, 2);
    std::cout << "移动后地图中心坐标: (" 
              << grid_map.getWorldCenterX() << ", " 
              << grid_map.getWorldCenterY() << ")" << std::endl;

    // 移动后，原来的点(0,0)现在位于(-3,-2)世界坐标
    std::cout << "移动后，默认图层 (-3,-2) 值: " << grid_map.getCellValue(-3.0, -2.0) << std::endl;
    
    // 原来的障碍物点(2,2)现在位于(-1,-0)世界坐标
    std::cout << "移动后，障碍物图层 (-1,0) 值: " << grid_map.getCellValue(-1.0, 0.0, "obstacle") << std::endl;

    // 获取以新中心为基准的5x5视图
    auto defaultView = grid_map.getView(10, 10);
    auto obstacleView = grid_map.getView(10, 10, "obstacle");

    // 打印视图
    std::cout << "\n默认图层视图:" << std::endl;
    std::cout << defaultView << std::endl;
    
    std::cout << "障碍物图层视图:" << std::endl;
    std::cout << obstacleView << std::endl;

    return 0;
}