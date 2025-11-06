#include "impls.h"


std::vector<cv::Mat> erode(const cv::Mat& src_erode, const cv::Mat& src_dilate) {
    /**
     * TODO: 先将图像转换为灰度图像, 然后二值化，然后进行腐蚀操作，具体内容：
     *  1. 将彩色图片 src_erode 转换为灰度图像
     *  2. 将灰度图像进行二值化
     *  3. 将二值化后的两张图片 分别进行 腐蚀 和 膨胀操作
     *  此任务需要你不断调整 erode 和 dilate 函数 的参数，以达到目标效果
     * HINT: 你可能需要以下函数
     *  1. 图像的腐蚀函数 cv::erode，该函数会扩大黑色部分，去除小的白色区域，参数如下
     *      void cv::erode(
     *          InputArray  src,        // 输入的二值图像
     *          OutputArray dst,        // 输出的二值图像
     *          InputArray  kernel,     // 腐蚀的核
     *          Point       anchor = Point(-1,-1), // 锚点
     *          int         iterations = 1,        // 迭代次数
     *          int         borderType = BORDER_CONSTANT, // 边界类型
     *          const Scalar& borderValue = morphologyDefaultBorderValue() // 边界值
     *      )
     *  2. 图像的膨胀函数 cv::dilate，该函数会扩大白色部分，去掉小的的黑色区域，参数如下
     *      void cv::dilate(
     *          InputArray  src,        // 输入的二值图像
     *          OutputArray dst,        // 输出的二值图像
     *          InputArray  kernel,     // 膨胀的核
     *          Point       anchor = Point(-1,-1), // 锚点
     *          int         iterations = 1,        // 迭代次数
     *          int         borderType = BORDER_CONSTANT, // 边界类型
     *          const Scalar& borderValue = morphologyDefaultBorderValue() // 边界值
     *      )
     *  3. 图像的腐蚀和膨胀的核可以使用 cv::getStructuringElement 函数生成，参数如下
     *      cv::Mat cv::getStructuringElement(
     *          int         shape,      // 核的形状 (具体类型可以参考网络资料，或者利用 vscode 的代码提示)
     *          Size        ksize,      // 核的大小
     *          Point       anchor = Point(-1,-1) // 锚点
     *      )
     *       膨胀腐蚀操作的本质是对图像进行卷积(暂时可以理解为在一个像素区域内取平均的意思)操作，
     *      核的大小决定了这个区域的大小
     *   PS: 这里的 二值化函数的参数需要看设置为 thresh = 50, maxval = 255, type = THRESH_BINARY
     *   GOAL:
     *     1. 腐蚀结果图片中，女孩头发里的白点被消除了(类似答案中的样子)
     *     2. dilate 之后的图像中，图中的小脚被消除了(类似答案中的样子)
     *     以上两个检查点需要自己检查，满足条件 则输入 p 通过, 否则输入 f 表示不通过
     */
    
    
    // TODO: 在这里实现你的代码
    
    // 定义输出变量：存储最终处理结果
    cv::Mat dst_erode;  // 存储腐蚀操作的结果图像
    cv::Mat dst_dilate; // 存储膨胀操作的结果图像

    cv::Mat gray_erode;  // 存储src_erode转换后的灰度图像
    cv::Mat gray_dilate; // 存储src_dilate转换后的灰度图像
    
    // 彩转灰
    // 源图像，目标图像，转换类型
    cv::cvtColor(src_erode, gray_erode, cv::COLOR_BGR2GRAY);
    cv::cvtColor(src_dilate, gray_dilate, cv::COLOR_BGR2GRAY);

    cv::Mat binary_erode;
    cv::Mat binary_dilate;
    
    // 灰图二值化
    // 源图像，目标图像，阈值，最大值255，二值化类型
    // 自适应阈值
    cv::threshold(gray_erode, binary_erode, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::threshold(gray_dilate, binary_dilate, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    
    // 创建腐蚀操作使用的核（结构元素）
    // 核形状（矩形），核大小（像素） 1*1 3*3效果不行
    // 消除头发中的白点
    cv::Mat kernel_erode = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    
    // 创建膨胀操作使用的核（结构元素）
    // 参数说明：核形状（矩形），核大小（像素）1*1 5*5效果不行
    // 消除图中的小脚
    cv::Mat kernel_dilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));

    
    // 对二值图像执行腐蚀操作
    // 作用：扩大黑色区域，消除小的白色噪点（如头发中的白点）
    // 参数说明：输入图像，输出图像，腐蚀核
    cv::erode(binary_erode, dst_erode, kernel_erode);
    
    // 对二值图像执行膨胀操作
    // 作用：扩大白色区域，消除小的黑色细节（如图中的小脚）
    // 参数说明：输入图像，输出图像，膨胀核
    cv::dilate(binary_dilate, dst_dilate, kernel_dilate);
    
    // 腐蚀 和 膨胀，两个向量
    return {dst_erode, dst_dilate};
}
