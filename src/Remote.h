#ifndef REMOTE_H
#define REMOTE_H

#include <globals.h>

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



#endif /* REMOTE_H */


