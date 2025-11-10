#include "armor_detect.h"
#include "../../include/utils.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace cv;
using namespace std;

// 装甲板跟踪器类实现
ArmorTracker::ArmorTracker(double iou_thresh, int max_miss) 
    : next_id_(0), iou_threshold_(iou_thresh), max_misses_(max_miss) {}

double ArmorTracker::calculateIOU(const Rect& rect1, const Rect& rect2) {
    int x_left = max(rect1.x, rect2.x);
    int y_top = max(rect1.y, rect2.y);
    int x_right = min(rect1.x + rect1.width, rect2.x + rect2.width);
    int y_bottom = min(rect1.y + rect1.height, rect2.y + rect2.height);
    
    if (x_right < x_left || y_bottom < y_top) {
        return 0.0;
    }
    
    int intersection_area = (x_right - x_left) * (y_bottom - y_top);
    int union_area = rect1.area() + rect2.area() - intersection_area;
    
    return static_cast<double>(intersection_area) / union_area;
}

vector<TrackedArmor> ArmorTracker::update(const vector<pair<Rect, vector<Point2f>>>& detections) {
    vector<bool> detection_matched(detections.size(), false);
    vector<bool> tracker_matched(tracked_armors_.size(), false);
    
    for (size_t i = 0; i < tracked_armors_.size(); i++) {
        double best_iou = iou_threshold_;
        int best_detection_idx = -1;
        
        for (size_t j = 0; j < detections.size(); j++) {
            if (detection_matched[j]) continue;
            
            double iou = calculateIOU(tracked_armors_[i].bbox, detections[j].first);
            if (iou > best_iou) {
                best_iou = iou;
                best_detection_idx = j;
            }
        }
        
        if (best_detection_idx != -1) {
            tracked_armors_[i].bbox = detections[best_detection_idx].first;
            tracked_armors_[i].corners = detections[best_detection_idx].second;
            tracked_armors_[i].hits++;
            tracked_armors_[i].misses = 0;
            tracked_armors_[i].age++;
            
            detection_matched[best_detection_idx] = true;
            tracker_matched[i] = true;
        } else {
            tracked_armors_[i].misses++;
            tracked_armors_[i].age++;
        }
    }
    
    tracked_armors_.erase(
        remove_if(tracked_armors_.begin(), tracked_armors_.end(),
            [this](const TrackedArmor& armor) {
                return armor.misses > max_misses_;
            }),
        tracked_armors_.end()
    );
    
    for (size_t i = 0; i < detections.size(); i++) {
        if (!detection_matched[i]) {
            tracked_armors_.emplace_back(next_id_++, detections[i].first, detections[i].second);
        }
    }
    
    return tracked_armors_;
}

void ArmorTracker::clear() {
    tracked_armors_.clear();
    next_id_ = 0;
}

// 装甲板检测器类实现
ArmorDetector::ArmorDetector() {
    // 初始化相机参数
    camera_matrix_ = (Mat_<double>(3, 3) <<
        9.28130989e+02, 0, 3.77572945e+02,
        0, 9.30138391e+02, 2.83892859e+02,
        0, 0, 1);
    
    dist_coeffs_ = (Mat_<double>(5, 1) <<
        -2.54433647e-01, 5.69431382e-01, 3.65405229e-03, 
        -1.09433818e-03, -1.33846840e+00);
    
    float armor_width = 0.2f;
    float armor_height = 0.1f;
    
    obj_points_ = {
        Point3f(-armor_width/2, -armor_height/2, 0),
        Point3f(armor_width/2, -armor_height/2, 0),
        Point3f(armor_width/2, armor_height/2, 0),
        Point3f(-armor_width/2, armor_height/2, 0)
    };
}

Mat ArmorDetector::preprocessFrame(const Mat& frame) {
    Mat gray, binary;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    adaptiveThreshold(gray, binary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, 
                     THRESH_BINARY, 11, 2);
    
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel);
    morphologyEx(binary, binary, MORPH_OPEN, kernel);
    
    return binary;
}

vector<RotatedRect> ArmorDetector::findLightBars(const Mat& binary) {
    vector<vector<Point>> contours;
    vector<RotatedRect> light_bars;
    
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        if (contourArea(contour) < 100) continue;
        
        RotatedRect rect = minAreaRect(contour);
        Size2f size = rect.size;
        float aspect_ratio = max(size.width, size.height) / min(size.width, size.height);
        
        if (aspect_ratio > 2.0) {
            light_bars.push_back(rect);
        }
    }
    
    return light_bars;
}

vector<pair<RotatedRect, RotatedRect>> ArmorDetector::pairLightBars(const vector<RotatedRect>& light_bars) {
    vector<pair<RotatedRect, RotatedRect>> pairs;
    
    for (size_t i = 0; i < light_bars.size(); i++) {
        for (size_t j = i + 1; j < light_bars.size(); j++) {
            const RotatedRect& bar1 = light_bars[i];
            const RotatedRect& bar2 = light_bars[j];
            
            double distance = norm(bar1.center - bar2.center);
            double angle_diff = abs(bar1.angle - bar2.angle);
            
            if (angle_diff < 15 && distance > 20 && distance < 200) {
                pairs.push_back(make_pair(bar1, bar2));
            }
        }
    }
    
    return pairs;
}

vector<Point2f> ArmorDetector::calculateArmorCorners(const RotatedRect& left_bar, const RotatedRect& right_bar) {
    Point2f left_points[4], right_points[4];
    left_bar.points(left_points);
    right_bar.points(right_points);
    
    vector<Point2f> left_right_points;
    for (int i = 0; i < 4; i++) {
        if (left_points[i].x > left_bar.center.x) {
            left_right_points.push_back(left_points[i]);
        }
    }
    
    vector<Point2f> right_left_points;
    for (int i = 0; i < 4; i++) {
        if (right_points[i].x < right_bar.center.x) {
            right_left_points.push_back(right_points[i]);
        }
    }
    
    sort(left_right_points.begin(), left_right_points.end(), 
         [](const Point2f& a, const Point2f& b) { return a.y < b.y; });
    sort(right_left_points.begin(), right_left_points.end(), 
         [](const Point2f& a, const Point2f& b) { return a.y < b.y; });
    
    vector<Point2f> corners = {
        left_right_points[0],
        right_left_points[0],
        right_left_points[1],
        left_right_points[1]
    };
    
    return corners;
}

bool ArmorDetector::estimatePose(const vector<Point2f>& corners, Vec3d& rvec, Vec3d& tvec) {
    try {
        return solvePnP(obj_points_, corners, camera_matrix_, dist_coeffs_, rvec, tvec);
    } catch (const Exception& e) {
        cerr << "Pose estimation error: " << e.what() << endl;
        return false;
    }
}

vector<TrackedArmor> ArmorDetector::processFrame(const Mat& frame) {
    Mat binary = preprocessFrame(frame);
    vector<RotatedRect> light_bars = findLightBars(binary);
    vector<pair<RotatedRect, RotatedRect>> light_pairs = pairLightBars(light_bars);
    
    vector<pair<Rect, vector<Point2f>>> detections;
    for (const auto& pair : light_pairs) {
        vector<Point2f> corners = calculateArmorCorners(pair.first, pair.second);
        Rect bbox = boundingRect(corners);
        detections.push_back(make_pair(bbox, corners));
    }
    
    return tracker_.update(detections);
}

void ArmorDetector::drawResults(Mat& frame, const vector<TrackedArmor>& armors) {
    // 首先绘制装甲板轮廓
    drawArmorContours(frame, armors);
    
    for (const auto& armor : armors) {
        // 绘制边界框（绿色矩形）
        rectangle(frame, armor.bbox, Scalar(0, 255, 0), 2);
        
        // 绘制角点（蓝色圆点）
        for (const auto& corner : armor.corners) {
            circle(frame, corner, 3, Scalar(255, 0, 0), -1);
        }
        
        // 绘制ID信息
        string id_text = "ID: " + to_string(armor.id);
        putText(frame, id_text, Point(armor.bbox.x, armor.bbox.y - 10),
               FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
        
        // 绘制跟踪状态
        string status = "H:" + to_string(armor.hits) + " M:" + to_string(armor.misses);
        putText(frame, status, Point(armor.bbox.x, armor.bbox.y + armor.bbox.height + 20),
               FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
        
        // 计算并绘制3D姿态
        Vec3d rvec, tvec;
        if (estimatePose(armor.corners, rvec, tvec)) {
            // 绘制坐标轴
            drawCoordinateAxes(frame, rvec, tvec);
            
            // 获取姿态信息字符串
            string pose_info = getPoseInfo(tvec, rvec);
            
            // 在图像上显示姿态信息
            putText(frame, pose_info, Point(armor.bbox.x, armor.bbox.y + armor.bbox.height + 40),
                   FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
            
            // 在控制台输出姿态信息（可选）
            cout << "装甲板 " << armor.id << " 姿态信息: " << pose_info << endl;
        }
    }
}

// 新增：绘制装甲板轮廓
void ArmorDetector::drawArmorContours(Mat& frame, const vector<TrackedArmor>& armors) {
    for (const auto& armor : armors) {
        // 将角点转换为整数类型，用于绘制轮廓
        vector<Point> int_corners;
        for (const auto& corner : armor.corners) {
            int_corners.push_back(Point(static_cast<int>(corner.x), static_cast<int>(corner.y)));
        }
        
        // 绘制装甲板轮廓（红色多边形）
        // 参数说明：
        // - frame: 目标图像
        // - int_corners: 轮廓点集
        // - true: 是否闭合轮廓
        // - Scalar(0, 0, 255): 红色 (BGR格式)
        // - 2: 线宽
        polylines(frame, int_corners, true, Scalar(0, 0, 255), 2);
        
        // 可选：填充轮廓（半透明）
        // vector<vector<Point>> contours = {int_corners};
        // Mat overlay = frame.clone();
        // fillPoly(overlay, contours, Scalar(0, 0, 255));
        // addWeighted(overlay, 0.3, frame, 0.7, 0, frame);
    }
}

// 新增：获取姿态信息的字符串
string ArmorDetector::getPoseInfo(const Vec3d& tvec, const Vec3d& rvec) {
    stringstream ss;
    
    // 设置输出精度
    ss << fixed << setprecision(2);
    
    // 位置信息
    ss << "Pos:(" << tvec[0] << "," << tvec[1] << "," << tvec[2] << ")m";
    
    // 可选：添加旋转信息
    ss << " Rot:(" << rvec[0] << "," << rvec[1] << "," << rvec[2] << ")";
    
    return ss.str();
}

void ArmorDetector::drawCoordinateAxes(Mat& frame, const Vec3d& rvec, const Vec3d& tvec) {
    vector<Point3f> axis_points = {
        Point3f(0, 0, 0),
        Point3f(0.1, 0, 0),
        Point3f(0, 0.1, 0),
        Point3f(0, 0, 0.1)
    };
    
    vector<Point2f> image_points;
    projectPoints(axis_points, rvec, tvec, camera_matrix_, dist_coeffs_, image_points);
    
    // 绘制坐标轴
    arrowedLine(frame, image_points[0], image_points[1], Scalar(0, 0, 255), 3); // X轴 - 红色
    arrowedLine(frame, image_points[0], image_points[2], Scalar(0, 255, 0), 3); // Y轴 - 绿色
    arrowedLine(frame, image_points[0], image_points[3], Scalar(255, 0, 0), 3); // Z轴 - 蓝色
    
    // 添加坐标轴标签
    putText(frame, "X", image_points[1], FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
    putText(frame, "Y", image_points[2], FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
    putText(frame, "Z", image_points[3], FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2);
}