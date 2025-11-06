#include "impls.h"
#include <unordered_map>


std::unordered_map<int, cv::Rect> roi_color(const cv::Mat& input) {
    /**
     * INPUT: 一张彩色图片, 路径: assets/roi_color/input.png
     * OUTPUT: 一个 unordered_map, key 为颜色(Blue: 0, Green: 1, Red: 2), value 为对应颜色的矩形区域(cv::Rect)
     * TODO:
     *  1. 找到图片中三个颜色的轮廓
     *  2. 计算出三处矩形的颜色
     *  3. 返回一个 unordered_map, key 为颜色, value 为cv::Rect (表示矩形的位置)
     * HINT:
     *  1. 预处理
     *    1. 在学习 findContours 时，我们提供的输入图片已经是一个二值图像了，
     *       但是在这里，input 是一个彩色图形。
     *       而 findContours 的 input 最好是一个二值图像。
     *    2. findContours 处理时，一般是在一个黑色背景上，找白色的轮廓
     *       (因此在二值化时，你可能需要考虑一下 cv::THRESH_BINARY_INV 选项
     *       同时，建议了解一下 cv::THRESH_OTSU 选项以及他们如何配合使用)
     *    3. 因此，预处理流程如下:
     *      1. 将 input 转换成灰度图像
     *      2. 对灰度图像进行二值化，得到黑底的二值化图
     *   2. 找到三个矩形 (findContours)
     *   3. 对找到的三个矩形，分别进行如下计算:
     *      1. 使用 cv::boundingRect 将轮廓转换成矩形 cv::Rect
     *      2. 使用该 cv::Rect 得到 input 中的 ROI区域 (写法: roi = input(rect))
     *      3. 使用统计的方法，得到该 ROI 区域的颜色
     *      4. 将颜色 和 矩形位置 存入 map 中
     */
    std::unordered_map<int, cv::Rect> res;
    // IMPLEMENT YOUR CODE HERE


    // 1 . 图像预处理：彩色 -> 灰度 -> 二值
    cv::Mat gray, binary;
    
    // 将彩色图像转换为灰度图像
    // 参数：源图像, 目标图像, 转换类型
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    
    // 对灰度图像进行二值化处理
    // 参数：源图像, 目标图像, 阈值(使用OTSU时设为0), 255-最大值, 
    // cv::THRESH_BINARY_INV | cv::THRESH_OTSU-二值化类型(反转+OTSU自适应阈值)
    // THRESH_BINARY_INV: 将大于阈值的像素设为0，小于等于的设为255（白底黑字变为黑底白字）
    // THRESH_OTSU: 自动计算最佳阈值
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    // 2. 查找轮廓
    std::vector<std::vector<cv::Point>> contours;  // 存储找到的轮廓点集
    std::vector<cv::Vec4i> hierarchy;             // 存储轮廓的层次结构信息
    
    // 查找图像中的所有轮廓
    // 参数：输入二值图像, 输出轮廓, 输出层次结构,
    // cv::RETR_EXTERNAL-只检索最外层轮廓, cv::CHAIN_APPROX_SIMPLE-压缩轮廓点
    cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 3. 处理每个找到的轮廓
    for (const auto& contour : contours) {
        // 使用boundingRect计算轮廓的最小外接矩形
        cv::Rect rect = cv::boundingRect(contour);
        
        
        cv::Mat roi = input(rect);// 从原图中提取ROI区域
        cv::Scalar mean_color = cv::mean(roi);// 计算ROI区域的平均颜色
        
        // 根据BGR通道值判断颜色类型
        int color_type = -1;
        
        // OpenCV使用BGR格式：mean_color[0]=B, mean_color[1]=G, mean_color[2]=R
        if (mean_color[0] > mean_color[1] && mean_color[0] > mean_color[2]) {
            // 蓝色通道值最大 -> 蓝色
            color_type = 0; // Blue
        } else if (mean_color[1] > mean_color[0] && mean_color[1] > mean_color[2]) {
            // 绿色通道值最大 -> 绿色
            color_type = 1; // Green
        } else if (mean_color[2] > mean_color[0] && mean_color[2] > mean_color[1]) {
            // 红色通道值最大 -> 红色
            color_type = 2; // Red
        }
        
        // 如果成功识别颜色，将颜色和矩形位置存入结果map
        if (color_type != -1) {
            res[color_type] = rect;
        }
    }

    return res;
}
