#ifndef UI_STATE_MANAGER_H
#define UI_STATE_MANAGER_H

#include <Preferences.h>

struct UIState {
    bool darkMode = true;
    uint8_t colorIndex = 2;
    uint8_t brightness  = 70;
    bool autoDim = true;
};

class UIStateManager {
public:
    UIStateManager();

    void load();
    void save();

    UIState getState() const { return state; }

    void setDarkMode(bool dark);
    void setColorIndex(uint8_t idx);
    void setBrightness(uint8_t brightness);
    void setAutoDim(bool enabled);


private:
    UIState state;
    Preferences prefs;
};

#endif