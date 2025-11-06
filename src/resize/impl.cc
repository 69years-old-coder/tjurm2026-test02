#include "impls.h"


cv::Mat my_resize(const cv::Mat& input, float scale) { //原图、缩放比例
    /**
     * 要求：
     *      实现resize算法，只能使用基础的语法，比如说for循环，Mat的基本操作。不能
     * 用cv::resize。resize算法的内容自行查找学习，不是很难。
     * 
     * 提示：
     * 无。
     * 
     * 通过条件：
     * 运行测试点，你的结果跟答案长的差不多就行。
     */

    /*
    Mat功能：
    class Mat {
        public:
        // 头部信息（元数据）
        int rows;        // 行数（图像高度）
        int cols;        // 列数（图像宽度）
        int type();      // 数据类型和通道数
        int channels();  // 通道数
        int depth();     // 数据类型深度
    
        // 数据指针
        uchar* data;     // 指向实际像素数据的指针
    
        // 其他管理信息...
    };
    
    
    */

    int new_rows = input.rows * scale, new_cols = input.cols * scale;
    // IMPLEMENT YOUR CODE HERE
    
    cv::Mat output(new_rows, new_cols, input.type());
    
    // 计算缩放比例 新图-->旧图
    float scale_x = static_cast<float>(input.cols) / new_cols;
    float scale_y = static_cast<float>(input.rows) / new_rows;
    
    
    for (int y = 0; y < new_rows; y++) {
        for (int x = 0; x < new_cols; x++) {
            //每个像素在输入图像中的对应位置
            float src_x = x * scale_x;
            float src_y = y * scale_y;
            
            // 双线性插值
            int x0 = static_cast<int>(src_x);
            int y0 = static_cast<int>(src_y);
            int x1 = x0 + 1;
            int y1 = y0 + 1;
            
            // 边界检查
            if (x1 >= input.cols) x1 = input.cols - 1;
            if (y1 >= input.rows) y1 = input.rows - 1;
            if (x0 >= input.cols) x0 = input.cols - 1;
            if (y0 >= input.rows) y0 = input.rows - 1;
            
            // 计算权重
            float dx = src_x - x0;
            float dy = src_y - y0;
            float w1 = (1 - dx) * (1 - dy);
            float w2 = dx * (1 - dy);
            float w3 = (1 - dx) * dy;
            float w4 = dx * dy;
            
            // 根据通道数处理
            if (input.channels() == 1) {
                // 单通道图像
                float value = input.at<uchar>(y0, x0) * w1 +
                              input.at<uchar>(y0, x1) * w2 +
                              input.at<uchar>(y1, x0) * w3 +
                              input.at<uchar>(y1, x1) * w4;
                output.at<uchar>(y, x) = cv::saturate_cast<uchar>(value);//第y行第x列
            } else if (input.channels() == 3) {
                // 三通道图像
                cv::Vec3b p1 = input.at<cv::Vec3b>(y0, x0);
                cv::Vec3b p2 = input.at<cv::Vec3b>(y0, x1);
                cv::Vec3b p3 = input.at<cv::Vec3b>(y1, x0);
                cv::Vec3b p4 = input.at<cv::Vec3b>(y1, x1);
                
                cv::Vec3b result;
                for (int c = 0; c < 3; c++) {
                    float value = p1[c] * w1 + p2[c] * w2 + p3[c] * w3 + p4[c] * w4;
                    result[c] = cv::saturate_cast<uchar>(value);
                }
                output.at<cv::Vec3b>(y, x) = result;
            }
        }
    }
    
    return output;
}
