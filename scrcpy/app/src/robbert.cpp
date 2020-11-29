#include "robbert.hpp"
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

using namespace cv;
using namespace std;

void houghCircles(Mat src); // decl

int nb_frame = 0;

//Mat avframe_to_cvmat(AVFrame *frame)
//{
//}

void robbert_func(struct screen *screen, AVFrame *frame) {
	LOGI("robbert_func called");
	// converting from AVFrame to OpenCV mat	
	// based on https://answers.opencv.org/question/36948/cvmat-to-avframe/
	AVFrame dst;
	cv::Mat m;

	memset(&dst, 0, sizeof(dst));

	int w = frame->width, h = frame->height;
	int dst_width = w/4, dst_height = h/4;  // less pixels means running faster
	m = cv::Mat(dst_height, dst_width, CV_8UC3);
	dst.data[0] = (uint8_t *)m.data;
	//avpicture_fill( (AVPicture *)&dst,(uint8_t*) dst.data[0], AV_PIX_FMT_BGR24, w, h);
	//av_image_fill_arrays();
	//av_image_fill_arrays( (AVPicture *)&dst, dst.data[0], AV_PIX_FMT_BGR24, w, h);
	//avpicture_fill( (AVPicture *)&dst, dst.data[0], AV_PIX_FMT_BGR24, w, h);
	//avpicture_fill(AVPicture *picture, uint8_t *ptr,enum AVPixelFormat pix_fmt, int width, int height)
	//av_image_fill_arrays(picture->data, picture->linesize, ptr, pix_fmt, width, height, 1);
	av_image_fill_arrays((&dst)->data, (&dst)->linesize, dst.data[0], AV_PIX_FMT_BGR24, dst_width, dst_height, 1);

	struct SwsContext *convert_ctx = NULL;
	enum AVPixelFormat src_pixfmt = (enum AVPixelFormat)frame->format;
	enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_BGR24;
	convert_ctx = sws_getContext(w, h, src_pixfmt, w/4, h/4, dst_pixfmt,
					SWS_FAST_BILINEAR, NULL, NULL, NULL);
	sws_scale(convert_ctx, frame->data, frame->linesize, 0, h,
					dst.data, dst.linesize);
	sws_freeContext(convert_ctx);

	//return m;
	// done converting from AVFrame to OpenCV mat	


	//LOGI("robbert_func executing");

	//Mat mat(screen->frame_size.height, screen->frame_size.width, CV_8UC3, frame->data[0], frame->linesize[0]);
	//Mat mat(2220, 720, CV_8UC3, frame->data[0], frame->linesize[0]);
	//resize(mat, mat, Size(1024, 768), 0, 0, INTER_CUBIC); // resize to 1024x768 resolution
//	LOGI("%d", screen->frame_size.height);
//	LOGI("%d", screen->frame_size.width);
	//LOGI("%d", m.rows);
	//LOGI("%d", m.cols);
	
//	LOGI(frame->data[0])
	//Mat mat(2220, 1080, CV_8UC3, frame->data[0], frame->linesize[0]);
	
	//Mat mot = avframe_to_cvmat(frame);
	//mat = avframe_to_cvmat(frame);

//	imshow("frame", m);
	
	if (nb_frame % 1 == 0){
		houghCircles(m);
	}
	nb_frame++;
//	waitKey(10);
	// Use opencv 1 the C implementation to extract Hough circles
	// use the input manager to send taps to the right place
	// input manager process touch (SDL Touchfingerevent)
}


// Previous ball position
float previous_x = 0.5;
float previous_y = 0.9;
float speed_x;
float speed_y;

void houghCircles(Mat src) {
	LOGI("circles called");

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

    for( size_t i = 0; i < circles.size(); i++ ) // should just be one circle
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
		speed_x = c[0] - previous_x;  // divided by nb frames but that's fixed anyway
		speed_y = c[1] - previous_y;
		previous_x = c[0];
		previous_y = c[1];
        // circle center
        circle( src, center, 1, Scalar(0,100,100), 3, LINE_AA);
        // circle outline
		int radius = c[2];
        circle( src, center, radius, Scalar(255,0,255), 3, LINE_AA);
		double leftmost_picture_width = 270;
		double picture_height = 554;
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
			float timedelta = 5;
			event.x = (c[0]+speed_x*timedelta)/leftmost_picture_width;
			int vertical_offset = 40;  // y axis starts at top
			event.y = (c[1]+speed_y*timedelta)/picture_height;
		}
    }
	imshow("detected circles", src);
    waitKey();
	
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

	if (event.y > 0.2) { // don't let the ball go out of frame! Y axis starts at top!
		robbert_send_tap(event);
		event.type = SDL_FINGERUP;
		LOGI("Tapping at  %f %f ", event.x , event.y);
		robbert_send_tap(event);
	}

}
