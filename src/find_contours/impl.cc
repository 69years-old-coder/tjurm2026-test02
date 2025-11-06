#include "impls.h"


std::vector<std::vector<cv::Point>> find_contours(const cv::Mat& input) {
    /**
     * 要求：
     * 使用cv::findContours函数，从输入图像（3个通道）中找出所有的最内层轮廓。
     * 将它们保存起来作为函数返回值就行。contours的中文意思就是轮廓。
     * 
     * 提示：
     * 1. 使用cv::imshow来查看输入图像。
     * 2. 使用cv::drawContours来在一张图上绘制轮廓。
     * 3. 直接使用原图一般而言不利于轮廓的寻找，可以做一些简单的处理。
     * 4. findContours函数可以返回轮廓的层次结构，理解层次结构的保存方式并使用它（重点）。
     * 
     * 通过条件：
     * 运行测试点，你找到的轮廓与答案的轮廓一样就行。
     */
    
    
    // IMPLEMENT YOUR CODE HERE
    /*
     * 功能：从输入图像中找出所有的最内层轮廓
     * 输入：input - 3通道彩色图像
     * 输出：res - 包含所有最内层轮廓的向量，每个轮廓由一系列点组成
     */
    
    // 存储最内层轮廓
    std::vector<std::vector<cv::Point>> res;
    
    // 彩色-->灰度-->二值，简化处理
    cv::Mat gray;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);// 源图像，目标图像，转换类型
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);//源图像，目标图像，阈值，最大值，二值化类型
    

    std::vector<std::vector<cv::Point>> contours;// 用于存储所有找到的轮廓
    std::vector<cv::Vec4i> hierarchy;// 用于存储轮廓的层次结构信息，cv::Vec4i表示每个轮廓的4个层次信息
    /*
    对于cv::RETR_TREE 模式，hierarchy中的每个元素是一个包含4个整数的数组，表示：
    [0]下一个轮廓的索引
    [1]前一个轮廓的索引
    [2]第一个子轮廓的索引
    [3]父轮廓的索引
    没有对应关系的情况,则索引值为-1。
    对于cv::RETR_EXTERNAL 模式，hierarchy中的每个元素的[2]和[3]值始终为-1，因为只检索最外层轮廓。
    对于cv::RETR_LIST 模式，hierarchy中的每个元素的[2]和[3]值始终为-1，因为不建立轮廓之间的层次关系。
    对于cv::RETR_CCOMP 模式，hierarchy中的每个元素的[2]值表示下一个同级轮廓的索引，[3]值表示父轮廓的索引。

    */
    // 使用findContours函数查找轮廓
    // 参数说明：
    //   binary - 输入二值图像
    //   contours - 输出轮廓点集
    //   hierarchy - 输出轮廓层次结构
    //   cv::RETR_TREE - 检索所有轮廓并重建完整层次结构
    //   cv::CHAIN_APPROX_SIMPLE - 压缩水平、垂直和对角线段，只保留端点
    cv::findContours(binary, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    

    // 遍历所有轮廓，检查哪些是最内层轮廓
    for (int i = 0; i < contours.size(); i++) {
        // hierarchy[i]是一个包含4个整数的数组，表示：
        // [0]下一个轮廓索引, [1]前一个轮廓索引, [2]第一个子轮廓索引, [3]父轮廓索引
        
        // 检查当前轮廓是否有子轮廓
        // 如果hierarchy[i][2] == -1，表示没有子轮廓，即这是最内层轮廓
        if (hierarchy[i][2] == -1) {
            // 将最内层轮廓添加到结果中
            res.push_back(contours[i]);
        }
    }
    
    return res;
}
