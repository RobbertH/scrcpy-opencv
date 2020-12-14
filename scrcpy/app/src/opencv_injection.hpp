#include "video_buffer.h" // AVFrame
#include "screen.h"

#ifdef __cplusplus
extern "C" {
#endif

	void opencv_injection(struct screen *screen, AVFrame *frame);
	void opencv_injection_send_tap(SDL_TouchFingerEvent event);

#ifdef __cplusplus
}
#endif
