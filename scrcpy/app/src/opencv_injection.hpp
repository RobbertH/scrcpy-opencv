#include "video_buffer.h" // AVFrame
#include "screen.h"

#ifdef __cplusplus

#include <opencv2/opencv.hpp> // cv:Mat

void extract_circle_and_tap(cv::Mat src);  // define C++ function outside of extern "C"

extern "C" {
#endif

	void opencv_injection(AVFrame *frame);  // used in screen.c

#ifdef __cplusplus
}
#endif
