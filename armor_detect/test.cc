#include "armor_detect.h"
#include "../../include/utils.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool test_armor_detect() {
    LOG_MSG("开始装甲板检测测试");
    
    // 创建装甲板检测器
    ArmorDetector detector;
    
    // 创建一个测试图像（模拟装甲板）
    Mat test_frame = Mat::zeros(480, 640, CV_8UC3);
    
    // 在图像中心绘制两个模拟灯带（红色矩形）
    rectangle(test_frame, Point(280, 200), Point(300, 280), Scalar(0, 0, 255), -1);
    rectangle(test_frame, Point(340, 200), Point(360, 280), Scalar(0, 0, 255), -1);
    
    // 处理图像
    auto armors = detector.processFrame(test_frame);
    
    // 绘制结果（包括轮廓和姿态）
    detector.drawResults(test_frame, armors);
    
    // 保存测试结果图像
    imwrite("armor_detect_test_result.jpg", test_frame);
    
    LOG_MSG("测试图像已保存为 armor_detect_test_result.jpg");
    
    // 检查是否检测到装甲板
    if (armors.size() > 0) {
        LOG_MSG("成功检测到 %d 个装甲板", (int)armors.size());
        
        // 输出每个装甲板的详细信息
        for (const auto& armor : armors) {
            LOG_MSG("装甲板 ID: %d, 角点数量: %d", armor.id, (int)armor.corners.size());
        }
        
        return true;
    } else {
        LOG_WARN("未检测到装甲板");
        return false;
    }
}