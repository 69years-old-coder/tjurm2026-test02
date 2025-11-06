#include "impls.h"
#include <iostream>

std::pair<cv::Rect, cv::RotatedRect> get_rect_by_contours(const cv::Mat& input) {
    std::pair<cv::Rect, cv::RotatedRect> res;
    
    // ========== 第一步：图像预处理 ==========
    cv::Mat gray; 
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    cv::Mat binary;
    
    // 尝试不同的二值化方法
    // 方法1：使用固定阈值（根据你的图像调整这个值）
    cv::threshold(gray, binary, 100, 255, cv::THRESH_BINARY);
    
    // 或者方法2：使用自适应阈值
    // cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);
    
    // 或者方法3：先反转图像再二值化（如果矩形是黑色背景白色边框）
    // cv::bitwise_not(gray, gray);
    // cv::threshold(gray, binary, 100, 255, cv::THRESH_BINARY);
    
    // ========== 调试：保存二值化图像 ==========
    cv::imwrite("debug_binary.jpg", binary);
    std::cout << "二值化图像已保存为 debug_binary.jpg" << std::endl;
    
    // ========== 第二步：轮廓检测 ==========
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    
    // 尝试使用 RETR_LIST 而不是 RETR_EXTERNAL
    cv::findContours(binary, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    
    std::cout << "找到轮廓数量: " << contours.size() << std::endl;
    
    // ========== 第三步：筛选矩形轮廓 ==========
    if (contours.empty()) {
        std::cout << "没有找到任何轮廓" << std::endl;
        return res;
    }
    
    std::vector<std::vector<cv::Point>> rectangle_contours;
    
    for (size_t i = 0; i < contours.size(); i++) {
        const auto& contour = contours[i];
        double area = cv::contourArea(contour);
        std::cout << "轮廓 " << i << " 面积: " << area << std::endl;
        
        // 忽略太小的轮廓
        if (area < 100) {
            std::cout << "轮廓 " << i << " 太小，跳过" << std::endl;
            continue;
        }
        
        // 排除边框轮廓
        double image_area = input.rows * input.cols;
        if (area > image_area * 0.95) {
            std::cout << "轮廓 " << i << " 太大，可能是边框，跳过" << std::endl;
            continue;
        }
        
        cv::Rect bbox = cv::boundingRect(contour);
        int margin = 5;
        if (bbox.x <= margin || bbox.y <= margin ||
            bbox.x + bbox.width >= input.cols - margin ||
            bbox.y + bbox.height >= input.rows - margin) {
            std::cout << "轮廓 " << i << " 太接近边界，跳过" << std::endl;
            continue;
        }
        
        // 对轮廓进行多边形近似
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 0.02 * cv::arcLength(contour, true), true);
        
        std::cout << "轮廓 " << i << " 顶点数: " << approx.size() << std::endl;
        
        // 检查是否是四边形
        if (approx.size() == 4) {
            std::cout << "轮廓 " << i << " 是四边形，添加到候选列表" << std::endl;
            rectangle_contours.push_back(contour);
        }
    }
    
    std::cout << "筛选后矩形轮廓数量: " << rectangle_contours.size() << std::endl;
    
    if (rectangle_contours.empty()) {
        std::cout << "没有找到合适的矩形轮廓" << std::endl;
        return res;
    }
    
    // ========== 第四步：选择最佳矩形 ==========
    auto largest_rectangle = std::max_element(rectangle_contours.begin(), rectangle_contours.end(), 
        [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
            return cv::contourArea(a) < cv::contourArea(b);
        });
    
    // ========== 第五步：计算两种外接矩形 ==========
    cv::Rect bounding_rect = cv::boundingRect(*largest_rectangle);
    cv::RotatedRect min_area_rect = cv::minAreaRect(*largest_rectangle);
    
    std::cout << "选择的矩形位置: x=" << bounding_rect.x << ", y=" << bounding_rect.y 
              << ", w=" << bounding_rect.width << ", h=" << bounding_rect.height << std::endl;
    
    res.first = bounding_rect;
    res.second = min_area_rect;
    
    return res;
}
