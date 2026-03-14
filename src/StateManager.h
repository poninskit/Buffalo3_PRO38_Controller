// #ifndef STATE_MANAGER_H
// #define STATE_MANAGER_H

// #include "globals.h"
// #include <functional>

// struct DACState {
//     DAC_INPUT input = OPT1;
//     uint8_t volume = DEFAULT_VOL;
//     bool muted = false;
//     LOCK_STATUS lockStatus = Unknown;
//     uint32_t sampleRate = 0;
//     uint8_t firShape = 0;
//     uint8_t iirBandwidth = 0;
//     uint8_t dpllBandwidth = 0;
//     uint8_t jitterEliminator = 0;
// };

// class StateManager {
// public:
//     StateManager();

//     // Get current state
//     DACState getState() const { return currentState; }

//     // Update state (called by any source: touch, remote, DAC polling)
//     void updateInput(DAC_INPUT newInput);
//     void updateVolume(uint8_t newVolume, bool isMuted = false);
//     void updateLockStatus(LOCK_STATUS status);
//     void updateSampleRate(uint32_t rate);
//     void updateSettings(uint8_t fir, uint8_t iir, uint8_t dpll, uint8_t je);

//     // Register callbacks for state changes
//     void onStateChange(std::function<void(const DACState&)> callback);

// private:
//     DACState currentState;
//     std::function<void(const DACState&)> stateChangeCallback;
// };

// #endif