#ifndef CVVARIABLES_H
#define CVVARIABLES_H

#include <opencv2/opencv.hpp>

//两种常用的颜色
extern cv::Scalar BLACK;
extern cv::Scalar WHITE;

namespace cqw {
    extern int QRCODESIZE;
    extern cv::Size FRAME;
    extern const int test_perspective_size;
    extern uint8_t version;
    extern uint8_t version_size;
}





#endif // CVVARIABLES_H
