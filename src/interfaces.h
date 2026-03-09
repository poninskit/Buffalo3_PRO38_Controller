#ifndef INTERFACES_H
#define INTERFACES_H

#include <globals.h>
#include <lvgl.h>         // Touch read via LVGL indev (no URTouch needed)

// Forward declaration only — IRremote.hpp included only in interfaces.cpp
class IRrecv;

//==============================================================================
// IR Remote – Apple remote, NEC protocol, IRremote v4.x
//==============================================================================
class RemoteInterface {
  public:
    RemoteInterface( int recvpin = 4 );
    ACTION getAction( PAGE page = MAIN_MENU );

  private:
    IRrecv *_irrecv; 

    unsigned long lastRemoteMillis = 0;
    ACTION action;
    ACTION prevAct;
};

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

#endif /* INTERFACES_H */


// #ifndef INTERFACES_H
// #define INTERFACES_H


// #include <globals.h>
// #include <IRremote2.h> // Remote
// #include <URTouch.h> // For Touchscreen



// //==============================================================================

// class RemoteInterface:IRrecv{
//   public:
//     RemoteInterface( int recvpin = 8 );
//     ACTION getAction( PAGE page = MAIN_MENU );

//   private:
//     decode_results results;
//     unsigned long lastRemoteMillis = 0; //Prevente Debounce in Remote,  it keeps track of time between clicks

//     ACTION action;  // actual action
//     ACTION prevAct; // last action taken for repeat

// };

// //==============================================================================
// class TouchInterface:URTouch{
//   public:
//     TouchInterface(byte tclk = 6, byte tcs = 5, byte tdin = 4, byte dout = 3, byte irq = 2);
//     ACTION getAction( PAGE page );
//     bool detectHold( ACTION& act, ACTION lastAct );

//   private:
//     ACTION action;
//     int x;
//     int y;

//     ACTION actionFromMAIN( int x, int y );
//     ACTION actionFromSETTINGS( int x, int y );

//     //detect button hold
//     int act_after = 1000;
//     int wait_after_act = act_after + 300;

//     unsigned long pressTime = 0;
//     unsigned long lastInterruptTime = 0;
// };
// #endif /* INTERFACES_H */
