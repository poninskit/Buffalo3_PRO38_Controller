#include "Touch.h"
#include "lvgl_v8_port.h"



//==============================================================================
// TouchInterface  –  polls LVGL indev (touch managed by ESP32_Display_Panel)
//==============================================================================
/*
  Display: BOARD_VIEWE_UEDX80480070E_WB_A  →  800 x 480 pixels  (landscape)

  Main-menu touch zones:
  ┌──────────────────────────────────────────────────────────────────────────┐
  │  [1] Channel      (40,20)-(244,94)   │              │  [5] Settings  │
  │                                       │              │  (660,20)-(760,94)│
  │  [2] Vol-down     (40,320)-(124,404) │  [4] Mute    │                   │
  │  [3] Vol-up      (170,320)-(254,404) │  (390,200)-  │                   │
  │                                       │  (760,440)   │                   │
  └──────────────────────────────────────────────────────────────────────────┘

  Settings-menu touch zones:
  ┌──────────────────────────────────────────────────────────────────────────┐
  │  [1] FIR Filter   (50,25)-(630,105)  │  [5] Back  (660,20)-(760,94)  │
  │  [2] IIR Bandw    (50,120)-(630,200)                                    │
  │  [3] DPLL         (50,215)-(630,295)                                    │
  │  [4] Jitter El    (50,310)-(630,390)                                    │
  └──────────────────────────────────────────────────────────────────────────┘
*/

TouchInterface::TouchInterface()
    : _indev( nullptr )
{
    x = 0;
    y = 0;
}

//------------------------------------------------------------------------------
lv_indev_t* TouchInterface::findTouchIndev()
{
    lv_indev_t *dev = lv_indev_get_next( nullptr );
    while ( dev ) {
        if ( lv_indev_get_type(dev) == LV_INDEV_TYPE_POINTER ) return dev;
        dev = lv_indev_get_next( dev );
    }
    return nullptr;
}

//------------------------------------------------------------------------------
ACTION TouchInterface::getAction( PAGE page )
{
    // Lazy-init: LVGL indev is registered by Graphics constructor
    if ( _indev == nullptr ) {
        _indev = findTouchIndev();
        if ( _indev == nullptr ) return NONE;
    }

    if ( !lvgl_port_lock(10) ) return NONE;

    // lv_indev_get_state() does not exist in LVGL v8 (added in v9).
    // Access the internal proc struct directly – the fields are public in v8.
    lv_indev_state_t state = _indev->proc.state;
    lv_point_t       point = _indev->proc.types.pointer.act_point;  

    lvgl_port_unlock();

    if ( state != LV_INDEV_STATE_PRESSED ) return NONE;

    x = point.x;
    y = point.y;

    switch ( page ) {
        case MAIN_MENU:     return actionFromMAIN    ( x, y );
        case SETTINGS_MENU: return actionFromSETTINGS( x, y );
    }
    return NONE;
}

//------------------------------------------------------------------------------
ACTION TouchInterface::actionFromMAIN( int x, int y )
{
    action = NONE;

    //                   p1              p2
    Rect channel  = { { 40,  20}, { 244,  94} };  // 1 – channel icon
    Rect vol_down = { { 40, 320}, { 124, 404} };  // 2 – vol-down icon
    Rect vol_up   = { {170, 320}, { 254, 404} };  // 3 – vol-up icon
    Rect mute     = { {390, 200}, { 760, 440} };  // 4 – mute area (right half)
    Rect settings = { {660,  20}, { 760,  94} };  // 5 – settings icon

    if ( x >= channel.p1.x && x <= channel.p2.x &&
         y >= channel.p1.y && y <= channel.p2.y ) {
        action = CHANNEL_RIGHT;
        LOG("CHANNEL_RIGHT\n");

    } else if ( x >= vol_down.p1.x && x <= vol_down.p2.x &&
                y >= vol_down.p1.y && y <= vol_down.p2.y ) {
        action = VOLUME_DOWN;
        LOG("VOLUME_DOWN\n");

    } else if ( x >= vol_up.p1.x && x <= vol_up.p2.x &&
                y >= vol_up.p1.y && y <= vol_up.p2.y ) {
        action = VOLUME_UP;
        LOG("VOLUME_UP\n");

    } else if ( x >= mute.p1.x && x <= mute.p2.x &&
                y >= mute.p1.y && y <= mute.p2.y ) {
        action = ENTER;
        LOG("ENTER\n");

    } else if ( x >= settings.p1.x && x <= settings.p2.x &&
                y >= settings.p1.y && y <= settings.p2.y ) {
        action = MENU;
        LOG("MENU\n");
    }

    return action;
}

//------------------------------------------------------------------------------
ACTION TouchInterface::actionFromSETTINGS( int x, int y )
{
    action = NONE;

    Rect fir_filter = { { 50,  25}, {630, 105} };  // 1
    Rect iir_bandw  = { { 50, 120}, {630, 200} };  // 2
    Rect dpll_band  = { { 50, 215}, {630, 295} };  // 3
    Rect jitter_el  = { { 50, 310}, {630, 390} };  // 4
    Rect settings   = { {660,  20}, {760,  94} };  // 5 – back

    if ( x >= fir_filter.p1.x && x <= fir_filter.p2.x ) {

        if      ( y >= fir_filter.p1.y && y <= fir_filter.p2.y ) {
            action = SET_FIR_FILTER;   LOG("FIR\n");
        } else if ( y >= iir_bandw.p1.y && y <= iir_bandw.p2.y ) {
            action = SET_IIR_BANDWIDTH; LOG("IIR\n");
        } else if ( y >= dpll_band.p1.y && y <= dpll_band.p2.y ) {
            action = SET_DPLL;          LOG("DPLL\n");
        } else if ( y >= jitter_el.p1.y && y <= jitter_el.p2.y ) {
            action = TOGGLE_JE;         LOG("TOGGLE JE\n");
        }

    } else if ( x >= settings.p1.x && x <= settings.p2.x &&
                y >= settings.p1.y && y <= settings.p2.y ) {
        action = MENU;
        LOG("MENU\n");
    }

    return action;
}

//------------------------------------------------------------------------------
bool TouchInterface::detectHold( ACTION& act, ACTION lastAct )
{
    int interval = millis() - pressTime;

    if ( lastAct != act || interval > wait_after_act ) {
        pressTime = lastInterruptTime = millis();
        LOG("----------first touch----------\n");
    } else {
        int no_touch = millis() - lastInterruptTime;
        if ( no_touch > 200 ) {
            act = RESET;
            pressTime = lastInterruptTime = millis();
            LOG("Interruption, start new counter (reset)\n");
            return false;
        } else {
            lastInterruptTime = millis();
        }
        interval = millis() - pressTime;
        LOG( interval );
        if ( interval > act_after ) {
            LOG("\nhold button: " + String(action));
            act = RESET;
            pressTime = millis();
            return true;
        }
    }
    return false;
}





// #include <interfaces.h>

// //==============================================================================
// //==============================================================================
// RemoteInterface::RemoteInterface(int recvpin)
//                 :IRrecv(recvpin)
// {
//     enableIRIn(); // Start the IR receiver
//     prevAct = NONE;
// }

// //------------------------------------------------------------------------------
// /*
// Bits 1-16: Apple Custom ID (Always the same)
// Bits 17-23: The Pairing ID (This changes per remote!)
// Bits 24-31: The Button Command
// Bit 32: Odd parity bit

// mask the results with 0x0000FF00 to skip pairing
// or update switch to short masked codes
// UP: 0x0D, DOWN: 0x0B, LEFT: 0x08, RIGHT: 0x07, MENU: 0x02, CENTER: 0x5D/0x04
// */

// ACTION RemoteInterface::getAction( PAGE page ){
//   if ( !decode( &results ) )
//     return NONE;

//   uint32_t val = results.value;

//   // Lower byte order of argument depends on IR library.
//   // For standard NEC (32 bits, LSB first):
//   // Raw value: 0x77e1b036
//   // Bit positions (MSB → LSB):
//   // 31........24 23........16 15........8 7........0
//   //    77           e1          b0         36
//   // But NEC logical order is:
//   // Address (LSB) | Address (MSB) | Command | ~Command
//   uint16_t custom_id  = (val >> 16) & 0xFFFF;  // bits 0–15 
//   uint8_t  command    = (val >> 8)  & 0xFF;    // bits 16–22, ignore for now
//   uint8_t  pair_id    =  val        & 0xFF;    // bits 24–31 if you’ve shifted differently

//     LOG(  "\nRemote RAW:" + String( val, HEX ) 
//         + " -> id: "      + String( custom_id, HEX ) 
//         + " / command: "  + String( command, HEX )
//         + " / pair_id: "  + String( pair_id, HEX ));


  
//   // 1. Handle Repeat
//   if (val == 0xFFFFFFFF) {
//     resume();
//     if (prevAct == VOLUME_UP || prevAct == VOLUME_DOWN) return prevAct;
//     return NONE;
//   }


//   // 2. Ignore non‑Apple NEC IDs
//   const uint16_t APPLE_CUSTOM_ID = 0x77E1;  // Apple ID for IR Remotes
//   if (custom_id != APPLE_CUSTOM_ID) {
//     resume();
//     return NONE;
//   }

//   // 3. DEBOUNCE CHECK
//   // If the last button press was less than 250ms ago, ignore this one.
//   // This prevents the "double action" on Play/Enter/Menu.
//   if (millis() - lastRemoteMillis < 200) {
//     resume();
//     return NONE; 
//   }


//   LOG( "Extracted Command: " + String(command, HEX) );

//   switch (command) {
//     case 0xD0: case 0xE1:
//     case 0x50:            
//       action = VOLUME_UP;
//       break;
//     case 0xB0: case 0x30: 
//       action = VOLUME_DOWN;
//       break;
//     case 0x10: case 0x90: 
//       action = CHANNEL_LEFT;
//       break;
//     case 0xE0: case 0x60: 
//       action = CHANNEL_RIGHT;
//       break;
//     case 0xBA: case 0x3A: // MIDDLE (Select)
//     case 0x20: case 0xA0: // SPAM Bits, repeats the code so it can cause bounce
//       action = ENTER;
//       break;
//     case 0x40: case 0xC0: 
//       action = MENU;
//       break;
//     case 0x5E: case 0xDE: // PLAY/PAUSE
//     case 0x7A: case 0xFA: // SPAM Bits, repeats the code so it can cause bounce
//       action = PLAY_PAUSE;
//       break;
//     default:
//       action = NONE;
//       break;

//   }

//   LOG(" -> Action Assigned: " + String(action) );
  
//   // Update the timer so we know when this last successful press happened
//   lastRemoteMillis = millis();

//   resume();
//   prevAct = action;
//   return action;
// }

// //==============================================================================
// //==============================================================================
// TouchInterface::TouchInterface(byte tclk, byte tcs, byte tdin, byte dout, byte irq)
//                :URTouch(tclk, tcs, tdin, dout, irq)
// {

//   InitTouch(LANDSCAPE); 
//   setPrecision(PREC_MEDIUM);

//   x = 0;
//   y = 0;

// }

// //------------------------------------------------------------------------------
// ACTION TouchInterface::getAction( PAGE page ){

//   if (!dataAvailable()){
//     return NONE;
//   }

//   read();
//   x = getX();
//   y = getY();

//   //LOG("x: "); LOG(x); LOG(" / y: "); LOG(y); LOG("\n");

//   switch ( page ){
//     case MAIN_MENU:
//       return actionFromMAIN( x, y );
//     case SETTINGS_MENU:
//       return actionFromSETTINGS( x, y );
//   }

// return NONE;
// }

// //------------------------------------------------------------------------------
// ACTION TouchInterface::actionFromMAIN( int x, int y ){

// /* 4.3", 480 X 272
// CAL_X 0x03E9804BUL
// CAL_Y 0x03AC412EUL
// CAL_S 0x801DF10FUL

// x = 0 ->
// y = 0 v
// -------------------------------------------
// ||       1      |           |  6  ||  5  ||
// ||______________|           |_____||_____||
// |                                         |
// |                            _____________|
// |________________           |            ||
// ||  2  |  |  3  |           |     4      ||
// ||     |  |     |           |            ||
// ------------------------------------------- x = 470, y = 270
// */

//   action = NONE;

//   Rect channel  = { { 40,  20}, {220,  80} }; //1
//   Rect vol_down = { { 40, 185}, {100, 250} }; //2
//   Rect vol_up   = { {160, 185}, {220, 250} }; //3
//   Rect mute     = { {300, 140}, {450, 250} }; //4
//   Rect settings = { {390,  20}, {450,  80} }; //5
//   //Rect power_on = { {300,  20}, {360,  80} }; //6


//   if ( x >= channel.p1.x && x <= channel.p2.x ){        // Left column

//       if ( y >= channel.p1.y && y <= channel.p2.y ){    // Button: Input
//         action = CHANNEL_RIGHT;
//         LOG("CHANNEL_RIGHT\n");
//       }else if ( y >= vol_down.p1.y && y <= vol_down.p2.y ) {// Button: vol_down.y == vol_up.y

//         if ( x >= vol_down.p1.x && x <= vol_down.p2.x ){ //Vol down 
//           action = VOLUME_DOWN; 
//           LOG("VOLUME_DOWN\n");
//         } else if ( x >= vol_up.p1.x && x <= vol_up.p2.x ){  //vol up
//           action = VOLUME_UP; 
//           LOG("VOLUME_UP\n");
//         }

//       }

//   }else if ( x >= mute.p1.x && x <= mute.p2.x ){ //Right Column

//       if ( y >= mute.p1.y && y <= mute.p2.y ){          // Button: Mute
//         action = ENTER;
//         LOG("ENTER\n");
//       } else if ( y >= settings.p1.y && y <= settings.p2.y ){  // Button: settings.y == power_on.y
  
//         if ( x >= settings.p1.x && x <= settings.p2.x ){  // Button: MENU
//           action = MENU; 
//           LOG("MENU\n");
//         } 
//         // else if ( x >= power_on.p1.x && x <= power_on.p2.x ){ // Button: POWER 
//         //   action = POWER_ON;
//         //   LOG("POWER_ON\n");
//         // } 

//       }
        
//   }

// return action;
// }

// //------------------------------------------------------------------------------
// ACTION TouchInterface::actionFromSETTINGS( int x, int y ){
  
// /*
// x = 0 ->
// y = 0 v

//   int x = 30;
//   int y = 20; 
//   int h = 50;
//   int l = 320;
//   int jump = 60; 

// -------------------------------------------
// |   |         1          |         |  5  ||
// |   |____________________|         |_____||
// |   |         2          |                |
// |   |____________________|                |
// |   |         3          |                |
// |   |____________________|                |
// |   |         4          |                |
// ------------------------------------------- x = 470, y = 270

// NOTE:
// Use only two buttons shifted in the middle for now
//   int y = 40;
//   int jump = 80;

// */

//   action = NONE;

//   Rect fir_filter = { {30,  20},  {350,  70} }; //1
//   Rect iir_bandw  = { {30,  80},  {350, 130} }; //2
//   Rect dpll_band  = { {30,  140}, {350, 190} }; //3
//   Rect jitter_el  = { {30,  200}, {350, 250} }; //4

//   Rect settings = { {390,  20}, {450,  80} }; //5

//   if ( x >= fir_filter.p1.x && x <= fir_filter.p2.x ){  // Settings Buttons, all the same as FIR

//     if ( y >= fir_filter.p1.y && y <= fir_filter.p2.y ){  // Button: FIR
//       action = SET_FIR_FILTER; 
//       LOG("FIR\n");
//     } else if ( y >= iir_bandw.p1.y && y <= iir_bandw.p2.y ){  // Button: IIR
//       action = SET_IIR_BANDWIDTH; 
//       LOG("IIR\n");
//     }else if ( y >= dpll_band.p1.y && y <= dpll_band.p2.y ){  // Button: DPLL
//       action = SET_DPLL; 
//       LOG("DPLL\n");
//     } else if ( y >= jitter_el.p1.y && y <= jitter_el.p2.y ){  // Button: Jitter eliminator
//       action = TOGGLE_JE; 
//       LOG("TOGLE JE\n");
//     }

//   }else if ( x >= settings.p1.x && x <= settings.p2.x ){  // Button: MENU
//     if ( y >= settings.p1.y && y <= settings.p2.y ){  // Button: MENU
//       action = MENU; 
//       LOG("MENU\n");
//     }  
//   }
        
// return action;
// }

// //------------------------------------------------------------------------------
// bool TouchInterface::detectHold( ACTION& act, ACTION lastAct ){

//         int interval = millis() - pressTime;
//         if( lastAct != act || interval > wait_after_act ){ //interval wait to start new holding
//           pressTime = lastInterruptTime = millis();
//           LOG("----------first touch----------\n");
//         }else{ //hold
//           //First holding
//           int no_touch = millis() - lastInterruptTime;
//           if( no_touch > 200 ){ //reset as holding interrupt detected
//             act = RESET; //this will set all new
//             pressTime = lastInterruptTime = millis();
//             LOG("Interruption, start new counter (reset)\n");
//             return false;
//           }else{
//             lastInterruptTime = millis(); //update interrupt time
//           }
//           interval = millis() - pressTime;
//           LOG( interval );
//           if( interval > act_after ){ // holding
//             /*
//             This is where the touch screen hold action should be placed
//             */
//             LOG("\nhold button: " + String (action) );
//             act = RESET; //this will set all new 
//             return true;

//             pressTime = millis(); //update press time
//         }else if ( interval < 300 ){
//           //tapped_only;
//         }
//       }
//   return false;
// }

// //==============================================================================
