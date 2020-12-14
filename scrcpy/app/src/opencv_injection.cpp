#include "opencv_injection.hpp"
#include "input_manager.h"
#include "util/log.h"
#include "video_buffer.h" // AVFrame

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/imgutils.h> // av_image_fill_arrays
	#include <libswscale/swscale.h>
}

#include "scrcpy.h"

#define DRAW_CIRCLES false  // Set to true only for debugging. Slows everything down heavily.
#define DEBUG_MESSAGES_ENABLED false

using namespace cv;
using namespace std;

void extract_circle_and_tap(Mat src);

int nb_frame = 0;

/*
 * Receives an AVFrame and converts it to an OpenCV Mat.
 * Then calls further processing of that OpenCV Mat.
 * */
void opencv_injection(struct screen *screen, AVFrame *frame) {
	if (DEBUG_MESSAGES_ENABLED) {
		LOGI("OpenCV injection function called");
	}

	// Converting from AVFrame to OpenCV mat	
	// based on https://answers.opencv.org/question/36948/cvmat-to-avframe/
	AVFrame dst;
	cv::Mat m;

	memset(&dst, 0, sizeof(dst));

	int w = frame->width, h = frame->height;
	int dst_width = w/4, dst_height = h/4;  // less pixels means running faster
	m = cv::Mat(dst_height, dst_width, CV_8UC3);
	dst.data[0] = (uint8_t *)m.data;
	av_image_fill_arrays((&dst)->data, (&dst)->linesize, dst.data[0], AV_PIX_FMT_BGR24, dst_width, dst_height, 1);

	struct SwsContext *convert_ctx = NULL;
	enum AVPixelFormat src_pixfmt = (enum AVPixelFormat)frame->format;
	enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_BGR24;
	convert_ctx = sws_getContext(w, h, src_pixfmt, w/4, h/4, dst_pixfmt,
					SWS_FAST_BILINEAR, NULL, NULL, NULL);
	sws_scale(convert_ctx, frame->data, frame->linesize, 0, h,
					dst.data, dst.linesize);
	sws_freeContext(convert_ctx);

	//imshow("frame", m); // Show the OpenCV Mat frame. Slows everything down.
	//waitKey(10); // needed to display frame using imshow.
	
	if (nb_frame % 1 == 0){ // Optionally process only one in n frames.
		extract_circle_and_tap(m);
	}
	nb_frame++;

}

// Previous ball position
float previous_x = 0.5;
float previous_y = 0.9;
float speed_x;
float speed_y;


void extract_circle_and_tap(Mat src) {
	if (DEBUG_MESSAGES_ENABLED) {
		LOGI("extract_circle_and_tap called");
	}

    // Check if image is loaded fine
    if(src.empty()){
        printf(" Error opening image\n");
        return; // Exit 
    }
	
	// Convert to grayscale and apply blurring
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    medianBlur(gray, gray, 5);
    vector<Vec3f> circles;

	// Extract circles
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                 gray.rows/16,  // change this value to detect circles with different distances to each other
                 100, 30, 30, 70 // change the last two parameters
								// (min_radius & max_radius) to detect larger circles
    );

	// Calculate where to tap
	SDL_TouchFingerEvent event; // This is the event we will use later to tap at a specific coordinate

    for( size_t i = 0; i < circles.size(); i++ ) // Should just be one circle
    {
		// Store circle information in different format
        Vec3i c = circles[i]; // x, y, radius
        Point center = Point(c[0], c[1]);
		int radius = c[2];

		// Estimate speed for trajectory prediction
		speed_x = c[0] - previous_x;  // divided by nb frames but that's fixed anyway
		speed_y = c[1] - previous_y;
		previous_x = c[0];
		previous_y = c[1];
		
		if (DRAW_CIRCLES) {
			// Optionally draw circle on 'src', to display
			circle( src, center, 1, Scalar(0,100,100), 3, LINE_AA);
			// circle outline
			circle( src, center, radius, Scalar(255,0,255), 3, LINE_AA);
		}

		double opencv_mat_width = 270;
		double opencv_mat_height = 554;

		float timedelta = 5;  // how much into the future we want to predict
		event.x = (c[0]+speed_x*timedelta)/opencv_mat_width;
		// int vertical_offset = 40;  // y axis starts at top. Add if you want to tap lower. 
		event.y = (c[1]+speed_y*timedelta)/opencv_mat_height;
    }

	if (DRAW_CIRCLES) {
		imshow("detected circles", src); // show the frame with detected circle. Slows everything down.
		waitKey();  // needed to display frame using imshow. Press a key to advance to the next frame.
	}

	// Set some event parameters.
	event.fingerId = POINTER_ID_MOUSE;
	event.pressure = 1.f;

	// Actual tap
	if (event.y > 0.2) {  // Don't tap the uppermost pixels, to avoid the ball going out of frame. Y axis starts at top.
		if (DEBUG_MESSAGES_ENABLED) {
			LOGI("Tapping at  %f %f ", event.x , event.y);
		}
		event.type = SDL_FINGERDOWN;
		opencv_injection_send_tap(event);
		event.type = SDL_FINGERUP;
		opencv_injection_send_tap(event);
	}

}
