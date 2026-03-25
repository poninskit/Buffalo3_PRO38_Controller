#ifndef REMOTE_H
#define REMOTE_H

#include <globals.h>

//==============================================================================
// IR Remote – Apple remote, NEC protocol, IRremote v4.x
//==============================================================================
class RemoteInterface {
  public:
    RemoteInterface( int recvpin = 17);
    ACTION getAction( PAGE page = MAIN_MENU );
    bool isRepeat() const { return _isRepeat; }

  private:
    unsigned long lastRemoteMillis = 0;
    ACTION action;
    ACTION prevAct;
    bool _isRepeat = false;
};



#endif /* REMOTE_H */


