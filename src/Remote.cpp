#include "Remote.h"
#include <IRremote.hpp>   // z3t0/IRremote @ ^4.4.1  (ESP32-compatible)

#define DECODE_NEC

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
    _irrecv = new IRrecv( (uint_fast8_t)recvpin );
    _irrecv->begin( (uint_fast8_t)recvpin, DISABLE_LED_FEEDBACK );

    IrReceiver.begin( (uint_fast8_t)recvpin, DISABLE_LED_FEEDBACK );
    
    prevAct = NONE;
}


//------------------------------------------------------------------------------
ACTION RemoteInterface::getAction( PAGE page )
{
    if ( !IrReceiver.decode() )
        return NONE;

    // ---- Handle auto-repeat -------------------------------------------------
    if ( _irrecv->decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT ) {
        _irrecv->resume();
        if ( prevAct == VOLUME_UP || prevAct == VOLUME_DOWN ) return prevAct;
        return NONE;
    }

    uint32_t val = _irrecv->decodedIRData.decodedRawData;

    uint16_t custom_id = (val >> 16) & 0xFFFF;
    uint8_t  command   = (val >>  8) & 0xFF;

    LOG(  "\nRemote RAW:" + String( val, HEX )
        + " -> id: "      + String( custom_id, HEX )
        + " / cmd: "      + String( command,   HEX ));

    // ---- Ignore non-Apple remotes ------------------------------------------
    const uint16_t APPLE_CUSTOM_ID = 0x77E1;
    if ( custom_id != APPLE_CUSTOM_ID ) {
        _irrecv->resume();
        return NONE;
    }

    // ---- Debounce ----------------------------------------------------------
    if ( millis() - lastRemoteMillis < 200 ) {
        _irrecv->resume();
        return NONE;
    }

    // ---- Map command to action ----------------------------------------------
    switch ( command ) {
        case 0xD0: case 0xE1: case 0x50:
            action = VOLUME_UP;    break;
        case 0xB0: case 0x30:
            action = VOLUME_DOWN;  break;
        case 0x10: case 0x90:
            action = CHANNEL_LEFT; break;
        case 0xE0: case 0x60:
            action = CHANNEL_RIGHT;break;
        case 0xBA: case 0x3A:
        case 0x20: case 0xA0:
            action = ENTER;        break;
        case 0x40: case 0xC0:
            action = MENU;         break;
        case 0x5E: case 0xDE:
        case 0x7A: case 0xFA:
            action = PLAY_PAUSE;   break;
        default:
            action = NONE;         break;
    }

    LOG( " -> Action: " + String(action) );

    lastRemoteMillis = millis();
    _irrecv->resume();
    prevAct = action;
    return action;
}



