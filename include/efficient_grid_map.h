#ifndef GRID_MAP_H
#define GRID_MAP_H

#include <Eigen/Dense>
#include <string>
#include <unordered_map>
#include <memory>

template<typename T>
class GridLayer {
public:
    GridLayer(uint32_t width, uint32_t height, const T& initial_value = T())
        : width_(width), height_(height), 
          logical_offset_row_(0), logical_offset_col_(0),
          prev_logical_offset_row_(0), prev_logical_offset_col_(0),
          initial_value_(initial_value) {
        data_ = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Constant(height, width, initial_value);
    }

    // 逻辑坐标转物理坐标
    inline std::pair<uint32_t, uint32_t> logicalToPhysical(int32_t logical_row, int32_t logical_col) const {
        int32_t physical_row = (logical_row + logical_offset_row_) % height_;
        if (physical_row < 0) physical_row += height_;
        
        int32_t physical_col = (logical_col + logical_offset_col_) % width_;
        if (physical_col < 0) physical_col += width_;
        
        return {physical_row, physical_col};
    }

    // 物理坐标转逻辑坐标
    inline std::pair<int32_t, int32_t> physicalToLogical(uint32_t physical_row, uint32_t physical_col) const {
        int32_t logical_row = (static_cast<int32_t>(physical_row) - logical_offset_row_) % height_;
        if (logical_row < 0) logical_row += height_;
        
        int32_t logical_col = (static_cast<int32_t>(physical_col) - logical_offset_col_) % width_;
        if (logical_col < 0) logical_col += width_;
        
        return {logical_row, logical_col};
    }

    // 零拷贝移动逻辑原点，并初始化新区域
    void moveLogicalOrigin(int32_t d_row, int32_t d_col) {
        // 保存之前的逻辑原点
        prev_logical_offset_row_ = logical_offset_row_;
        prev_logical_offset_col_ = logical_offset_col_;
        
        // 更新逻辑原点
        logical_offset_row_ = (logical_offset_row_ + d_row) % height_;
        logical_offset_col_ = (logical_offset_col_ + d_col) % width_;
        
        if (logical_offset_row_ < 0) logical_offset_row_ += height_;
        if (logical_offset_col_ < 0) logical_offset_col_ += width_;
        
        // 初始化新扫过的区域
        initializeNewRegion();
    }

    // 按逻辑坐标访问
    T getValue(int32_t logical_row, int32_t logical_col) const {
        auto [physical_row, physical_col] = logicalToPhysical(logical_row, logical_col);
        return data_(physical_row, physical_col);
    }

    // 按逻辑坐标设置
    void setValue(int32_t logical_row, int32_t logical_col, const T& value) {
        auto [physical_row, physical_col] = logicalToPhysical(logical_row, logical_col);
        data_(physical_row, physical_col) = value;
    }

    // 获取视图（逻辑坐标）
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> getView(
        int32_t center_row, int32_t center_col, uint32_t view_height, uint32_t view_width) const {
        
        Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> view = 
            Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(view_height, view_width);
        
        for (uint32_t y = 0; y < view_height; ++y) {
            for (uint32_t x = 0; x < view_width; ++x) {
                int32_t logical_row = center_row - static_cast<int32_t>(view_height/2) + y;
                int32_t logical_col = center_col - static_cast<int32_t>(view_width/2) + x;
                
                view(y, x) = getValue(logical_row, logical_col);
            }
        }
        
        return view;
    }

    // 重置所有值
    void reset(const T& value = T()) {
        data_.setConstant(value);
        initial_value_ = value;
    }

private:
    // 初始化新扫过的区域
    void initializeNewRegion() {
        // 计算新旧区域的差异
        int32_t row_diff = logical_offset_row_ - prev_logical_offset_row_;
        int32_t col_diff = logical_offset_col_ - prev_logical_offset_col_;
        
        // 处理垂直方向的新区域
        if (row_diff != 0) {
            if (row_diff > 0) {
                // 向上移动，初始化底部的新行
                for (int32_t r = prev_logical_offset_row_; 
                     r < prev_logical_offset_row_ + row_diff; ++r) {
                    int32_t physical_row = r % height_;
                    if (physical_row < 0) physical_row += height_;
                    data_.row(physical_row).setConstant(initial_value_);
                }
            } else {
                // 向下移动，初始化顶部的新行
                int32_t abs_row_diff = -row_diff;
                for (int32_t r = logical_offset_row_; 
                     r < logical_offset_row_ + abs_row_diff; ++r) {
                    int32_t physical_row = r % height_;
                    if (physical_row < 0) physical_row += height_;
                    data_.row(physical_row).setConstant(initial_value_);
                }
            }
        }
        
        // 处理水平方向的新区域
        if (col_diff != 0) {
            if (col_diff > 0) {
                // 向左移动，初始化右侧的新列
                for (int32_t c = prev_logical_offset_col_; 
                     c < prev_logical_offset_col_ + col_diff; ++c) {
                    int32_t physical_col = c % width_;
                    if (physical_col < 0) physical_col += width_;
                    data_.col(physical_col).setConstant(initial_value_);
                }
            } else {
                // 向右移动，初始化左侧的新列
                int32_t abs_col_diff = -col_diff;
                for (int32_t c = logical_offset_col_; 
                     c < logical_offset_col_ + abs_col_diff; ++c) {
                    int32_t physical_col = c % width_;
                    if (physical_col < 0) physical_col += width_;
                    data_.col(physical_col).setConstant(initial_value_);
                }
            }
        }
    }

    uint32_t width_;
    uint32_t height_;
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> data_;
    int32_t logical_offset_row_;  // 逻辑原点在物理矩阵中的行偏移
    int32_t logical_offset_col_;  // 逻辑原点在物理矩阵中的列偏移
    int32_t prev_logical_offset_row_;  // 之前的行偏移，用于初始化新区域
    int32_t prev_logical_offset_col_;  // 之前的列偏移，用于初始化新区域
    T initial_value_;  // 新区域的初始值
};

template<typename T>
class CircularGridMap {
public:
    CircularGridMap(uint32_t width, uint32_t height, const T& initial_value = T())
        : width_(width), height_(height), 
          world_center_x_(0.0), world_center_y_(0.0),
          initial_value_(initial_value) {
        addLayer("default");
    }

    // 添加新图层
    void addLayer(const std::string& layer_name) {
        if (layers_.find(layer_name) == layers_.end()) {
            layers_[layer_name] = std::make_unique<GridLayer<T>>(width_, height_, initial_value_);
        }
    }

    // 删除图层
    void removeLayer(const std::string& layer_name) {
        layers_.erase(layer_name);
    }

    // 移动地图中心（零拷贝）
    void moveCenter(int32_t dx, int32_t dy) {
        // 更新世界坐标系中的地图中心
        world_center_x_ += dx;
        world_center_y_ += dy;
        
        // 计算逻辑坐标的变化量
        int32_t d_row = dx;  // 行方向与x轴方向相反
        int32_t d_col = dy;   // 列方向与x轴方向相同
        
        // 更新所有图层的逻辑原点
        for (auto& [name, layer] : layers_) {
            layer->moveLogicalOrigin(d_row, d_col);
        }
    }

    // 获取单元格值（支持指定图层）
    T getCellValue(double world_x, double world_y, const std::string& layer_name = "default") const {
        // 计算相对于地图中心的坐标偏移
        double rel_x = world_x - world_center_x_;
        double rel_y = world_y - world_center_y_;

        // 转换为逻辑坐标
        int32_t logical_row = static_cast<int32_t>(rel_y);
        int32_t logical_col = static_cast<int32_t>(rel_x);

        // 获取图层并查询值
        auto it = layers_.find(layer_name);
        if (it != layers_.end()) {
            return it->second->getValue(logical_row, logical_col);
        }
        
        return T();
    }

    // 设置单元格值（支持指定图层）
    void setCellValue(double world_x, double world_y, const T& value, 
                     const std::string& layer_name = "default") {
        // 计算相对于地图中心的坐标偏移
        double rel_x = world_x - world_center_x_;
        double rel_y = world_y - world_center_y_;

        // 转换为逻辑坐标
        int32_t logical_row = static_cast<int32_t>(rel_y);
        int32_t logical_col = static_cast<int32_t>(rel_x);

        // 获取图层并设置值
        auto it = layers_.find(layer_name);
        if (it != layers_.end()) {
            it->second->setValue(logical_row, logical_col, value);
        }
    }

    // 获取视图（支持指定图层）
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> getView(
        uint32_t view_width, uint32_t view_height, 
        const std::string& layer_name = "default") const {
        
        // 地图中心对应的逻辑坐标为 (0, 0)
        int32_t center_row = 0;
        int32_t center_col = 0;

        // 获取图层并生成视图
        auto it = layers_.find(layer_name);
        if (it != layers_.end()) {
            return it->second->getView(center_row, center_col, view_height, view_width);
        }
        
        return Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Zero(view_height, view_width);
    }

    // 重置图层数据
    void resetLayer(const std::string& layer_name = "default", const T& value = T()) {
        auto it = layers_.find(layer_name);
        if (it != layers_.end()) {
            it->second->reset(value);
        }
    }

    uint32_t getWidth() const { return width_; }
    uint32_t getHeight() const { return height_; }
    double getWorldCenterX() const { return world_center_x_; }
    double getWorldCenterY() const { return world_center_y_; }

private:
    uint32_t width_;
    uint32_t height_;
    double world_center_x_;
    double world_center_y_;
    std::unordered_map<std::string, std::unique_ptr<GridLayer<T>>> layers_;
    T initial_value_;  // 新图层的默认初始值
};

#endif // GRID_MAP_H    