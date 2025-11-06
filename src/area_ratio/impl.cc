#include "impls.h"

float compute_area_ratio(const std::vector<cv::Point>& contour) {
    /**
     * 要求：
     *      计算输入的轮廓的面积与它的最小外接矩形面积的比例。
     * 
     * 提示：
     * 无。
     * 
     * 通过条件:
     * 运行测试点，通过即可。
     */

    // 轮廓面积
    double contour_area = cv::contourArea(contour);
    
    // 最小外接矩形（旋转矩形）
    cv::RotatedRect min_rect = cv::minAreaRect(contour);
    
    // 最小外接矩形的面积
    cv::Size2f rect_size = min_rect.size;
    double min_rect_area = rect_size.width * rect_size.height;
    
    // 计算面积比率
    return static_cast<float>(contour_area / min_rect_area);
    
    return 0.f;
}
