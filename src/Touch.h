#ifndef TOUCH_H
#define TOUCH_H

#include <globals.h>
#include <lvgl.h>         // Touch read via LVGL indev (no URTouch needed)


//==============================================================================
// Touch – polled through LVGL indev (touch managed by ESP32_Display_Panel)
//==============================================================================
class TouchInterface {
  public:
    TouchInterface();
    ACTION getAction( PAGE page );
    bool   detectHold( ACTION& act, ACTION lastAct );

  private:
    lv_indev_t *_indev;   // lazily resolved on first use
    ACTION action;
    int x;
    int y;

    lv_indev_t *findTouchIndev();
    ACTION actionFromMAIN    ( int x, int y );
    ACTION actionFromSETTINGS( int x, int y );

    // hold-detection timing
    int act_after      = 1000;
    int wait_after_act = 1000 + 300;

    unsigned long pressTime         = 0;
    unsigned long lastInterruptTime = 0;
};

#endif /* TOUCH_H */

