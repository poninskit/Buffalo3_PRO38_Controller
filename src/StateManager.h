#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "globals.h"
#include "dac.h"
#include <functional>

struct DACState {
    DAC_INPUT input = OPT1;
    uint8_t volume = DEFAULT_VOL;
    bool muted = false;

    LOCK_STATUS lockStatus = Unknown;
    uint32_t sampleRate = 0;

    const char* lockStatusStr  = "Unknown";
    const char* sampleRateStr  = ""; 

    //filter values
    uint8_t firShape = 0;
    uint8_t iirBandwidth = 0;
    uint8_t dpllBandwidth = 0;
    uint8_t jitterEliminator = 0;
    
    //since Graphics only needs to redraw the value string, we can store it here to avoid querying DAC again for the string conversion
    const char* firShapeStr     = "";
    const char* iirBandwidthStr = "";
    const char* dpllBandwidthStr= "";
    const char* jitterElStr     = "";
};

class StateManager {
public:
    StateManager();

    // Get current state
    DACState getState() const { return currentState; }

    // Update state (called by any source: touch, remote, DAC polling)
    void updateInput(DAC_INPUT newInput);
    void updateVolume(uint8_t newVolume, bool isMuted = false);
    void updateLockStatus(LOCK_STATUS status, const char* str);
    void updateSampleRate(uint32_t rate, const char* str);
    void updateSettings(uint8_t fir, uint8_t iir, uint8_t dpll, uint8_t je);
    void updateSettingsStrings(const char* fir, const char* iir, const char* dpll, const char* je);

    // Register callbacks for state changes
    void onStateChange(std::function<void(const DACState&)> callback);

private:
    DACState currentState;
    std::function<void(const DACState&)> stateChangeCallback;
};

#endif