#include "impls.h"
#include <algorithm>

float compute_iou(const cv::Rect& a, const cv::Rect& b) {
    /**
     * 要求：
     *      有一个重要的指标叫做“交并比”，简称“IOU”，可以用于衡量
     * 两个矩形的重合程度，本练习要求自行学习交并比的计算方式，计算输入
     * 的两个矩形的交并比。
     * 
     * 提示：
     * (1) 注意OpenCV中规定的xy坐标系方向跟平常的xy坐标系不一样。
     * (2) 注意矩形的x, y, width, height四个属性在坐标系内的含义。
     * (3) 注意分母不要除以0（不过我们不会测试这一点）
     * 
     * 通过条件：
     * 运行测试点，显示通过就行，不通过会告诉你哪一组矩形错了。
    */
    // IMPLEMENT YOUR CODE HERE
    /*

    ·------> x
    |
    |
    |
    |
    V
    y
              __________
             |          |
             |    a     |
             |__________|
                  _________
                 |         |
                 |    b    |
                 |_________|
    */


    //交叉区域的左上角和右下角
    int x1 = std::max(a.x, b.x);
    int y1 = std::max(a.y, b.y);
    int x2 = std::min(a.x + a.width, b.x + b.width);
    int y2 = std::min(a.y + a.height, b.y + b.height);
    
    //交叉区域的宽度和高度
    int w = std::max(0, x2 - x1);//无边界时为负
    int h = std::max(0, y2 - y1);
    float A_and_B = w * h;//AB
    
    float area_a = a.width * a.height;
    float area_b = b.width * b.height;
    float A_or_B = area_a + area_b - A_and_B;//A并B=A+B-AB
    
    // 6. 避免除零错误
    if (A_and_B <= 0) {
        return 0.f;//float的0
    }

    return A_and_B / A_or_B;
}
