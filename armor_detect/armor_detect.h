#ifndef ARMOR_DETECT_H
#define ARMOR_DETECT_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <algorithm>

// 跟踪的装甲板结构体
struct TrackedArmor {
    int id;
    cv::Rect bbox;
    std::vector<cv::Point2f> corners;
    int age;
    int hits;
    int misses;
    
    TrackedArmor(int _id, const cv::Rect& _bbox, const std::vector<cv::Point2f>& _corners)
        : id(_id), bbox(_bbox), corners(_corners), age(0), hits(1), misses(0) {}
};

// 装甲板跟踪器类
class ArmorTracker {
private:
    std::vector<TrackedArmor> tracked_armors_;
    int next_id_;
    double iou_threshold_;
    int max_misses_;
    
public:
    ArmorTracker(double iou_thresh = 0.3, int max_miss = 5);
    double calculateIOU(const cv::Rect& rect1, const cv::Rect& rect2);
    std::vector<TrackedArmor> update(const std::vector<std::pair<cv::Rect, std::vector<cv::Point2f>>>& detections);
    void clear();
};

// 装甲板检测器类
class ArmorDetector {
private:
    ArmorTracker tracker_;
    cv::Mat camera_matrix_;
    cv::Mat dist_coeffs_;
    std::vector<cv::Point3f> obj_points_;
    
    cv::Mat preprocessFrame(const cv::Mat& frame);
    std::vector<cv::RotatedRect> findLightBars(const cv::Mat& binary);
    std::vector<std::pair<cv::RotatedRect, cv::RotatedRect>> pairLightBars(const std::vector<cv::RotatedRect>& light_bars);
    std::vector<cv::Point2f> calculateArmorCorners(const cv::RotatedRect& left_bar, const cv::RotatedRect& right_bar);
    bool estimatePose(const std::vector<cv::Point2f>& corners, cv::Vec3d& rvec, cv::Vec3d& tvec);
    void drawCoordinateAxes(cv::Mat& frame, const cv::Vec3d& rvec, const cv::Vec3d& tvec);
    
public:
    ArmorDetector();
    std::vector<TrackedArmor> processFrame(const cv::Mat& frame);
    void drawResults(cv::Mat& frame, const std::vector<TrackedArmor>& armors);
};

// 测试函数声明
bool test_armor_detect();

#endif // ARMOR_DETECT_H