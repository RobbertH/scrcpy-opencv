# scrcpy-opencv
[Scrcpy](https://github.com/Genymobile/scrcpy) is a tool that streams your android screen to your computer, and allows you to send mouse or keyboard input back to the android device.
This repository modifies scrcpy to also send back automatically generated input by passing the frame on to the OpenCV computer vision library.

# Video
[Youtube link](https://www.youtube.com/watch?v=4Ikzw7TttuU)

# Blog post
[Blog post link](https://robberthofman.com/projects/2020/03/30/hacking-scrcpy-to-win-fb-soccer-game/)

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

# Future work
* Clean up code. It's currently a scratchpad.
* Use [a better conversion](https://sourcey.com/articles/ffmpeg-avpacket-to-opencv-mat-converter) from AVFrame to Mat. The current conversion does outputs three times the same thing (color channels?).
* Trajectory prediction to tap more accurately. (Fit observations to parabolic curve).

