#include "video_buffer.h" // AVFrame
#include "screen.h"

#ifdef __cplusplus
extern "C" {
#endif

	void robbert_func(struct screen *screen, AVFrame *frame);
	void robbert_send_tap(SDL_TouchFingerEvent event);

#ifdef __cplusplus
}
#endif
