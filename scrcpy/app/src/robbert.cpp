#include "robbert.hpp"
#include "input_manager.h"
#include "util/log.h"
#include "video_buffer.h" // AVFrame

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "scrcpy.h"

using namespace cv;
using namespace std;

void houghCircles(Mat src); // decl

int nb_frame = 0;

void robbert_func(struct screen *screen, AVFrame *frame) {
	//LOGI("robbert_func executing");

	Mat mat(screen->frame_size.height, screen->frame_size.width, CV_8UC3, frame->data[0], frame->linesize[0]);
	//Mat mat(2220, 720, CV_8UC3, frame->data[0], frame->linesize[0]);
	resize(mat, mat, Size(1024, 768), 0, 0, INTER_CUBIC); // resize to 1024x768 resolution
//	LOGI("%d", screen->frame_size.height);
//	LOGI("%d", screen->frame_size.width);
//	LOGI("%d", mat.rows);
//	LOGI("%d", mat.cols);
	
//	LOGI(frame->data[0])
	//Mat mat(2220, 1080, CV_8UC3, frame->data[0], frame->linesize[0]);
	//imshow("frame", mat);
	
	if (nb_frame % 6 == 0){
		houghCircles(mat);
	}
	nb_frame++;
	//waitKey(10);
	// Use opencv 1 the C implementation to extract Hough circles
	// use the input manager to send taps to the right place
	// input manager process touch (SDL Touchfingerevent)
}


void houghCircles(Mat src) {

    // Loads an image
    // Mat src = imread( samples::findFile( filename ), IMREAD_COLOR );
    // Check if image is loaded fine
    if(src.empty()){
        printf(" Error opening image\n");
        // printf(" Program Arguments: [image_name -- default %s] \n", filename);
        return; // Exit 
    }
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    medianBlur(gray, gray, 5);
    vector<Vec3f> circles;
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                 gray.rows/16,  // change this value to detect circles with different distances to each other
                 100, 30, 30, 70 // change the last two parameters
            // (min_radius & max_radius) to detect larger circles
    );

	// --------------------------------------------------------------------------------
	SDL_TouchFingerEvent event; // needed later to tap

    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
//        Point center = Point(c[0], c[1]);
        // circle center
//        circle( src, center, 1, Scalar(0,100,100), 3, LINE_AA);
        // circle outline
//        int radius = c[2];
//        circle( src, center, radius, Scalar(255,0,255), 3, LINE_AA);
		double leftmost_picture_width = 340;
		double picture_height = 742;
//		double phone_screen_width = 1080; // will be used in a division
//		double phone_screen_height = 1950;
//		double width_multiplier = ((double) phone_screen_width) / leftmost_picture_width;
//		double height_multiplier = ((double) phone_screen_height) / picture_height;
		if (c[0] < leftmost_picture_width) { // only interested in leftmost circle
//			double tapX = (c[0]*width_multiplier);
//			double tapY = (c[1]*height_multiplier);
//			LOGI("circle found at ");
//			LOGI(" %d %d ", c[0], c[1]);
//			LOGI("Conversion to tap:");
//			LOGI(" %d %d ", (int) tapX, (int) tapY);
			event.x = c[0]/leftmost_picture_width;
			event.y = c[1]/picture_height;
		}
    }
	//imshow("detected circles", src);
    //waitKey();
	
	// INPUT THE LATEST TAP VALUE
	//struct input_manager* input_manager_pointer; // this one is initialized at scrcpy.c
//	input_manager_pointer = give_robbert_input_manager();
	event.fingerId = POINTER_ID_MOUSE; // ?
	// SDL touch event coordinates are normalized in the range [0; 1]
	// event.x = 0.5;
	// event.y = 0.5;
	event.pressure = 1.f; // ??
	event.type = SDL_FINGERDOWN; // ??
	
	// event-> type can be  SDL_FINGERDOWN or SDL_FINGERUP

	//input_manager_process_touch( input_manager_pointer , &event);

	if (event.y > 0.5) { // don't let the ball go out of frame! Y axis starts at top!
		robbert_send_tap(event);
		event.type = SDL_FINGERUP;
		LOGI(" %f %f ", event.x , event.y);
		robbert_send_tap(event);
	}

}
