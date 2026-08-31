#include "input.h"

InputState input_poll(struct android_app* app) {
    InputState state = {0};
    if (app->inputQueue == NULL) return state;
    AInputEvent* event = NULL;
    while (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
        AInputQueue_preDispatchEvent(app->inputQueue, event);
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            state.touch_x = AMotionEvent_getX(event, 0);
            state.touch_y = AMotionEvent_getY(event, 0);
            state.touch_down = 1;
        }
        AInputQueue_finishEvent(app->inputQueue, event, 1);
    }
    return state;
}
