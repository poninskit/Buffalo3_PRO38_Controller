#include "Remote.h"
#define DECODE_NEC        // must be before IRremote.hpp to disable all other decoders
#undef DEBUG              // globals.h defines DEBUG=0, but IRremote checks #ifdef DEBUG (not #if DEBUG)
                          // which triggers its verbose internal logging even when DEBUG=0
#include <IRremote.hpp>   // z3t0/IRremote @ ^4.4.1  (ESP32-compatible)

//==============================================================================
// RemoteInterface  –  Apple IR remote, NEC protocol, IRremote v4.x
//==============================================================================
/*
  Apple remote NEC 32-bit frame layout (MSB → LSB in the raw 32-bit word):
    bits 31..16  Custom ID  (always 0x77E1 for Apple remotes)
    bits 15.. 8  Command byte
    bits  7.. 0  Pairing/generation byte (ignored – we only match the command)

  Known commands (command byte):
    UP        0xD0 / 0xE1 / 0x50
    DOWN      0xB0 / 0x30
    LEFT      0x10 / 0x90
    RIGHT     0xE0 / 0x60
    CENTER    0xBA / 0x3A / 0x20 / 0xA0
    MENU      0x40 / 0xC0
    PLAY/PAUSE 0x5E / 0xDE / 0x7A / 0xFA
*/

RemoteInterface::RemoteInterface( int recvpin )
{
    pinMode(recvpin, INPUT_PULLUP);  // prevent floating
    IrReceiver.begin( (uint_fast8_t)recvpin, DISABLE_LED_FEEDBACK );

    if ( IrReceiver.isIdle() ) {
        Serial.println("IrReceiver: initialized OK");
    } else {
        Serial.println("IrReceiver: init may have failed");
    }

    prevAct = NONE;
}


//------------------------------------------------------------------------------
ACTION RemoteInterface::getAction( PAGE page )
{


    if ( !IrReceiver.decode() ){ 
        return NONE;
    }


    
    // ---- Handle auto-repeat -------------------------------------------------
    if ( IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT ) {
        IrReceiver.resume();
        if ( prevAct == VOLUME_UP || prevAct == VOLUME_DOWN ) {
            _isRepeat = true;
            return prevAct;
        }
        return NONE;
    }
    _isRepeat = false;


    // ---- Only reject zeros for non-repeat frames --------------------------
    if (IrReceiver.decodedIRData.decodedRawData == 0) {
        IrReceiver.resume();
        return NONE;
    }


    uint32_t val = IrReceiver.decodedIRData.decodedRawData;

    // Lower byte order of argument depends on IR library.
    // For standard NEC (32 bits, LSB first):
    // Raw value: 0x77e1b036
    // Bit positions (MSB → LSB):
    // 31........24 23........16 15........8 7........0
    //    77           e1          b0         36
    // But NEC logical order is:
    // Address (LSB) | Address (MSB) | Command | ~Command

    // v4 stores bits reversed compared to v2
    uint8_t  pair_id   = (val >> 24) & 0xFF;    // bits 31-24
    uint8_t  command   = (val >> 16) & 0xFF;    // bits 23-16
    uint16_t custom_id = (val >>  0) & 0xFFFF;  // bits 15-0


    LOG(  "\nRemote RAW:" + String( val, HEX )
        + " -> id: "      + String( custom_id, HEX )
        + " / cmd: "      + String( command,   HEX ));


    // ---- Ignore unknown remotes --------------------------------------------
    const uint16_t REMOTE_CUSTOM_ID = 0x87EE; // v4 bit-reversed 0x77E1
    if ( custom_id != REMOTE_CUSTOM_ID ) {
        LOG("\nUnknown remote – ignoring, use apple aluminium remote.");
        IrReceiver.resume();
        return NONE;
    }


    // ---- Debounce ----------------------------------------------------------
    if ( millis() - lastRemoteMillis < 200 ) {
        IrReceiver.resume();
        return NONE;
    }

    // ---- Map command to action ----------------------------------------------
    switch ( command ) {
        case 0x0B: action = CHANNEL_LEFT;   break; //Up
        case 0x0D: action = CHANNEL_RIGHT;  break; //Down
        case 0x07: action = VOLUME_UP;      break; //Right
        case 0x08: action = VOLUME_DOWN;    break; //Left
        case 0x5D: action = ENTER;          break; //Center
        case 0x02: action = MENU;           break; //Menu
        case 0x5E: action = PLAY_PAUSE;     break; //Play/Pause
        default:   action = NONE;           break;
    }

    LOG( " -> Action: " + String(action) );

    lastRemoteMillis = millis();
    IrReceiver.resume();
    prevAct = action;
    
    return action;
}



