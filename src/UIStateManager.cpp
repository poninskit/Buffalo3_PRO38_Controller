#include "UIStateManager.h"

UIStateManager::UIStateManager() {}

void UIStateManager::load() {
    prefs.begin("ui", true);

    state.darkMode  = prefs.getBool("dark", true);
    state.colorIndex = prefs.getUChar("color", 2);
    state.brightness = prefs.getUChar("brightness", 70);
    state.autoDim    = prefs.getBool("autodim", true);

    prefs.end();
}

void UIStateManager::save() {
    prefs.begin("ui", false);

    prefs.putBool("dark", state.darkMode);
    prefs.putUChar("color", state.colorIndex);
    prefs.putUChar("brightness", state.brightness);
    prefs.putBool("autodim", state.autoDim);

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

void UIStateManager::setBrightness(uint8_t val) {
    if (state.brightness != val) {
        state.brightness = val;
        save();
    }
}

void UIStateManager::setAutoDim(bool enabled) {
    if (state.autoDim != enabled) {
        state.autoDim = enabled;
        save();
    }
}