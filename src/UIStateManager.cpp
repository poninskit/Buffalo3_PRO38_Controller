#include "UIStateManager.h"

UIStateManager::UIStateManager() {}

void UIStateManager::load() {
    prefs.begin("ui", true);

    state.darkMode  = prefs.getBool("dark", true);
    state.colorIndex = prefs.getUChar("color", 2);

    prefs.end();
}

void UIStateManager::save() {
    prefs.begin("ui", false);

    prefs.putBool("dark", state.darkMode);
    prefs.putUChar("color", state.colorIndex);

    prefs.end();
}

void UIStateManager::setDarkMode(bool dark) {
    if (state.darkMode != dark) {
        state.darkMode = dark;
        save();
    }
}

void UIStateManager::setColorIndex(uint8_t idx) {
    if (state.colorIndex != idx) {
        state.colorIndex = idx;
        save();
    }
}