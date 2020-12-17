# scrcpy-opencv
[Scrcpy](https://github.com/Genymobile/scrcpy) is a tool that streams your android screen to your computer, and allows you to send mouse or keyboard input back to the android device.
This project modifies scrcpy to also send back automatically generated input by passing the frame on to the OpenCV computer vision library.
To see scrcpy-opencv in action, check out this [Youtube video](https://www.youtube.com/watch?v=4Ikzw7TttuU)!

![detected_circle.png](detected_circle.png)

# Run it yourself
## Building scrcpy
Install [dependencies to build scrcpy](https://github.com/Genymobile/scrcpy/blob/master/BUILD.md):  
runtime dependencies  
`sudo apt install ffmpeg libsdl2-2.0-0 adb`  
client build dependencies  
`sudo apt install gcc git pkg-config meson ninja-build libavcodec-dev libavformat-dev libavutil-dev libsdl2-dev`  
server build dependencies  
`sudo apt install openjdk-8-jdk`  

Some more dependencies you might not have installed:  
`sudo apt install g++ cmake libswscale-dev `  

Clone this repository:  
`git clone https://github.com/RobbertH/scrcpy-opencv.git`  
Go into the folder you just cloned:  
`cd scrcpy-opencv`  
Save the directory of the prebuilt server in an environment variable, we'll need this later:  
`PREBUILT_SERVER_PATH=$PWD/prebuilt`  
Change directory to the desktop part of the project, called `scrcpy`,  
`cd scrcpy`  
then run the following command, making sure $PREBUILT_SERVER_PATH contains the prebuilt scrcpy server:    
`meson x --buildtype release --strip -Db_lto=true -Dprebuilt_server=$PREBUILT_SERVER_PATH/scrcpy-server-v1.12.1`  
to configure the build, and then  
`ninja -Cx`  
to build the application, so you can finally  
`./run x`  
to run the modified scrcpy.

Also make sure your android phone is plugged in over USB and has `adb` enabled.

## Building OpenCV
If the `meson` step above fails because of OpenCV, you can install it as follows. It's a slight variation on [the OpenCV tutorial](https://docs.opencv.org/4.5.0/d0/d3d/tutorial_general_install.html).  
`wget https://github.com/opencv/opencv/archive/4.4.0.zip`  
`unzip 4.4.0.zip`  
`cd opencv-4.4.0`  
`mkdir build`  
`cd build`  
`cmake -D OPENCV_GENERATE_PKGCONFIG=ON -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..`  
`make -j3`  
`sudo make install`  
`pkg-config --cflags --libs opencv4`

After this, meson should find opencv4 and you're good to go.

# Making changes
When making changes, make sure to run `meson reconfigure` and rebuild using `ninja -Cx`. Alternatively, you could delete the `x` directory and run the commands above again.

The files that this repository changes versus the original scrcpy repository are the following:
* added: `app/src/opencv_injection.cpp`  This is where all new code is written. It is recommended that you alter this file.
* added: `app/src/opencv_injection.hpp`
* modified: `app/src/scrcpy.c`  Defines a method that can send a tap to the phone (used in opencv_injection.cpp)
* modified: `app/src/scrcpy.h`
* modified: `app/src/screen.c`  This is where the 'hook' is that sends the AVFrame to the opencv_injection function
* modified: `app/meson.build`  to add dependencies and source files to be compiled
* modified: `meson.build`  to enable C++ compilation instead of only C

# Background and context
I wanted to win a soccer game on android, where you have to tap a ball to keep it in the air.
To do so, the 'AVFrame' used in scrcpy is converted to an OpenCV 'mat' so that OpenCV's image processing functions can be run on it.
Then, a circle the size of the football is extracted, using "HoughCircles".
Lastly, an input tap is sent back to the android device, in the center of that circle.

The main modifications to scrcpy include:
* Modifying the meson config files to compile both scrcpy (C) and the additional functions (C++) that depend on OpenCV (C++).
* Converting the scrcpy image format to one that OpenCV can understand.
* Writing the additional functions to use OpenCV and send input to the android device.

Since most work done was part of "making it work" rather than writing the actual code, I figured this might help other people in achieving similar goals.
The concrete case of the soccer game is just one example of what can be achieved with this powerful tool:
any OpenCV function can be run to produce input to the android device, based on the frames.

If you want to read more, there is a [blog post](https://robberthofman.com/projects/2020/03/30/hacking-scrcpy-to-win-fb-soccer-game/).
For questions, simply open an issue.
